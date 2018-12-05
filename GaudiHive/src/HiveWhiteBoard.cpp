//====================================================================
//	WhiteBoard (Concurrent Event Data Store)
//--------------------------------------------------------------------
//
//====================================================================
// Include files
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"
#include "Rtypes.h"
#include "ThreadLocalStorage.h"
#include "boost/callable_traits.hpp"
#include "tbb/concurrent_queue.h"
#include "tbb/mutex.h"
#include "tbb/recursive_mutex.h"
#include <utility>

// Interfaces
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"

namespace {
  struct Partition final {
    SmartIF<IDataProviderSvc> dataProvider;
    SmartIF<IDataManagerSvc>  dataManager;
    int                       eventNumber = -1;

    // allow acces 'by type' -- used in fwd
    template <typename T>
    T* get();
  };
  template <>
  IDataProviderSvc* Partition::get<IDataProviderSvc>() {
    return dataProvider.get();
  }
  template <>
  IDataManagerSvc* Partition::get<IDataManagerSvc>() {
    return dataManager.get();
  }

  // C++20: replace with http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0290r2.html
  //         http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4033.html

  template <typename T, typename Mutex = tbb::recursive_mutex, typename ReadLock = typename Mutex::scoped_lock,
            typename WriteLock = ReadLock>
  class Synced {
    T             m_obj;
    mutable Mutex m_mtx;

  public:
    template <typename F>
    decltype( auto ) with_lock( F&& f ) {
      WriteLock lock{m_mtx};
      return f( m_obj );
    }
    template <typename F>
    decltype( auto ) with_lock( F&& f ) const {
      ReadLock lock{m_mtx};
      return f( m_obj );
    }
  };
  // transform an f(T) into an f(Synced<T>)
  template <typename Fun>
  auto with_lock( Fun&& f ) {
    return [f = std::forward<Fun>( f )]( auto& p ) -> decltype( auto ) { return p.with_lock( f ); };
  }
  // call f(T) for each element in a container of Synced<T>
  template <typename ContainerOfSynced, typename Fun>
  void for_( ContainerOfSynced& c, Fun&& f ) {
    std::for_each( begin( c ), end( c ), with_lock( std::forward<Fun>( f ) ) );
  }
} // namespace

TTHREAD_TLS( Synced<Partition>* ) s_current = nullptr;

namespace {
  namespace detail {
    // given a callable F(Arg_t,...)
    // argument_t<F> will be equal to Arg_t
    template <typename F>
    using argument_t = std::tuple_element_t<0, boost::callable_traits::args_t<F>>;
  } // namespace detail

  template <typename Fun>
  StatusCode fwd( Fun f ) {
    if ( !s_current ) return IDataProviderSvc::Status::INVALID_ROOT;
    return s_current->with_lock( [&]( Partition& p ) {
      auto* svc = p.get<std::decay_t<detail::argument_t<Fun>>>();
      return svc ? f( *svc ) : IDataProviderSvc::Status::INVALID_ROOT;
    } );
  }
} // namespace

/**
 * @class HiveWhiteBoard
 *
 * Data service base class. A data service manages the transient data stores
 * and implements the IDataProviderSvc, the IDataManagerSvc and the
 * IPartitionControl interfaces.
 *
 * @author Markus Frank
 * @author Sebastien Ponce
 * @author Pere Mato
 * @version 1.0
 */
class HiveWhiteBoard : public extends<Service, IDataProviderSvc, IDataManagerSvc, IHiveWhiteBoard> {
protected:
  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<std::string> m_loader{this, "DataLoader", "EventPersistencySvc", ""};
  Gaudi::Property<size_t>      m_slots{this, "EventSlots", 1, "number of event slots"};
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};
  Gaudi::Property<bool> m_enableFaultHdlr{this, "EnableFaultHandler", false,
                                          "enable incidents on data creation requests"};

  Gaudi::Property<bool> m_useEvtStoreSvc{this, "useEvtStoreSvc", false};
  /// Pointer to data loader service
  IConversionSvc* m_dataLoader = nullptr;
  /// Reference to address creator
  IAddressCreator* m_addrCreator = nullptr;
  /// Datastore partitions
  std::vector<Synced<Partition>> m_partitions;
  /// fifo queue of free slots
  tbb::concurrent_queue<size_t> m_freeSlots;

public:
  /// Inherited constructor
  using extends::extends;

  /// Standard Destructor
  ~HiveWhiteBoard() override {
    setDataLoader( 0 ).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();
    for_( m_partitions, []( Partition& p ) {
      p.dataManager->release();
      p.dataProvider->release();
    } );
    m_partitions.clear();
  }

  /// Get free slots number
  size_t freeSlots() override { return m_freeSlots.unsafe_size(); }

  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override { return (CLID)m_rootCLID; }
  /// Name for root Event
  const std::string& rootName() const override { return m_rootName; }

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( std::string_view path, IOpaqueAddress* pAddr ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.registerAddress( path, pAddr ); } );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parent, std::string_view path, IOpaqueAddress* pAdd ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.registerAddress( parent, path, pAdd ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.unregisterAddress( path ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.unregisterAddress( pParent, path ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& leaves ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectLeaves( pObject, leaves ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const IRegistry* pObject, std::vector<IRegistry*>& leaves ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectLeaves( pObject, leaves ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const DataObject* pObject, IRegistry*& refpParent ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectParent( pObject, refpParent ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const IRegistry* pObject, IRegistry*& refpParent ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectParent( pObject, refpParent ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.clearSubTree( path ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( DataObject* pObject ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.clearSubTree( pObject ); } );
  }
  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override {
    for_( m_partitions, []( Partition& p ) { p.dataManager->clearStore().ignore(); } );
    return StatusCode::SUCCESS;
  }

  /// Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( std::string_view path, IDataStoreAgent* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseSubTree( path, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseSubTree( pObject, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  StatusCode traverseTree( IDataStoreAgent* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseTree( pAgent ); } );
  }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  StatusCode setRoot( std::string path, DataObject* pObj ) override {
    return fwd(
        [pObj, path = std::move( path )]( IDataManagerSvc& p ) { return p.setRoot( std::move( path ), pObj ); } );
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  StatusCode setRoot( std::string path, IOpaqueAddress* pAddr ) override {
    return fwd(
        [pAddr, path = std::move( path )]( IDataManagerSvc& p ) { return p.setRoot( std::move( path ), pAddr ); } );
  }

  /** IDataManagerSvc: Pass a default data loader to the service.
   * The optional data provider is not considered. On the other hand, the data
   * provider is specified to be the whiteboard.
   */
  StatusCode setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* = nullptr ) override {
    if ( pDataLoader ) pDataLoader->addRef();
    if ( m_dataLoader ) m_dataLoader->release();
    if ( pDataLoader ) pDataLoader->setDataProvider( this );
    m_dataLoader = pDataLoader;
    for_( m_partitions, [&]( Partition& p ) { p.dataManager->setDataLoader( m_dataLoader, this ).ignore(); } );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( const DataStoreItem& item ) override {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->addPreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->removePreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Clear the preload list
  StatusCode resetPreLoad() override {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->resetPreLoad(); } );
    return StatusCode::SUCCESS;
  }
  /// load all preload items of the list
  StatusCode preLoad() override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.preLoad(); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( std::string_view parent, std::string_view obj, DataObject* pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, obj, pObj ); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( DataObject* parent, std::string_view obj, DataObject* pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, obj, pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( path ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj, std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( pObj, path ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* parent, std::string_view path, DataObject*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( std::string_view path, DataObject*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( path, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* parent, std::string_view path, DataObject*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, std::string_view objPath, DataObject* to ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( from, objPath, to ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( std::string_view fullPath, DataObject* to ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( fullPath, to ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, std::string_view objPath ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( DataObject* from, std::string_view objPath ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( path ); } );
  }
  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( pDirectory ); } );
  }
  /// Update object.
  StatusCode updateObject( DataObject* pObj ) override {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( pObj ); } );
  }

  //
  //---IHiveWhiteBard implemenation--------------------------------------------------
  //

  /// Remove all data objects in one 'slot' of the data store.
  StatusCode clearStore( size_t partition ) override {
    return m_partitions[partition].with_lock( []( Partition& p ) { return p.dataManager->clearStore(); } );
  }

  /// Activate a partition object. The  identifies the partition uniquely.
  StatusCode selectStore( size_t partition ) override {
    s_current = &m_partitions[partition];
    return StatusCode::SUCCESS;
  }

  /// Set the number of event slots (copies of DataSvc objects).
  StatusCode setNumberOfStores( size_t slots ) override {
    if ( FSMState() == Gaudi::StateMachine::INITIALIZED || FSMState() == Gaudi::StateMachine::RUNNING ) {
      warning() << "Too late to change the number of slots!" << endmsg;
      return StatusCode::FAILURE;
    }
    m_slots = slots;
    Gaudi::Concurrency::ConcurrencyFlags::setNumConcEvents( slots );
    return StatusCode::SUCCESS;
  }

  /// Get the number of event slots (copies of DataSvc objects).
  size_t getNumberOfStores() const override { return m_slots; }

  /// check if a data object exists in the current store
  bool exists( const DataObjID& id ) override {
    DataObject* pObject{nullptr};
    return findObject( id.fullKey(), pObject ).isSuccess();
  }

  /// Allocate a store partition for a given event number
  size_t allocateStore( int evtnumber ) override {
    // take next free slot in the list
    size_t slot = std::string::npos;
    if ( m_freeSlots.try_pop( slot ) ) {
      assert( slot != std::string::npos );
      assert( slot < m_partitions.size() );
      m_partitions[slot].with_lock( [evtnumber]( Partition& p ) {
        assert( p.eventNumber == -1 ); // or whatever value represents 'free'
        p.eventNumber = evtnumber;
      } );
    }
    return slot;
  }

  /// Free a store partition
  StatusCode freeStore( size_t partition ) override {
    assert( partition < m_partitions.size() );
    auto prev = m_partitions[partition].with_lock( []( Partition& p ) { return std::exchange( p.eventNumber, -1 ); } );
    if ( UNLIKELY( prev == -1 ) ) return StatusCode::FAILURE; // double free -- should never happen!
    m_freeSlots.push( partition );
    return StatusCode::SUCCESS;
  }

  /// Get the partition number corresponding to a given event
  size_t getPartitionNumber( int eventnumber ) const override {
    auto i = std::find_if( begin( m_partitions ), end( m_partitions ),
                           with_lock( [eventnumber]( const Partition& p ) { return p.eventNumber == eventnumber; } ) );
    return i != end( m_partitions ) ? std::distance( begin( m_partitions ), i ) : std::string::npos;
  }

  StatusCode attachServices() {
    StatusCode sc = service( m_loader, m_addrCreator, true );
    if ( !sc.isSuccess() ) {
      error() << "Failed to retrieve data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    IConversionSvc* dataLoader = nullptr;
    sc                         = service( m_loader, dataLoader, true );
    if ( !sc.isSuccess() ) {
      error() << MSG::ERROR << "Failed to retrieve data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    sc = setDataLoader( dataLoader );
    dataLoader->release();
    if ( !sc.isSuccess() ) {
      error() << MSG::ERROR << "Failed to set data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    return sc;
  }

  StatusCode detachServices() {
    if ( m_addrCreator ) m_addrCreator->release();
    if ( m_dataLoader ) m_dataLoader->release();
    m_addrCreator = nullptr;
    m_dataLoader  = nullptr;
    return StatusCode::SUCCESS;
  }

  //
  //---IService implemenation---------------------------------------------------------
  //

  /// Service initialisation
  StatusCode initialize() override {
    StatusCode sc = Service::initialize();
    if ( !sc.isSuccess() ) {
      error() << "Unable to initialize base class" << endmsg;
      return sc;
    }
    if ( m_slots < (size_t)1 ) {
      error() << "Invalid number of slots (" << m_slots << ")" << endmsg;
      return StatusCode::FAILURE;
    }

    if ( !setNumberOfStores( m_slots ).isSuccess() ) {
      error() << "Cannot set number of slots" << endmsg;
      return StatusCode::FAILURE;
    }

    m_partitions = std::vector<Synced<Partition>>( m_slots );
    for ( size_t i = 0; i < m_slots; i++ ) {
      SmartIF<IService> isvc;
      if ( m_useEvtStoreSvc.value() ) {
        IService* svc = nullptr;
        serviceLocator()->service( Gaudi::Utils::TypeNameString( name() + "_" + std::to_string( i ), "EvtStoreSvc" ),
                                   svc );
        isvc = svc;
      } else {
        DataSvc* svc = new DataSvc( name() + "_" + std::to_string( i ), serviceLocator() );
        svc->setProperty( m_enableFaultHdlr ).ignore();
        isvc = svc;
      }
      // Percolate properties
      auto iprp = isvc.as<IProperty>();
      iprp->setProperty( m_forceLeaves ).ignore();
      iprp->setProperty( m_rootCLID ).ignore();
      iprp->setProperty( m_rootName ).ignore();
      // make sure that CommonMessaging is initialized
      iprp->setProperty( m_outputLevel ).ignore();

      if ( !m_useEvtStoreSvc.value() ) sc = isvc->initialize();
      if ( !sc.isSuccess() ) {
        error() << "Failed to instantiate DataSvc as store partition" << endmsg;
        return sc;
      }
      m_partitions[i].with_lock( [&]( Partition& p ) {
        p.dataProvider = isvc;
        p.dataManager  = isvc;
      } );
      m_freeSlots.push( i );
    }
    selectStore( 0 ).ignore();
    return attachServices();
  }

  /// Service initialisation
  StatusCode reinitialize() override {
    StatusCode sc = Service::reinitialize();
    if ( !sc.isSuccess() ) {
      error() << "Unable to reinitialize base class" << endmsg;
      return sc;
    }
    detachServices();
    sc = attachServices();
    if ( !sc.isSuccess() ) {
      error() << "Failed to attach necessary services." << endmsg;
      return sc;
    }
    return StatusCode::SUCCESS;
  }

  /// Service initialisation
  StatusCode finalize() override
  {
    setDataLoader( nullptr ).ignore();
    clearStore().ignore();
    return Service::finalize();
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( HiveWhiteBoard )

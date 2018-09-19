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

namespace
{
  struct Partition final {
    SmartIF<IDataProviderSvc> dataProvider;
    SmartIF<IDataManagerSvc>  dataManager;
    DataObjIDColl             newDataObjects;
    int                       eventNumber = -1;

    // allow acces 'by type' -- used in fwd
    template <typename T>
    T* get();
  };
  template <>
  IDataProviderSvc* Partition::get<IDataProviderSvc>()
  {
    return dataProvider.get();
  }
  template <>
  IDataManagerSvc* Partition::get<IDataManagerSvc>()
  {
    return dataManager.get();
  }

  // C++20: replace with http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0290r2.html
  //         http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4033.html

  template <typename T, typename Mutex = tbb::recursive_mutex, typename ReadLock = typename Mutex::scoped_lock,
            typename WriteLock = ReadLock>
  class Synced
  {
    T             m_obj;
    mutable Mutex m_mtx;

  public:
    template <typename F>
    auto with_lock( F&& f ) -> decltype( auto )
    {
      WriteLock lock{m_mtx};
      return f( m_obj );
    }
    template <typename F>
    auto with_lock( F&& f ) const -> decltype( auto )
    {
      ReadLock lock{m_mtx};
      return f( m_obj );
    }
  };
  // transform an f(T) into an f(Synced<T>)
  template <typename Fun>
  auto with_lock( Fun&& f )
  {
    return [f = std::forward<Fun>( f )]( auto& p )->decltype( auto ) { return p.with_lock( f ); };
  }
  // call f(T) for each element in a container of Synced<T>
  template <typename ContainerOfSynced, typename Fun>
  void for_( ContainerOfSynced& c, Fun&& f )
  {
    std::for_each( begin( c ), end( c ), with_lock( std::forward<Fun>( f ) ) );
  }
}

TTHREAD_TLS( Synced<Partition>* ) s_current = nullptr;

namespace
{
  namespace detail
  {
    template <typename lambda>
    struct arg_helper : public arg_helper<decltype( &lambda::operator() )> {
    };
    template <typename T, typename Ret, typename Arg>
    struct arg_helper<Ret ( T::* )( Arg ) const> {
      using type = Arg;
    };

    // given a unary lambda whose argument is of type Arg_t,
    // argument_t<lambda> will be equal to Arg_t
    template <typename lambda>
    using argument_t = typename arg_helper<lambda>::type;
  }

  template <typename Fun>
  StatusCode fwd( Fun f )
  {
    if ( !s_current ) return IDataProviderSvc::Status::INVALID_ROOT;
    return s_current->with_lock( [&]( Partition& p ) {
      auto* svc = p.get<std::decay_t<detail::argument_t<Fun>>>();
      return svc ? f( *svc ) : IDataProviderSvc::Status::INVALID_ROOT;
    } );
  }
}

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
class HiveWhiteBoard : public extends<Service, IDataProviderSvc, IDataManagerSvc, IHiveWhiteBoard>
{
protected:
  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<std::string> m_loader{this, "DataLoader", "EventPersistencySvc", ""};
  Gaudi::Property<size_t>      m_slots{this, "EventSlots", 1, "number of event slots"};
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};
  Gaudi::Property<bool> m_enableFaultHdlr{this, "EnableFaultHandler", false,
                                          "enable incidents on data creation requests"};

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
  ~HiveWhiteBoard() override
  {
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
  StatusCode registerAddress( boost::string_ref path, IOpaqueAddress* pAddr ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.registerAddress( path, pAddr ); } );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( DataObject* parent, boost::string_ref path, IOpaqueAddress* pAddr ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.registerAddress( parent, path, pAddr ); } );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parent, boost::string_ref path, IOpaqueAddress* pAdd ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.registerAddress( parent, path, pAdd ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( boost::string_ref path ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.unregisterAddress( path ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( DataObject* pParent, boost::string_ref path ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.unregisterAddress( pParent, path ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, boost::string_ref path ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.unregisterAddress( pParent, path ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& leaves ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectLeaves( pObject, leaves ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const IRegistry* pObject, std::vector<IRegistry*>& leaves ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectLeaves( pObject, leaves ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const DataObject* pObject, IRegistry*& refpParent ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectParent( pObject, refpParent ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const IRegistry* pObject, IRegistry*& refpParent ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.objectParent( pObject, refpParent ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( boost::string_ref path ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.clearSubTree( path ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( DataObject* pObject ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.clearSubTree( pObject ); } );
  }
  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override
  {
    for_( m_partitions, []( Partition& p ) { p.dataManager->clearStore().ignore(); } );
    return StatusCode::SUCCESS;
  }

  /// Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( boost::string_ref path, IDataStoreAgent* pAgent ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseSubTree( path, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseSubTree( pObject, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  StatusCode traverseTree( IDataStoreAgent* pAgent ) override
  {
    return fwd( [&]( IDataManagerSvc& p ) { return p.traverseTree( pAgent ); } );
  }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  StatusCode setRoot( std::string path, DataObject* pObj ) override
  {
    return fwd(
        [ pObj, path = std::move( path ) ]( IDataManagerSvc & p ) { return p.setRoot( std::move( path ), pObj ); } );
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  StatusCode setRoot( std::string path, IOpaqueAddress* pAddr ) override
  {
    return fwd(
        [ pAddr, path = std::move( path ) ]( IDataManagerSvc & p ) { return p.setRoot( std::move( path ), pAddr ); } );
  }

  /** IDataManagerSvc: Pass a default data loader to the service.
   * The optional data provider is not considered. On the other hand, the data
   * provider is specified to be the whiteboard.
   */
  StatusCode setDataLoader( IConversionSvc*   pDataLoader,
                            IDataProviderSvc* dpsvc __attribute__( ( unused ) ) = nullptr ) override
  {
    if ( pDataLoader ) pDataLoader->addRef();
    if ( m_dataLoader ) m_dataLoader->release();
    if ( pDataLoader ) pDataLoader->setDataProvider( this );
    m_dataLoader = pDataLoader;
    for_( m_partitions, [&]( Partition& p ) { p.dataManager->setDataLoader( m_dataLoader, this ).ignore(); } );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( const DataStoreItem& item ) override
  {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->addPreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( std::string item ) override
  {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->addPreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override
  {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->removePreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode removePreLoadItem( std::string item ) override
  {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->removePreLoadItem( item ); } );
    return StatusCode::SUCCESS;
  }
  /// Clear the preload list
  StatusCode resetPreLoad() override
  {
    for_( m_partitions, [&]( Partition& p ) { p.dataProvider->resetPreLoad(); } );
    return StatusCode::SUCCESS;
  }
  /// load all preload items of the list
  StatusCode preLoad() override
  {
    return s_current->with_lock( []( Partition& p ) {
      StatusCode sc = p.dataProvider->preLoad();
      p.dataManager->traverseTree( [&p]( IRegistry* pReg, int ) {
        if ( !pReg->object() ) return false;
        p.newDataObjects.insert( DataObjID( pReg->identifier() ) );
        return true;
      } );
      return sc;
    } );
  }
  /// Register object with the data store.  (The most common one is the only monitored one for the time being....)
  StatusCode registerObject( boost::string_ref path, DataObject* pObj ) override
  {
    return s_current->with_lock( [&]( Partition& p ) {
      StatusCode sc = p.dataProvider->registerObject( path, pObj );
      if ( sc.isSuccess() ) {
        p.newDataObjects.insert( DataObjID( std::string{path.data(), path.size()} ) );
      }
      return sc;
    } );
  }
  /// Register object with the data store.
  StatusCode registerObject( boost::string_ref parent, boost::string_ref obj, DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, obj, pObj ); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( boost::string_ref parent, int item, DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, item, pObj ); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( DataObject* parent, boost::string_ref obj, DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, obj, pObj ); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( DataObject* parent, int obj, DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.registerObject( parent, obj, pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( boost::string_ref path ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( path ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( boost::string_ref parent, boost::string_ref obj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( parent, obj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( boost::string_ref parent, int obj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( parent, obj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj, boost::string_ref path ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( pObj, path ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj, int item ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unregisterObject( pObj, item ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* parent, boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Retrieve object identified by its full path from the data store.
  StatusCode retrieveObject( boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( path, pObj ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( boost::string_ref parent, boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( boost::string_ref parent, int item, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, item, pObj ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parent, boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parent, int item, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, item, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( path, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* parent, boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Find object in the data store.
  StatusCode findObject( boost::string_ref parent, boost::string_ref path, DataObject*& pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.retrieveObject( parent, path, pObj ); } );
  }
  /// Find object in the data store.
  StatusCode findObject( boost::string_ref parent, int item, DataObject*& pObject ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.findObject( parent, item, pObject ); } );
  }
  /// Find object in the data store.
  StatusCode findObject( DataObject* parent, boost::string_ref path, DataObject*& pObject ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.findObject( parent, path, pObject ); } );
  }
  /// Find object in the data store.
  StatusCode findObject( DataObject* parent, int item, DataObject*& pObject ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.findObject( parent, item, pObject ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, boost::string_ref objPath, DataObject* to ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( from, objPath, to ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( boost::string_ref from, boost::string_ref objPath, DataObject* to ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( from, objPath, to ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( DataObject* from, boost::string_ref objPath, DataObject* to ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( from, objPath, to ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( boost::string_ref fullPath, DataObject* to ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.linkObject( fullPath, to ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, boost::string_ref objPath ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( boost::string_ref from, boost::string_ref objPath ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( DataObject* from, boost::string_ref objPath ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( boost::string_ref path ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.unlinkObject( path ); } );
  }
  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( pDirectory ); } );
  }
  /// Update object.
  StatusCode updateObject( boost::string_ref path ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( path ); } );
  }
  /// Update object.
  StatusCode updateObject( DataObject* pObj ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( pObj ); } );
  }
  /// Update object.
  StatusCode updateObject( boost::string_ref parent, boost::string_ref updatePath ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( parent, updatePath ); } );
  }
  /// Update object.
  StatusCode updateObject( DataObject* parent, boost::string_ref updatePath ) override
  {
    return fwd( [&]( IDataProviderSvc& p ) { return p.updateObject( parent, updatePath ); } );
  }

  //
  //---IHiveWhiteBard implemenation--------------------------------------------------
  //

  /// Remove all data objects in one 'slot' of the data store.
  StatusCode clearStore( size_t partition ) override
  {
    return m_partitions[partition].with_lock( []( Partition& p ) { return p.dataManager->clearStore(); } );
  }

  /// Activate a partition object. The  identifies the partition uniquely.
  StatusCode selectStore( size_t partition ) override
  {
    s_current = &m_partitions[partition];
    return StatusCode::SUCCESS;
  }

  /// Set the number of event slots (copies of DataSvc objects).
  StatusCode setNumberOfStores( size_t slots ) override
  {
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

  /// Get the list of new DataObjects in the current store.
  DataObjIDColl getNewDataObjects() override
  {
    return s_current->with_lock( [&]( Partition& p ) { return std::exchange( p.newDataObjects, {} ); } );
  }

  /// add to the list of new DataObjects in the current store.
  void addNewDataObjects( DataObjIDColl& products ) override
  {
    s_current->with_lock( [&]( Partition& p ) { p.newDataObjects.insert( begin( products ), end( products ) ); } );
  }

  /// check if a data object exists in the current store
  bool exists( const DataObjID& id ) override
  {
    DataObject* pObject{nullptr};
    return findObject( id.fullKey(), pObject ).isSuccess();
  }

  /// Allocate a store partition for a given event number
  size_t allocateStore( int evtnumber ) override
  {
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
  StatusCode freeStore( size_t partition ) override
  {
    assert( partition < m_partitions.size() );
    m_partitions[partition].with_lock( []( Partition& p ) { p.eventNumber = -1; } );
    m_freeSlots.push( partition );
    return StatusCode::SUCCESS;
  }

  /// Get the partition number corresponding to a given event
  size_t getPartitionNumber( int eventnumber ) const override
  {
    auto i = std::find_if( begin( m_partitions ), end( m_partitions ),
                           with_lock( [eventnumber]( const Partition& p ) { return p.eventNumber == eventnumber; } ) );
    return i != end( m_partitions ) ? std::distance( begin( m_partitions ), i ) : std::string::npos;
  }

  StatusCode attachServices()
  {
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

  StatusCode detachServices()
  {
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
  StatusCode initialize() override
  {
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
      DataSvc* svc = new DataSvc( name() + "_" + std::to_string( i ), serviceLocator() );
      // Percolate properties
      svc->setProperty( m_rootCLID ).ignore();
      svc->setProperty( m_rootName ).ignore();
      svc->setProperty( m_forceLeaves ).ignore();
      svc->setProperty( m_enableFaultHdlr ).ignore();
      // make sure that CommonMessaging is initialized
      svc->setProperty( m_outputLevel ).ignore();

      sc = svc->initialize();
      if ( !sc.isSuccess() ) {
        error() << "Failed to instantiate DataSvc as store partition" << endmsg;
        return sc;
      }
      m_partitions[i].with_lock( [&]( Partition& p ) {
        p.dataProvider = svc;
        p.dataManager  = svc;
      } );
      m_freeSlots.push( i );
    }
    selectStore( 0 ).ignore();
    return attachServices();
  }

  /// Service initialisation
  StatusCode reinitialize() override
  {
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
    setDataLoader( 0 ).ignore();
    clearStore().ignore();
    return Service::finalize();
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( HiveWhiteBoard )

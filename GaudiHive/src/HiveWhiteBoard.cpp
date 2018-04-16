//====================================================================
//	WhiteBoard (Concurrent Event Data Store)
//--------------------------------------------------------------------
//
//====================================================================
#define WHITEBOARD_CPP

// Include files
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/TypeNameString.h"
#include "Rtypes.h"
#include "ThreadLocalStorage.h"
#include "tbb/recursive_mutex.h"
#include "tbb/spin_mutex.h"

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

// Forward declarations

typedef tbb::recursive_mutex wbMutex;
// typedef  tbb::spin_mutex wbMutex;

namespace
{
  struct Partition {

    SmartIF<IDataProviderSvc> dataProvider;
    SmartIF<IDataManagerSvc>  dataManager;
    wbMutex                   storeMutex;
    DataObjIDColl             newDataObjects;
    int                       eventNumber;
    Partition() : dataProvider( 0 ), dataManager( 0 ), eventNumber( -1 ) {}
    Partition( IDataProviderSvc* dp, IDataManagerSvc* dm ) : dataProvider( dp ), dataManager( dm ), eventNumber( -1 ) {}
    Partition( const Partition& entry )
        : dataProvider( entry.dataProvider ), dataManager( entry.dataManager ), eventNumber( entry.eventNumber )
    {
    }
    Partition& operator=( const Partition& entry )
    {
      dataProvider = entry.dataProvider;
      dataManager  = entry.dataManager;
      return *this;
    }
  };
  class DataAgent : virtual public IDataStoreAgent
  {
  private:
    DataObjIDColl& m_dataObjects;

  public:
    DataAgent( DataObjIDColl& objs ) : m_dataObjects( objs ) {}
    ~DataAgent() override {}
    bool analyse( IRegistry* pReg, int ) override
    {
      if ( 0 != pReg->object() ) {
        m_dataObjects.insert( DataObjID( pReg->identifier() ) );
        return true;
      } else {
        return false;
      }
    }
  };
}

TTHREAD_TLS( Partition* ) s_current( 0 );

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
  typedef std::vector<Partition> Partitions;

  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<std::string> m_loader{this, "DataLoader", "EventPersistencySvc", ""};
  Gaudi::Property<int>         m_slots{this, "EventSlots", 1, "number of event slots"};
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};
  Gaudi::Property<bool> m_enableFaultHdlr{this, "EnableFaultHandler", false,
                                          "enable incidents on data creation requests"};

  /// Pointer to data loader service
  IConversionSvc* m_dataLoader = nullptr;
  /// Reference to address creator
  IAddressCreator* m_addrCreator = nullptr;
  /// Datastore partitions
  Partitions m_partitions;

public:
  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override { return (CLID)m_rootCLID; }
  /// Name for root Event
  const std::string& rootName() const override { return m_rootName; }

// macro to help writing the function calls
#define _CALL( P, F, ARGS )                                                                                            \
  if ( s_current ) {                                                                                                   \
    wbMutex::scoped_lock lock;                                                                                         \
    lock.acquire( s_current->storeMutex );                                                                             \
    return s_current->P ? s_current->P->F ARGS : IDataProviderSvc::INVALID_ROOT;                                       \
  }                                                                                                                    \
  return IDataProviderSvc::INVALID_ROOT;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( const std::string& path, IOpaqueAddress* pAddr ) override
  {
    _CALL( dataManager, registerAddress, ( path, pAddr ) );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( DataObject* parent, const std::string& path, IOpaqueAddress* pAddr ) override
  {
    _CALL( dataManager, registerAddress, ( parent, path, pAddr ) );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parent, const std::string& path, IOpaqueAddress* pAdd ) override
  {
    _CALL( dataManager, registerAddress, ( parent, path, pAdd ) );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( const std::string& path ) override
  {
    _CALL( dataManager, unregisterAddress, ( path ) );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( DataObject* pParent, const std::string& path ) override
  {
    _CALL( dataManager, unregisterAddress, ( pParent, path ) );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, const std::string& path ) override
  {
    _CALL( dataManager, unregisterAddress, ( pParent, path ) );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& leaves ) override
  {
    _CALL( dataManager, objectLeaves, ( pObject, leaves ) );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const IRegistry* pObject, std::vector<IRegistry*>& leaves ) override
  {
    _CALL( dataManager, objectLeaves, ( pObject, leaves ) );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const DataObject* pObject, IRegistry*& refpParent ) override
  {
    _CALL( dataManager, objectParent, ( pObject, refpParent ) );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const IRegistry* pObject, IRegistry*& refpParent ) override
  {
    _CALL( dataManager, objectParent, ( pObject, refpParent ) );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( const std::string& path ) override { _CALL( dataManager, clearSubTree, ( path ) ); }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( DataObject* pObject ) override { _CALL( dataManager, clearSubTree, ( pObject ) ); }
  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override
  {
    for ( auto& p : m_partitions ) p.dataManager->clearStore().ignore();
    return StatusCode::SUCCESS;
  }

  /// Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( const std::string& path, IDataStoreAgent* pAgent ) override
  {
    _CALL( dataManager, traverseSubTree, ( path, pAgent ) );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) override
  {
    _CALL( dataManager, traverseSubTree, ( pObject, pAgent ) );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  StatusCode traverseTree( IDataStoreAgent* pAgent ) override { _CALL( dataManager, traverseTree, ( pAgent ) ); }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  StatusCode setRoot( std::string path, DataObject* pObj ) override { _CALL( dataManager, setRoot, ( path, pObj ) ); }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  StatusCode setRoot( std::string path, IOpaqueAddress* pAddr ) override
  {
    _CALL( dataManager, setRoot, ( path, pAddr ) );
  }

  /** IDataManagerSvc: Pass a default data loader to the service.
   * The optional data provider is not considered. On the other hand, the data
   * provider is specified to be the whiteboard.
   */
  StatusCode setDataLoader( IConversionSvc*   pDataLoader,
                            IDataProviderSvc* dpsvc __attribute__( ( unused ) ) = nullptr ) override
  {
    if ( 0 != pDataLoader ) pDataLoader->addRef();
    if ( 0 != m_dataLoader ) m_dataLoader->release();
    if ( 0 != pDataLoader ) {
      pDataLoader->setDataProvider( this );
    }
    m_dataLoader = pDataLoader;
    for ( auto& p : m_partitions ) {
      p.dataManager->setDataLoader( m_dataLoader, this ).ignore();
    }
    return SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( const DataStoreItem& item ) override
  {
    for ( auto& p : m_partitions ) p.dataProvider->addPreLoadItem( item );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( const std::string& item ) override
  {
    for ( auto& p : m_partitions ) p.dataProvider->addPreLoadItem( item );
    return StatusCode::SUCCESS;
  }
  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override
  {
    for ( auto& p : m_partitions ) p.dataProvider->removePreLoadItem( item );
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode removePreLoadItem( const std::string& item ) override
  {
    for ( auto& p : m_partitions ) p.dataProvider->removePreLoadItem( item );
    return StatusCode::SUCCESS;
  }
  /// Clear the preload list
  StatusCode resetPreLoad() override
  {
    for ( auto& p : m_partitions ) p.dataProvider->resetPreLoad();
    return StatusCode::SUCCESS;
  }
  /// load all preload items of the list
  StatusCode preLoad() override
  {
    wbMutex::scoped_lock lock;
    lock.acquire( s_current->storeMutex );
    StatusCode sc = s_current->dataProvider->preLoad();
    DataAgent  da( s_current->newDataObjects );
    s_current->dataManager->traverseTree( &da );
    return sc;
  }
  /// Register object with the data store.  (The most common one is the only monitored one for the time being....)
  StatusCode registerObject( const std::string& path, DataObject* pObj ) override
  {
    wbMutex::scoped_lock lock;
    lock.acquire( s_current->storeMutex );
    StatusCode sc = s_current->dataProvider->registerObject( path, pObj );
    if ( sc.isSuccess() ) {
      s_current->newDataObjects.insert( DataObjID( path ) );
    }
    return sc;
  }
  /// Register object with the data store.
  StatusCode registerObject( const std::string& parent, const std::string& obj, DataObject* pObj ) override
  {
    _CALL( dataProvider, registerObject, ( parent, obj, pObj ) );
  }
  /// Register object with the data store.
  StatusCode registerObject( const std::string& parent, int item, DataObject* pObj ) override
  {
    _CALL( dataProvider, registerObject, ( parent, item, pObj ) );
  }
  /// Register object with the data store.
  StatusCode registerObject( DataObject* parent, const std::string& obj, DataObject* pObj ) override
  {
    _CALL( dataProvider, registerObject, ( parent, obj, pObj ) );
  }
  /// Register object with the data store.
  StatusCode registerObject( DataObject* parent, int obj, DataObject* pObj ) override
  {
    _CALL( dataProvider, registerObject, ( parent, obj, pObj ) );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& path ) override { _CALL( dataProvider, unregisterObject, ( path ) ); }
  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& parent, const std::string& obj ) override
  {
    _CALL( dataProvider, unregisterObject, ( parent, obj ) );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& parent, int obj ) override
  {
    _CALL( dataProvider, unregisterObject, ( parent, obj ) );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj ) override { _CALL( dataProvider, unregisterObject, ( pObj ) ); }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj, const std::string& path ) override
  {
    _CALL( dataProvider, unregisterObject, ( pObj, path ) );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObj, int item ) override
  {
    _CALL( dataProvider, unregisterObject, ( pObj, item ) );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* parent, const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, path, pObj ) );
  }
  /// Retrieve object identified by its full path from the data store.
  StatusCode retrieveObject( const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( path, pObj ) );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( const std::string& parent, const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, path, pObj ) );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( const std::string& parent, int item, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, item, pObj ) );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parent, const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, path, pObj ) );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parent, int item, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, item, pObj ) );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( path, pObj ) );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* parent, const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, path, pObj ) );
  }
  /// Find object in the data store.
  StatusCode findObject( const std::string& parent, const std::string& path, DataObject*& pObj ) override
  {
    _CALL( dataProvider, retrieveObject, ( parent, path, pObj ) );
  }
  /// Find object in the data store.
  StatusCode findObject( const std::string& parent, int item, DataObject*& pObject ) override
  {
    _CALL( dataProvider, findObject, ( parent, item, pObject ) );
  }
  /// Find object in the data store.
  StatusCode findObject( DataObject* parent, const std::string& path, DataObject*& pObject ) override
  {
    _CALL( dataProvider, findObject, ( parent, path, pObject ) );
  }
  /// Find object in the data store.
  StatusCode findObject( DataObject* parent, int item, DataObject*& pObject ) override
  {
    _CALL( dataProvider, findObject, ( parent, item, pObject ) );
  }
  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, const std::string& objPath, DataObject* to ) override
  {
    _CALL( dataProvider, linkObject, ( from, objPath, to ) );
  }
  /// Add a link to another object.
  StatusCode linkObject( const std::string& from, const std::string& objPath, DataObject* to ) override
  {
    _CALL( dataProvider, linkObject, ( from, objPath, to ) );
  }
  /// Add a link to another object.
  StatusCode linkObject( DataObject* from, const std::string& objPath, DataObject* to ) override
  {
    _CALL( dataProvider, linkObject, ( from, objPath, to ) );
  }
  /// Add a link to another object.
  StatusCode linkObject( const std::string& fullPath, DataObject* to ) override
  {
    _CALL( dataProvider, linkObject, ( fullPath, to ) );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, const std::string& objPath ) override
  {
    _CALL( dataProvider, unlinkObject, ( from, objPath ) );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( const std::string& from, const std::string& objPath ) override
  {
    _CALL( dataProvider, unlinkObject, ( from, objPath ) );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( DataObject* from, const std::string& objPath ) override
  {
    _CALL( dataProvider, unlinkObject, ( from, objPath ) );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( const std::string& path ) override { _CALL( dataProvider, unlinkObject, ( path ) ); }
  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override { _CALL( dataProvider, updateObject, ( pDirectory ) ); }
  /// Update object.
  StatusCode updateObject( const std::string& path ) override { _CALL( dataProvider, updateObject, ( path ) ); }
  /// Update object.
  StatusCode updateObject( DataObject* pObj ) override { _CALL( dataProvider, updateObject, ( pObj ) ); }
  /// Update object.
  StatusCode updateObject( const std::string& parent, const std::string& updatePath ) override
  {
    _CALL( dataProvider, updateObject, ( parent, updatePath ) );
  }
  /// Update object.
  StatusCode updateObject( DataObject* parent, const std::string& updatePath ) override
  {
    _CALL( dataProvider, updateObject, ( parent, updatePath ) );
  }

  //
  //---IHiveWhiteBard implemenation--------------------------------------------------
  //

  /// Remove all data objects in one 'slot' of the data store.
  StatusCode clearStore( size_t partition ) override { return m_partitions[partition].dataManager->clearStore(); }

  /// Activate a partition object. The  identifies the partition uniquely.
  StatusCode selectStore( size_t partition ) override
  {
    s_current = &m_partitions[partition];
    return StatusCode::SUCCESS;
  }

  /// Set the number of event slots (copies of DataSvc objects).
  StatusCode setNumberOfStores( size_t slots ) override
  {
    if ( (int)slots != m_slots and FSMState() == Gaudi::StateMachine::INITIALIZED ) {
      warning() << "Too late to change the number of slots!" << endmsg;
      return StatusCode::FAILURE;
    }
    m_slots = slots;
    return StatusCode::SUCCESS;
  }

  /// Get the number of event slots (copies of DataSvc objects).
  size_t getNumberOfStores() const override { return m_slots; }

  /// Get the list of new DataObjects in the current store.
  StatusCode getNewDataObjects( DataObjIDColl& products ) override
  {
    wbMutex::scoped_lock lock;
    lock.acquire( s_current->storeMutex );
    products = s_current->newDataObjects;
    s_current->newDataObjects.clear();
    return StatusCode::SUCCESS;
  }

  /// Check if new DataObjects are in the current store.
  bool newDataObjectsPresent() override
  {
    wbMutex::scoped_lock lock;
    lock.acquire( s_current->storeMutex );
    return s_current->newDataObjects.size() != 0;
  }

  /// Allocate a store partition for a given event number
  size_t allocateStore( int evtnumber ) override
  {
    // size_t free = std::string::npos;
    size_t index = 0;
    for ( auto& p : m_partitions ) {
      if ( p.eventNumber == evtnumber ) {
        error() << "Attempt to allocate a store partition for an event that is still active" << endmsg;
        return std::string::npos;
      } else if ( p.eventNumber == -1 ) {
        p.eventNumber = evtnumber;
        // info() << "Got allocated slot..." << index << endmsg;
        return index;
      }
      index++;
    }
    return std::string::npos;
  }

  /// Free a store partition
  StatusCode freeStore( size_t partition ) override
  {
    m_partitions[partition].eventNumber = -1;
    // info() << "Freed slot..." << partition << endmsg;
    return StatusCode::SUCCESS;
  }

  /// Get the partition number corresponding to a given event
  size_t getPartitionNumber( int eventnumber ) const override
  {
    size_t index{};
    for ( auto& p : m_partitions ) {
      if ( p.eventNumber == eventnumber ) return index;
      ++index;
    }
    return std::string::npos;
  }

  StatusCode attachServices()
  {
    StatusCode sc = service( m_loader, m_addrCreator, true );
    if ( !sc.isSuccess() ) {
      error() << "Failed to retrieve data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    IConversionSvc* dataLoader = 0;
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
    m_addrCreator = 0;
    m_dataLoader  = 0;
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

    if ( 1 > m_slots ) {
      error() << "Invalid number of slots (" << m_slots << ")" << endmsg;
      return StatusCode::FAILURE;
    }

    for ( int i = 0; i < m_slots; i++ ) {
      std::ostringstream oss;
      oss << name() << "_" << i;
      DataSvc* svc = new DataSvc( oss.str(), serviceLocator() );
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
      m_partitions.push_back( Partition( svc, svc ) );
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

  /// Inherited constructor
  using extends::extends;

  /// Standard Destructor
  virtual ~HiveWhiteBoard()
  {
    setDataLoader( 0 ).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();

    for ( Partitions::iterator i = m_partitions.begin(); i != m_partitions.end(); ++i ) {
      ( *i ).dataManager->release();
      ( *i ).dataProvider->release();
    }
    m_partitions.clear();
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY( HiveWhiteBoard )

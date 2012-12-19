//====================================================================
//	WhiteBoard (Concurrent Event Data Store)
//--------------------------------------------------------------------
//
//====================================================================
#define  WHITEBOARD_CPP

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataSvc.h"
#include "tbb/mutex.h"

//Interfaces
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"

// Forward declarations


namespace {  
  struct Partition  {
    IDataProviderSvc*        dataProvider;
    IDataManagerSvc*         dataManager;
    tbb::mutex               storeMutex;
    std::vector<std::string> newDataObjects;
    int                      eventNumber;
    Partition() : dataProvider(0), dataManager(0), eventNumber(-1) {}
    Partition(IDataProviderSvc* dp, IDataManagerSvc* dm) : dataProvider(dp), dataManager(dm), eventNumber(-1) {}
    Partition(const Partition& entry) : dataProvider(entry.dataProvider), dataManager(entry.dataManager), eventNumber(entry.eventNumber) {}
    Partition& operator=(const Partition& entry)  {
      dataProvider = entry.dataProvider;
      dataManager = entry.dataManager;
      return *this;
    }
  };
}

__thread Partition* s_current(0);

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
class HiveWhiteBoard: public extends3<Service,
                                     IDataProviderSvc,
                                     IDataManagerSvc,
                                     IHiveWhiteBoard>
{
protected:
  typedef std::vector<Partition> Partitions;
  
  /// Integer Property corresponding to CLID of root entry
  CLID                m_rootCLID;
  /// Name of root event
  std::string         m_rootName;
  /// Data loader name
  std::string         m_loader;
  /// Pointer to data loader service
  IConversionSvc*     m_dataLoader;
  /// Reference to address creator
  IAddressCreator*    m_addrCreator;
  /// Datastore partitions
  Partitions          m_partitions;
  /// Datastore slots
  size_t              m_slots;

public:
  /// IDataManagerSvc: Accessor for root event CLID
  virtual CLID rootCLID() const {
    return (CLID)m_rootCLID;
  }
  /// Name for root Event
  std::string rootName() const {
    return m_rootName;
  }

// macro to help writing the function calls
#define _CALL(P,F,ARGS) if(s_current) { \
tbb::mutex::scoped_lock lock; lock.acquire(s_current->storeMutex);\
return s_current->P ? s_current->P->F ARGS : IDataProviderSvc::INVALID_ROOT; }\
return IDataProviderSvc::INVALID_ROOT;

  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress(const std::string& path, IOpaqueAddress* pAddr)   {
    _CALL(dataManager, registerAddress, (path, pAddr));
  }
  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress(DataObject* parent, const std::string& path, IOpaqueAddress* pAddr)  {
    _CALL(dataManager, registerAddress, (parent, path, pAddr));
  }
  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress(IRegistry* parent, const std::string& path, IOpaqueAddress* pAdd)  {
    _CALL(dataManager, registerAddress, (parent, path, pAdd));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress(const std::string& path)  {
    _CALL(dataManager, unregisterAddress, (path));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress(DataObject* pParent, const std::string& path)  {
    _CALL(dataManager, unregisterAddress, (pParent, path));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress(IRegistry* pParent, const std::string& path)  {
    _CALL(dataManager, unregisterAddress, (pParent, path));
  }
  /// Explore the object store: retrieve all leaves attached to the object
  virtual StatusCode objectLeaves(const DataObject*  pObject, std::vector<IRegistry*>& leaves)  {
    _CALL(dataManager, objectLeaves, (pObject, leaves));
  }
  /// Explore the object store: retrieve all leaves attached to the object
  virtual StatusCode objectLeaves(const IRegistry* pObject, std::vector<IRegistry*>& leaves)  {
    _CALL(dataManager, objectLeaves, (pObject, leaves));
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual StatusCode objectParent(const DataObject* pObject, IRegistry*& refpParent)  {
    _CALL(dataManager, objectParent, (pObject, refpParent));
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual StatusCode objectParent(const IRegistry* pObject, IRegistry*& refpParent)  {
    _CALL(dataManager, objectParent, (pObject, refpParent));
  }
  /// Remove all data objects below the sub tree identified
  virtual StatusCode clearSubTree(const std::string& path)  {
    _CALL(dataManager, clearSubTree, (path));
  }
  /// Remove all data objects below the sub tree identified
  virtual StatusCode clearSubTree(DataObject* pObject)  {
    _CALL(dataManager, clearSubTree, (pObject));
  }
  /// IDataManagerSvc: Remove all data objects in the data store.
  virtual StatusCode clearStore()  {
    Partitions::iterator i;
    for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).dataManager->clearStore().ignore();
    }
    return StatusCode::SUCCESS;
  }

  
  /// Analyze by traversing all data objects below the sub tree
  virtual StatusCode traverseSubTree(const std::string& path, IDataStoreAgent* pAgent)  {
    _CALL(dataManager, traverseSubTree, (path, pAgent));
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  virtual StatusCode traverseSubTree(DataObject* pObject, IDataStoreAgent* pAgent)  {
    _CALL(dataManager, traverseSubTree, (pObject, pAgent));
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  virtual StatusCode traverseTree( IDataStoreAgent* pAgent )  {
    _CALL(dataManager, traverseTree, (pAgent));
  }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  virtual StatusCode setRoot( const std::string& path, DataObject* pObj)  {
    _CALL(dataManager, setRoot, (path, pObj));
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  virtual StatusCode setRoot (const std::string& path, IOpaqueAddress* pAddr)  {
    _CALL(dataManager, setRoot, (path, pAddr));
  }

  /// IDataManagerSvc: Pass a default data loader to the service.
  virtual StatusCode setDataLoader(IConversionSvc* pDataLoader)  {
    Partitions::iterator i;
    if ( 0 != pDataLoader  ) pDataLoader->addRef();
    if ( 0 != m_dataLoader ) m_dataLoader->release();
    if ( 0 != pDataLoader  )    {
      pDataLoader->setDataProvider(this);
    }
    m_dataLoader = pDataLoader;
    for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).dataManager->setDataLoader(m_dataLoader).ignore();
    }
    return SUCCESS;
  }
  /// Add an item to the preload list
  virtual StatusCode addPreLoadItem(const DataStoreItem& item)    {
    _CALL(dataProvider, addPreLoadItem, (item));
  }
  /// Add an item to the preload list
  virtual StatusCode addPreLoadItem(const std::string& item)   {
    _CALL(dataProvider, addPreLoadItem, (item));
  }
  /// Remove an item from the preload list
  virtual StatusCode removePreLoadItem(const DataStoreItem& item)  {
    _CALL(dataProvider, removePreLoadItem, (item));
  }
  /// Add an item to the preload list
  virtual StatusCode removePreLoadItem(const std::string& item)  {
    _CALL(dataProvider, removePreLoadItem, (item));
  }
  /// Clear the preload list
  virtual StatusCode resetPreLoad() {
    _CALL(dataProvider, resetPreLoad, ());
  }
  /// load all preload items of the list
  virtual StatusCode preLoad()  {
    _CALL(dataProvider, preLoad, ());
  }
  /// Register object with the data store.  (The most common one is the only monitored one for the time being....)
  virtual StatusCode registerObject(const std::string& path, DataObject* pObj)  {
    tbb::mutex::scoped_lock lock; lock.acquire(s_current->storeMutex);
    StatusCode sc = s_current->dataProvider->registerObject(path, pObj);
    if( sc.isSuccess()) {
      s_current->newDataObjects.push_back(path);
    }
    return sc;
  }
  /// Register object with the data store.
  virtual StatusCode registerObject(const std::string& parent, const std::string& obj, DataObject* pObj)  {
    _CALL(dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Register object with the data store.
  virtual StatusCode registerObject(const std::string& parent, int item, DataObject* pObj)  {
    _CALL(dataProvider, registerObject, (parent, item, pObj));
  }
  /// Register object with the data store.
  virtual StatusCode registerObject(DataObject* parent, const std::string& obj, DataObject* pObj)  {
    _CALL(dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Register object with the data store.
  virtual StatusCode registerObject(DataObject* parent, int obj, DataObject* pObj)  {
    _CALL(dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& path)   {
    _CALL(dataProvider, unregisterObject, (path));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& parent, const std::string& obj)  {
    _CALL(dataProvider, unregisterObject, (parent, obj));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& parent, int obj)  {
    _CALL(dataProvider, unregisterObject, (parent, obj));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObj)  {
    _CALL(dataProvider, unregisterObject, (pObj));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObj, const std::string& path)  {
    _CALL(dataProvider, unregisterObject, (pObj, path));
  }
  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObj, int item )  {
    _CALL(dataProvider, unregisterObject, (pObj, item));
  }
  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(IRegistry* parent, const std::string& path, DataObject*& pObj )  {
    _CALL(dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object identified by its full path from the data store.
  virtual StatusCode retrieveObject(const std::string& path, DataObject*& pObj)  {
    _CALL(dataProvider, retrieveObject, (path, pObj));
  }
  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(const std::string& parent, const std::string& path, DataObject*& pObj )  {
    _CALL(dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(const std::string& parent, int item, DataObject*& pObj)  {
    _CALL(dataProvider, retrieveObject, (parent, item, pObj));
  }
  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(DataObject* parent, const std::string& path, DataObject*& pObj )  {
    _CALL(dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(DataObject* parent, int item, DataObject*& pObj )  {
    _CALL(dataProvider, retrieveObject, (parent, item, pObj));
  }
  /// Find object identified by its full path in the data store.
  virtual StatusCode findObject(const std::string& path, DataObject*& pObj)  {
    _CALL(dataProvider, retrieveObject, (path, pObj));
  }
  /// Find object identified by its full path in the data store.
  virtual StatusCode findObject(IRegistry* parent, const std::string& path, DataObject*& pObj)  {
    _CALL(dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Find object in the data store.
  virtual StatusCode findObject(const std::string& parent, const std::string& path, DataObject*& pObj)  {
    _CALL(dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Find object in the data store.
  virtual StatusCode findObject(const std::string& parent, int item, DataObject*& pObject ) {
    _CALL(dataProvider, findObject, (parent, item, pObject));
  }
  /// Find object in the data store.
  virtual StatusCode findObject(DataObject* parent, const std::string& path, DataObject*& pObject)  {
    _CALL(dataProvider, findObject, (parent, path, pObject));
  }
  /// Find object in the data store.
  virtual StatusCode findObject(DataObject* parent, int item, DataObject*& pObject)  {
    _CALL(dataProvider, findObject, (parent, item, pObject));
  }
  /// Add a link to another object.
  virtual StatusCode linkObject(IRegistry* from, const std::string& objPath, DataObject* to)  {
    _CALL(dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual StatusCode linkObject(const std::string& from, const std::string& objPath, DataObject* to)   {
    _CALL(dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual StatusCode linkObject(DataObject* from, const std::string& objPath, DataObject* to)  {
    _CALL(dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual StatusCode linkObject(const std::string& fullPath, DataObject* to)  {
    _CALL(dataProvider, linkObject, (fullPath, to));
  }
  /// Remove a link to another object.
  virtual StatusCode unlinkObject(IRegistry* from, const std::string& objPath)  {
    _CALL(dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual StatusCode unlinkObject(const std::string& from, const std::string& objPath)  {
    _CALL(dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual StatusCode unlinkObject(DataObject* from, const std::string& objPath)  {
    _CALL(dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual StatusCode unlinkObject(const std::string& path) {
    _CALL(dataProvider, unlinkObject, (path));
  }
  /// Update object identified by its directory entry.
  virtual StatusCode updateObject(IRegistry* pDirectory )  {
    _CALL(dataProvider, updateObject, (pDirectory));
  }
  /// Update object.
  virtual StatusCode updateObject(const std::string& path)  {
    _CALL(dataProvider, updateObject, (path));
  }
  /// Update object.
  virtual StatusCode updateObject(DataObject* pObj )  {
    _CALL(dataProvider, updateObject, (pObj));
  }
  /// Update object.
  virtual StatusCode updateObject(const std::string& parent, const std::string& updatePath )  {
    _CALL(dataProvider, updateObject, (parent, updatePath));
  }
  /// Update object.
  virtual StatusCode updateObject(DataObject* parent, const std::string& updatePath)  {
    _CALL(dataProvider, updateObject, (parent, updatePath));
  }

  //
  //---IHiveWhiteBard implemenation--------------------------------------------------
  //

  /// Remove all data objects in one 'slot' of the data store.
  virtual StatusCode clearStore(size_t partition)  {
    return m_partitions[partition].dataManager->clearStore();
  }

  /// Activate a partition object. The  identifies the partition uniquely.
  virtual StatusCode selectStore(size_t partition) {
      s_current = &m_partitions[partition];
      return StatusCode::SUCCESS;
  }
  
  /// Set the number of event slots (copies of DataSvc objects).
  virtual StatusCode setNumberOfStores(size_t slots) {
    if(FSMState() ==  Gaudi::StateMachine::INITIALIZED) {
      warning() << "Too late to change the number of slots!" << endmsg;
      return StatusCode::FAILURE;
    }
    m_slots = slots;
    return StatusCode::SUCCESS;
  }
  
  /// Get the list of new DataObjects in the current store.
  virtual StatusCode getNewDataObjects(std::vector<std::string>& products) {
    tbb::mutex::scoped_lock lock; lock.acquire(s_current->storeMutex);
    products = s_current->newDataObjects;
    s_current->newDataObjects.clear();
    return StatusCode::SUCCESS;
  }

  /// Allocate a store partition for a given event number
  virtual size_t allocateStore( int evtnumber ) {
    size_t free = std::string::npos;
    size_t index = 0;
    for (auto& p : m_partitions ) {
      if( p.eventNumber == evtnumber) {
        error() << "Attempt to allocate a store partition for an event that is still active" << endmsg;
        return std::string::npos;
      } else if (p.eventNumber == -1) {
        p.eventNumber = evtnumber;
        //info() << "Got allocated slot..." << index << endmsg;
        return index;
      }
      index++;
    }
    return std::string::npos;
  }
  
  /// Free a store partition
  virtual StatusCode freeStore( size_t partition ) {
    m_partitions[partition].eventNumber = -1;
    //info() << "Freed slot..." << partition << endmsg;
    return StatusCode::SUCCESS;
  }
  
  
  /// Get the partition number corresponding to a given event
  virtual size_t getPartitionNumber(int eventnumber)  const {
    size_t index;
    for (auto& p : m_partitions) {
      if( p.eventNumber == eventnumber) return index;
      index++;
    }
    return std::string::npos;
  }

  
  //
  //---IService implemenation---------------------------------------------------------
  //
  
  /// Service initialisation
  virtual StatusCode initialize()  {
    StatusCode sc = Service::initialize();
    if ( !sc.isSuccess() )  {
      error() << "Unable to initialize base class" << endmsg;
      return sc;
    }
    for( size_t i = 0; i< m_slots; i++) {
      std::ostringstream oss;
      oss << name() << "_" << i;
      DataSvc* svc = new DataSvc(oss.str(), serviceLocator());
      sc = svc->initialize();
      if (!sc.isSuccess()) {
        error() << "Failed to instantiate DataSvc as store partition" << endmsg;
        return sc;
      }
      m_partitions.push_back(Partition(svc, svc));
    }
    selectStore(0);
    return StatusCode::SUCCESS;
  }

  /// Service initialisation
  virtual StatusCode reinitialize()   {
    StatusCode sc = Service::reinitialize();
    if (!sc.isSuccess()) {
      error() << "Unable to reinitialize base class" << endmsg;
      return sc;
    }
    return StatusCode::SUCCESS;
  }

  /// Service initialisation
  virtual StatusCode finalize()   {
    setDataLoader(0).ignore();
    clearStore().ignore();
    return Service::finalize();
  }


//protected:

  /// Standard Constructor
  HiveWhiteBoard( const std::string& name, ISvcLocator* svc )
  : base_class(name,svc), m_rootCLID(110), m_rootName("/Event"),
    m_dataLoader(0), m_addrCreator(0)
  {
    m_dataLoader = 0;
    declareProperty("RootCLID",         m_rootCLID);
    declareProperty("RootName",         m_rootName);
    declareProperty("DataLoader",       m_loader="EventPersistencySvc");
    declareProperty("EventSlots",       m_slots = 1);
  }

  /// Standard Destructor
  virtual ~HiveWhiteBoard()  {
    setDataLoader(0).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();
    
    for(Partitions::iterator i = m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).dataManager->release();
      (*i).dataProvider->release();
    }
    m_partitions.clear();
  }

};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(HiveWhiteBoard)

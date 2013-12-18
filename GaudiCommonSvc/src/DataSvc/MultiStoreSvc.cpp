//====================================================================
//	MultiStoreSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient event data service.
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  DATASVC_MULTISTORESVC_CPP

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IPartitionControl.h"

#include <map>

// Forward declarations
// This class
class MultiStoreSvc;

typedef const std::string CSTR;
typedef IDataStoreAgent   AGENT;
typedef DataObject        OBJECT;
typedef IOpaqueAddress    ADDRESS;
typedef StatusCode        STATUS;

namespace {

  struct Partition  {
    IDataProviderSvc* dataProvider;
    IDataManagerSvc*  dataManager;
    std::string       name;
    Partition() : dataProvider(0), dataManager(0)
    {
    }
    Partition(const Partition& entry)
      : dataProvider(entry.dataProvider),
        dataManager(entry.dataManager),
        name(entry.name)
    {
    }
    Partition& operator=(const Partition& entry)  {
      dataProvider = entry.dataProvider;
      dataManager = entry.dataManager;
      name = entry.name;
      return *this;
    }
  };
}

/**
 * @class MultiStoreSvc
 *
 * Data service base class. A data service manages the transient data stores
 * and implements the IDataProviderSvc, the IDataManagerSvc and the
 * IPartitionControl interfaces.
 *
 * @author Markus Frank
 * @author Sebastien Ponce
 * @version 1.0
*/
class MultiStoreSvc: public extends3<Service,
                                     IDataProviderSvc,
                                     IDataManagerSvc,
                                     IPartitionControl>
{
protected:
  typedef std::vector<std::string>         PartitionDefs;
  typedef std::map<std::string, Partition> Partitions;
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
  /// Root type (address or object)
  enum { no_type = 0, address_type = 1, object_type = 2};
  struct tagROOT {
    int type;
    std::string path;
    union {
      ADDRESS* address;
      OBJECT*  object;
    } root;
    tagROOT() : type(no_type) { root.address = 0; }
  }                        m_root;
  /// Current partition
  Partition                m_current;
  /// Datastore partitions
  Partitions               m_partitions;
  /// Datastore partition definitions
  PartitionDefs            m_partitionDefs;
  /// Default partition
  std::string              m_defaultPartition;

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
#define _CALL(P,F,ARGS) \
    P ? P->F ARGS : IDataProviderSvc::INVALID_ROOT

  /// IDataManagerSvc: Register object address with the data store.
  virtual STATUS registerAddress(CSTR& path, ADDRESS* pAddr)   {
    return _CALL(m_current.dataManager, registerAddress, (path, pAddr));
  }
  /// IDataManagerSvc: Register object address with the data store.
  virtual STATUS registerAddress(OBJECT* parent, CSTR& path, ADDRESS* pAddr)  {
    return _CALL(m_current.dataManager, registerAddress, (parent, path, pAddr));
  }
  /// IDataManagerSvc: Register object address with the data store.
  virtual STATUS registerAddress(IRegistry* parent, CSTR& path, ADDRESS* pAdd)  {
    return _CALL(m_current.dataManager, registerAddress, (parent, path, pAdd));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual STATUS unregisterAddress(CSTR& path)  {
    return _CALL(m_current.dataManager, unregisterAddress, (path));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual STATUS unregisterAddress(OBJECT* pParent, CSTR& path)  {
    return _CALL(m_current.dataManager, unregisterAddress, (pParent, path));
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  virtual STATUS unregisterAddress(IRegistry* pParent, CSTR& path)  {
    return _CALL(m_current.dataManager, unregisterAddress, (pParent, path));
  }
  /// Explore the object store: retrieve all leaves attached to the object
  virtual STATUS objectLeaves(const OBJECT*  pObject, std::vector<IRegistry*>& leaves)  {
    return _CALL(m_current.dataManager, objectLeaves, (pObject, leaves));
  }
  /// Explore the object store: retrieve all leaves attached to the object
  virtual STATUS objectLeaves(const IRegistry* pObject, std::vector<IRegistry*>& leaves)  {
    return _CALL(m_current.dataManager, objectLeaves, (pObject, leaves));
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual STATUS objectParent(const OBJECT* pObject, IRegistry*& refpParent)  {
    return _CALL(m_current.dataManager, objectParent, (pObject, refpParent));
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual STATUS objectParent(const IRegistry* pObject, IRegistry*& refpParent)  {
    return _CALL(m_current.dataManager, objectParent, (pObject, refpParent));
  }
  /// Remove all data objects below the sub tree identified
  virtual STATUS clearSubTree(CSTR& path)  {
    return _CALL(m_current.dataManager, clearSubTree, (path));
  }
  /// Remove all data objects below the sub tree identified
  virtual STATUS clearSubTree(OBJECT* pObject)  {
    return _CALL(m_current.dataManager, clearSubTree, (pObject));
  }
  /// IDataManagerSvc: Remove all data objects in the data store.
  virtual STATUS clearStore()  {
    Partitions::iterator i;
    for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).second.dataManager->clearStore().ignore();
    }
    if ( m_root.root.object )  {
      switch ( m_root.type )  {
        case address_type:
          m_root.root.address->release();
          break;
        case object_type:
          m_root.root.object->release();
          break;
      }
      m_root.root.object = 0;
    }
    m_root.path = "";
    m_root.type = no_type;
    return STATUS::SUCCESS;
  }
  /// Analyze by traversing all data objects below the sub tree
  virtual STATUS traverseSubTree(CSTR& path, AGENT* pAgent)  {
    return _CALL(m_current.dataManager, traverseSubTree, (path, pAgent));
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  virtual STATUS traverseSubTree(OBJECT* pObject, AGENT* pAgent)  {
    return _CALL(m_current.dataManager, traverseSubTree, (pObject, pAgent));
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  virtual STATUS traverseTree( AGENT* pAgent )  {
    return _CALL(m_current.dataManager, traverseTree, (pAgent));
  }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  virtual STATUS setRoot( CSTR& path, OBJECT* pObj)  {
    if ( m_root.root.object )  {
      switch ( m_root.type )  {
        case address_type:
          m_root.root.address->release();
          break;
        case object_type:
          m_root.root.object->release();
          break;
      }
    }
    m_root.path    = path;
    m_root.type    = object_type;
    m_root.root.object  = pObj;
    preparePartitions();
    return activate(m_defaultPartition);
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  virtual STATUS setRoot (CSTR& path, ADDRESS* pAddr)  {
    if ( m_root.root.object )  {
      switch ( m_root.type )  {
        case address_type:
          m_root.root.address->release();
          break;
        case object_type:
          m_root.root.object->release();
          break;
      }
    }
    m_root.path    = path;
    m_root.type    = address_type;
    m_root.root.address = pAddr;
    if ( m_root.root.address )  {
      m_root.root.address->addRef();
      preparePartitions();
      return activate(m_defaultPartition);
    }
    return STATUS::FAILURE;
  }
  /// IDataManagerSvc: Pass a default data loader to the service.
  virtual STATUS setDataLoader(IConversionSvc* pDataLoader)  {
    Partitions::iterator i;
    if ( 0 != pDataLoader  ) pDataLoader->addRef();
    if ( 0 != m_dataLoader ) m_dataLoader->release();
    if ( 0 != pDataLoader  )    {
      pDataLoader->setDataProvider(this);
    }
    m_dataLoader = pDataLoader;
    for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).second.dataManager->setDataLoader(m_dataLoader).ignore();
    }
    return SUCCESS;
  }
  /// Add an item to the preload list
  virtual STATUS addPreLoadItem(const DataStoreItem& item)    {
    return _CALL(m_current.dataProvider, addPreLoadItem, (item));
  }
  /// Add an item to the preload list
  virtual STATUS addPreLoadItem(CSTR& item)   {
    return _CALL(m_current.dataProvider, addPreLoadItem, (item));
  }
  /// Remove an item from the preload list
  virtual STATUS removePreLoadItem(const DataStoreItem& item)  {
    return _CALL(m_current.dataProvider, removePreLoadItem, (item));
  }
  /// Add an item to the preload list
  virtual STATUS removePreLoadItem(CSTR& item)  {
    return _CALL(m_current.dataProvider, removePreLoadItem, (item));
  }
  /// Clear the preload list
  virtual STATUS resetPreLoad() {
    return _CALL(m_current.dataProvider, resetPreLoad, ());
  }
  /// load all preload items of the list
  virtual STATUS preLoad()  {
    return _CALL(m_current.dataProvider, preLoad, ());
  }
  /// Register object with the data store.
  virtual STATUS registerObject(CSTR& path, OBJECT* pObj)  {
    return registerObject(0, path, pObj);
  }
  /// Register object with the data store.
  virtual STATUS registerObject(CSTR& parent, CSTR& obj, OBJECT* pObj)  {
    return _CALL(m_current.dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Register object with the data store.
  virtual STATUS registerObject(CSTR& parent, int item, OBJECT* pObj)  {
    return _CALL(m_current.dataProvider, registerObject, (parent, item, pObj));
  }
  /// Register object with the data store.
  virtual STATUS registerObject(OBJECT* parent, CSTR& obj, OBJECT* pObj)  {
    return _CALL(m_current.dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Register object with the data store.
  virtual STATUS registerObject(OBJECT* parent, int obj, OBJECT* pObj)  {
    return _CALL(m_current.dataProvider, registerObject, (parent, obj, pObj));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(CSTR& path)   {
    return _CALL(m_current.dataProvider, unregisterObject, (path));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(CSTR& parent, CSTR& obj)  {
    return _CALL(m_current.dataProvider, unregisterObject, (parent, obj));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(CSTR& parent, int obj)  {
    return _CALL(m_current.dataProvider, unregisterObject, (parent, obj));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(OBJECT* pObj)  {
    return _CALL(m_current.dataProvider, unregisterObject, (pObj));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(OBJECT* pObj, CSTR& path)  {
    return _CALL(m_current.dataProvider, unregisterObject, (pObj, path));
  }
  /// Unregister object from the data store.
  virtual STATUS unregisterObject(OBJECT* pObj, int item )  {
    return _CALL(m_current.dataProvider, unregisterObject, (pObj, item));
  }
  /// Retrieve object from data store.
  virtual STATUS retrieveObject(IRegistry* parent, CSTR& path, OBJECT*& pObj )  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object identified by its full path from the data store.
  virtual STATUS retrieveObject(CSTR& path, OBJECT*& pObj)  {
    return _CALL(m_current.dataProvider, retrieveObject, (path, pObj));
  }
  /// Retrieve object from data store.
  virtual STATUS retrieveObject(CSTR& parent, CSTR& path, OBJECT*& pObj )  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object from data store.
  virtual STATUS retrieveObject(CSTR& parent, int item, OBJECT*& pObj)  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, item, pObj));
  }
  /// Retrieve object from data store.
  virtual STATUS retrieveObject(OBJECT* parent, CSTR& path, OBJECT*& pObj )  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Retrieve object from data store.
  virtual STATUS retrieveObject(OBJECT* parent, int item, OBJECT*& pObj )  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, item, pObj));
  }
  /// Find object identified by its full path in the data store.
  virtual STATUS findObject(CSTR& path, OBJECT*& pObj)  {
    return _CALL(m_current.dataProvider, retrieveObject, (path, pObj));
  }
  /// Find object identified by its full path in the data store.
  virtual STATUS findObject(IRegistry* parent, CSTR& path, OBJECT*& pObj)  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Find object in the data store.
  virtual STATUS findObject(CSTR& parent, CSTR& path, OBJECT*& pObj)  {
    return _CALL(m_current.dataProvider, retrieveObject, (parent, path, pObj));
  }
  /// Find object in the data store.
  virtual STATUS findObject(CSTR& parent, int item, OBJECT*& pObject ) {
    return _CALL(m_current.dataProvider, findObject, (parent, item, pObject));
  }
  /// Find object in the data store.
  virtual STATUS findObject(OBJECT* parent, CSTR& path, OBJECT*& pObject)  {
    return _CALL(m_current.dataProvider, findObject, (parent, path, pObject));
  }
  /// Find object in the data store.
  virtual STATUS findObject(OBJECT* parent, int item, OBJECT*& pObject)  {
    return _CALL(m_current.dataProvider, findObject, (parent, item, pObject));
  }
  /// Add a link to another object.
  virtual STATUS linkObject(IRegistry* from, CSTR& objPath, OBJECT* to)  {
    return _CALL(m_current.dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual STATUS linkObject(CSTR& from, CSTR& objPath, OBJECT* to)   {
    return _CALL(m_current.dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual STATUS linkObject(OBJECT* from, CSTR& objPath, OBJECT* to)  {
    return _CALL(m_current.dataProvider, linkObject, (from, objPath, to));
  }
  /// Add a link to another object.
  virtual STATUS linkObject(CSTR& fullPath, OBJECT* to)  {
    return _CALL(m_current.dataProvider, linkObject, (fullPath, to));
  }
  /// Remove a link to another object.
  virtual STATUS unlinkObject(IRegistry* from, CSTR& objPath)  {
    return _CALL(m_current.dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual STATUS unlinkObject(CSTR& from, CSTR& objPath)  {
    return _CALL(m_current.dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual STATUS unlinkObject(OBJECT* from, CSTR& objPath)  {
    return _CALL(m_current.dataProvider, unlinkObject, (from, objPath));
  }
  /// Remove a link to another object.
  virtual STATUS unlinkObject(CSTR& path) {
    return _CALL(m_current.dataProvider, unlinkObject, (path));
  }
  /// Update object identified by its directory entry.
  virtual STATUS updateObject(IRegistry* pDirectory )  {
    return _CALL(m_current.dataProvider, updateObject, (pDirectory));
  }
  /// Update object.
  virtual STATUS updateObject(CSTR& path)  {
    return _CALL(m_current.dataProvider, updateObject, (path));
  }
  /// Update object.
  virtual STATUS updateObject(OBJECT* pObj )  {
    return _CALL(m_current.dataProvider, updateObject, (pObj));
  }
  /// Update object.
  virtual STATUS updateObject(CSTR& parent, CSTR& updatePath )  {
    return _CALL(m_current.dataProvider, updateObject, (parent, updatePath));
  }
  /// Update object.
  virtual STATUS updateObject(OBJECT* parent, CSTR& updatePath)  {
    return _CALL(m_current.dataProvider, updateObject, (parent, updatePath));
  }

  /// Create a partition object. The name identifies the partition uniquely
  virtual STATUS create(CSTR& nam, CSTR& typ)  {
    IInterface* pPartition = 0;
    return create(nam, typ, pPartition);
  }

  /// Create a partition object. The name identifies the partition uniquely
  virtual STATUS create(CSTR& nam, CSTR& typ, IInterface*& pPartition)  {
    STATUS sc = get(nam, pPartition);
    if ( !sc.isSuccess() )  {
      Gaudi::Utils::TypeNameString item(typ);
      /// @FIXME: In the old implementation the services were "unmanaged" (non-active)
      SmartIF<IService>& isvc = serviceLocator()->service(typ);
      if (isvc.isValid())  {
        SmartIF<IDataManagerSvc> dataMgr(isvc);
        SmartIF<IDataProviderSvc> dataProv(isvc);
        if ( dataMgr.isValid() && dataProv.isValid() )  {
          Partition p;
          p.name         = nam;
          p.dataManager  = dataMgr;
          p.dataProvider = dataProv;
          p.dataManager->addRef();
          p.dataProvider->addRef();
          m_partitions.insert(std::make_pair(nam, p));
          return STATUS::SUCCESS;
        }
        else  {
          // Error
          return NO_INTERFACE;
        }
      }
      else {
        // Error
        return NO_INTERFACE;
      }
    }
    return PARTITION_EXISTS;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  virtual STATUS drop(CSTR& nam)   {
    Partitions::iterator i = m_partitions.find(nam);
    if ( i != m_partitions.end() )  {
      if ( (*i).second.dataManager == m_current.dataManager )  {
        m_current = Partition();
      }
      (*i).second.dataManager->clearStore().ignore();
      (*i).second.dataProvider->release();
      (*i).second.dataManager->release();
      m_partitions.erase(i);
      return STATUS::SUCCESS;
    }
    return PARTITION_NOT_PRESENT;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  virtual STATUS drop(IInterface* pPartition)  {
    SmartIF<IDataProviderSvc> provider(pPartition);
    if ( provider.isValid() )  {
      Partitions::iterator i;
      for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
        if ( (*i).second.dataProvider == provider )  {
          (*i).second.dataManager->clearStore().ignore();
          (*i).second.dataProvider->release();
          (*i).second.dataManager->release();
          m_partitions.erase(i);
          return STATUS::SUCCESS;
        }
      }
      return PARTITION_NOT_PRESENT;
    }
    return NO_INTERFACE;
  }

  /// Activate a partition object. The name identifies the partition uniquely.
  virtual STATUS activate(CSTR& nam) {
    Partitions::const_iterator i = m_partitions.find(nam);
    if ( i != m_partitions.end() )  {
      m_current = (*i).second;
      return STATUS::SUCCESS;
    }
    m_current = Partition();
    return PARTITION_NOT_PRESENT;
  }

  /// Activate a partition object.
  virtual STATUS activate(IInterface* pPartition) {
    SmartIF<IDataProviderSvc> provider(pPartition);
    m_current = Partition();
    if ( provider )  {
      Partitions::iterator i;
      for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
        if ( (*i).second.dataProvider == provider )  {
          m_current = (*i).second;
          return STATUS::SUCCESS;
        }
      }
      return PARTITION_NOT_PRESENT;
    }
    return NO_INTERFACE;
  }

  /// Access a partition object. The name identifies the partition uniquely.
  virtual STATUS get(CSTR& nam, IInterface*& pPartition) const  {
    Partitions::const_iterator i = m_partitions.find(nam);
    if ( i != m_partitions.end() )  {
      pPartition = (*i).second.dataProvider;
      return STATUS::SUCCESS;
    }
    pPartition = 0;
    return PARTITION_NOT_PRESENT;
  }

  /// Access the active partition object.
  virtual StatusCode activePartition(std::string& nam, IInterface*& pPartition) const  {
    if ( m_current.dataProvider )  {
      nam = m_current.name;
      pPartition = m_current.dataProvider;
      return STATUS::SUCCESS;
    }
    nam = "";
    pPartition = 0;
    return NO_ACTIVE_PARTITION;
  }

  STATUS attachServices()  {
    MsgStream log(msgSvc(), name());
    // Attach address creator facility
    STATUS sc = service(m_loader, m_addrCreator, true);
    if (!sc.isSuccess()) {
      log << MSG::ERROR
          << "Failed to retrieve data loader "
          << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    IConversionSvc* dataLoader = 0;
    // Attach data loader facility
    sc = service(m_loader, dataLoader, true);
    if (!sc.isSuccess()) {
      log << MSG::ERROR << "Failed to retrieve data loader "
          << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    sc = setDataLoader(dataLoader);
    dataLoader->release();
    if (!sc.isSuccess()) {
      log << MSG::ERROR << "Failed to set data loader "
          << "\"" << m_loader << "\"" << endmsg;
      return sc;
    }
    return sc;
  }

  STATUS detachServices()  {
    if ( m_addrCreator )  m_addrCreator->release();
    if ( m_dataLoader )  m_dataLoader->release();
    m_addrCreator = 0;
    m_dataLoader = 0;
    return STATUS::SUCCESS;
  }

  /// Service initialisation
  virtual STATUS initialize()    {
    // Nothing to do: just call base class initialisation
    STATUS sc = Service::initialize();
    if ( !sc.isSuccess() )  {
      return sc;
    }
    sc = makePartitions();
    if (!sc.isSuccess()) {
      MsgStream log(msgSvc(), name());
      log << MSG::ERROR << "Failed to connect to all store partitions." << endmsg;
      return sc;
    }
    return attachServices();
  }

  /// Service initialisation
  virtual STATUS reinitialize()   {
    STATUS sc = Service::reinitialize();
    MsgStream log(msgSvc(), name());
    if (!sc.isSuccess()) {
      log << MSG::ERROR << "Enable to reinitialize base class"
          << endmsg;
      return sc;
    }
    detachServices();
    sc = attachServices();
    if ( !sc.isSuccess() )  {
      log << MSG::ERROR << "Failed to attach necessary services." << endmsg;
      return sc;
    }
    sc = makePartitions();
    if (!sc.isSuccess()) {
      log << MSG::ERROR << "Failed to connect to store partitions." << endmsg;
      return sc;
    }
    // return
    return STATUS::SUCCESS;
  }

  /// Service initialisation
  virtual STATUS finalize()   {
    setDataLoader(0).ignore();
    clearStore().ignore();
    clearPartitions().ignore();
    m_current = Partition();
    detachServices();
    return Service::finalize();
  }


//protected:

  /// Standard Constructor
  MultiStoreSvc( CSTR& name, ISvcLocator* svc )
  : base_class(name,svc), m_rootCLID(110), m_rootName("/Event"),
    m_dataLoader(0), m_addrCreator(0)
  {
    m_dataLoader = 0;
    declareProperty("RootCLID",         m_rootCLID);
    declareProperty("RootName",         m_rootName);
    declareProperty("Partitions",       m_partitionDefs);
    declareProperty("DataLoader",       m_loader="EventPersistencySvc");
    declareProperty("DefaultPartition", m_defaultPartition="Default");
  }

  /// Standard Destructor
  virtual ~MultiStoreSvc()  {
    setDataLoader(0).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();
    clearPartitions().ignore();
  }

  /// Prepare partition for usage
  STATUS preparePartitions()  {
    STATUS iret = STATUS::SUCCESS;
    for(Partitions::iterator i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      STATUS sc = STATUS::FAILURE;
      switch ( m_root.type )  {
        case address_type:
          if ( m_root.root.address )  {
            ADDRESS* pAdd = 0;
            ADDRESS* p = m_root.root.address;
            sc = m_addrCreator->createAddress(p->svcType(),
                                              p->clID(),
                                              p->par(),
                                              p->ipar(),
                                              pAdd);
            if ( sc.isSuccess() )  {
            sc = (*i).second.dataManager->setRoot(m_root.path, pAdd);
            }
          }
          break;
        case object_type:
          if ( m_root.root.object )  {
            if ( m_root.root.object->clID() == CLID_DataObject )  {
              DataObject* pObj = new DataObject();
              sc = (*i).second.dataManager->setRoot(m_root.path, pObj);
            }
          }
          break;
        default:
          sc = STATUS::FAILURE;
          break;
      }
      if ( !sc.isSuccess() )  {
        iret = sc;
      }
    }
    return iret;
  }

  /// Clear all partitions
  STATUS clearPartitions()  {
    Partitions::iterator i;
    for(i=m_partitions.begin(); i != m_partitions.end(); ++i) {
      (*i).second.dataManager->clearStore().ignore();
      (*i).second.dataProvider->release();
      (*i).second.dataManager->release();
    }
    m_partitions.clear();
    return STATUS::SUCCESS;
  }

  /// Create all partitions according to job options
  STATUS makePartitions()  {
    std::string typ, nam;
    PartitionDefs::iterator j;
    clearPartitions().ignore();
    for(j=m_partitionDefs.begin(); j != m_partitionDefs.end(); ++j)  {
      Tokenizer tok(true);
      Tokenizer::Items::iterator i;
      tok.analyse(*j, " ", "", "", "=", "'", "'");
      for(i = tok.items().begin(); i != tok.items().end(); i++ )   {
        CSTR& t = (*i).tag();
        CSTR& v = (*i).value();
        switch( ::toupper(t[0]) )    {
        case 'N':
          nam = v;
          break;
        case 'T':
          typ = v;
          break;
        }
      }
      STATUS sc = create(nam, typ);
      if ( !sc.isSuccess() )  {
        return sc;
      }
      else if ( !m_defaultPartition.length() )  {
        m_defaultPartition = nam;
      }
    }
    return STATUS::SUCCESS;
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(MultiStoreSvc)

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
#define DATASVC_MULTISTORESVC_CPP

// Include files
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IPartitionControl.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"

#include <map>

// Forward declarations
// This class
class MultiStoreSvc;

typedef const std::string CSTR;
typedef IDataStoreAgent   AGENT;
typedef DataObject        OBJECT;
typedef IOpaqueAddress    ADDRESS;
typedef StatusCode        STATUS;

namespace
{
  struct Partition final {
    SmartIF<IDataProviderSvc> dataProvider;
    SmartIF<IDataManagerSvc>  dataManager;
    std::string               name;
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
class MultiStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc, IPartitionControl>
{
protected:
  typedef std::vector<std::string> PartitionDefs;
  typedef std::map<std::string, Partition> Partitions;

  Gaudi::Property<CLID>          m_rootCLID{this, "RootCLID", 110, "CLID of root entry"};
  Gaudi::Property<std::string>   m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<PartitionDefs> m_partitionDefs{this, "Partitions", {}, "datastore partition definitions"};
  Gaudi::Property<std::string>   m_loader{this, "DataLoader", "EventPersistencySvc", "data loader name"};
  Gaudi::Property<std::string>   m_defaultPartition{this, "DefaultPartition", "Default", "default partition name"};

  /// Pointer to data loader service
  SmartIF<IConversionSvc> m_dataLoader;
  /// Reference to address creator
  SmartIF<IAddressCreator> m_addrCreator;
  /// Root type (address or object)
  enum { no_type = 0, address_type = 1, object_type = 2 };
  struct tagROOT {
    int         type = no_type;
    std::string path;
    union {
      ADDRESS* address;
      OBJECT*  object;
    } root;
    tagROOT() { root.address = nullptr; }
  } m_root;
  /// Current partition
  Partition m_current;
  /// Datastore partitions
  Partitions m_partitions;

  // member templates to help writing the function calls
  template <typename... Args, typename... UArgs>
  STATUS call_( STATUS ( IDataProviderSvc::*pmf )( Args... ), UArgs&&... args )
  {
    return m_current.dataProvider ? ( m_current.dataProvider->*pmf )( std::forward<UArgs>( args )... )
                                  : IDataProviderSvc::INVALID_ROOT;
  }
  template <typename... Args, typename... UArgs>
  STATUS call_( STATUS ( IDataManagerSvc::*pmf )( Args... ), UArgs&&... args )
  {
    return m_current.dataManager ? ( m_current.dataManager->*pmf )( std::forward<UArgs>( args )... )
                                 : IDataProviderSvc::INVALID_ROOT;
  }

public:
  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override { return m_rootCLID; }
  /// Name for root Event
  const std::string& rootName() const override { return m_rootName; }

  /// IDataManagerSvc: Register object address with the data store.
  STATUS registerAddress( CSTR& path, ADDRESS* pAddr ) override
  {
    return call_<CSTR&, ADDRESS*>( &IDataManagerSvc::registerAddress, path, pAddr );
  }
  /// IDataManagerSvc: Register object address with the data store.
  STATUS registerAddress( OBJECT* parent, CSTR& path, ADDRESS* pAddr ) override
  {
    return call_<OBJECT*, CSTR&, ADDRESS*>( &IDataManagerSvc::registerAddress, parent, path, pAddr );
  }
  /// IDataManagerSvc: Register object address with the data store.
  STATUS registerAddress( IRegistry* parent, CSTR& path, ADDRESS* pAdd ) override
  {
    return call_<IRegistry*, CSTR&, ADDRESS*>( &IDataManagerSvc::registerAddress, parent, path, pAdd );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  STATUS unregisterAddress( CSTR& path ) override { return call_<CSTR&>( &IDataManagerSvc::unregisterAddress, path ); }
  /// IDataManagerSvc: Unregister object address from the data store.
  STATUS unregisterAddress( OBJECT* pParent, CSTR& path ) override
  {
    return call_<OBJECT*, CSTR&>( &IDataManagerSvc::unregisterAddress, pParent, path );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  STATUS unregisterAddress( IRegistry* pParent, CSTR& path ) override
  {
    return call_<IRegistry*, CSTR&>( &IDataManagerSvc::unregisterAddress, pParent, path );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  STATUS objectLeaves( const OBJECT* pObject, std::vector<IRegistry*>& leaves ) override
  {
    return call_<const OBJECT*, std::vector<IRegistry*>&>( &IDataManagerSvc::objectLeaves, pObject, leaves );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  STATUS objectLeaves( const IRegistry* pObject, std::vector<IRegistry*>& leaves ) override
  {
    return call_<const IRegistry*, std::vector<IRegistry*>&>( &IDataManagerSvc::objectLeaves, pObject, leaves );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  STATUS objectParent( const OBJECT* pObject, IRegistry*& refpParent ) override
  {
    return call_<const OBJECT*, IRegistry*&>( &IDataManagerSvc::objectParent, pObject, refpParent );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  STATUS objectParent( const IRegistry* pObject, IRegistry*& refpParent ) override
  {
    return call_<const IRegistry*, IRegistry*&>( &IDataManagerSvc::objectParent, pObject, refpParent );
  }
  /// Remove all data objects below the sub tree identified
  STATUS clearSubTree( CSTR& path ) override { return call_<CSTR&>( &IDataManagerSvc::clearSubTree, path ); }
  /// Remove all data objects below the sub tree identified
  STATUS clearSubTree( OBJECT* pObject ) override { return call_<OBJECT*>( &IDataManagerSvc::clearSubTree, pObject ); }
  /// IDataManagerSvc: Remove all data objects in the data store.
  STATUS clearStore() override
  {
    for ( auto& i : m_partitions ) {
      i.second.dataManager->clearStore().ignore();
    }
    if ( m_root.root.object ) {
      switch ( m_root.type ) {
      case address_type:
        m_root.root.address->release();
        break;
      case object_type:
        m_root.root.object->release();
        break;
      }
      m_root.root.object = nullptr;
    }
    m_root.path.clear();
    m_root.type = no_type;
    return STATUS::SUCCESS;
  }
  /// Analyze by traversing all data objects below the sub tree
  STATUS traverseSubTree( CSTR& path, AGENT* pAgent ) override
  {
    return call_<CSTR&, AGENT*>( &IDataManagerSvc::traverseSubTree, path, pAgent );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  STATUS traverseSubTree( OBJECT* pObject, AGENT* pAgent ) override
  {
    return call_<OBJECT*, AGENT*>( &IDataManagerSvc::traverseSubTree, pObject, pAgent );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  STATUS traverseTree( AGENT* pAgent ) override { return call_<AGENT*>( &IDataManagerSvc::traverseTree, pAgent ); }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  STATUS setRoot( std::string path, OBJECT* pObj ) override
  {
    if ( m_root.root.object ) {
      switch ( m_root.type ) {
      case address_type:
        m_root.root.address->release();
        break;
      case object_type:
        m_root.root.object->release();
        break;
      }
    }
    m_root.path        = std::move( path );
    m_root.type        = object_type;
    m_root.root.object = pObj;
    preparePartitions();
    return activate( m_defaultPartition );
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  STATUS setRoot( std::string path, ADDRESS* pAddr ) override
  {
    if ( m_root.root.object ) {
      switch ( m_root.type ) {
      case address_type:
        m_root.root.address->release();
        break;
      case object_type:
        m_root.root.object->release();
        break;
      }
    }
    m_root.path         = std::move( path );
    m_root.type         = address_type;
    m_root.root.address = pAddr;
    if ( m_root.root.address ) {
      m_root.root.address->addRef();
      preparePartitions();
      return activate( m_defaultPartition );
    }
    return STATUS::FAILURE;
  }
  /// IDataManagerSvc: Pass a default data loader to the service.
  STATUS setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc = nullptr ) override
  {
    m_dataLoader = pDataLoader;
    if ( m_dataLoader ) m_dataLoader->setDataProvider( dpsvc ? dpsvc : this );
    for ( auto& i : m_partitions ) {
      i.second.dataManager->setDataLoader( m_dataLoader.get() ).ignore();
    }
    return SUCCESS;
  }
  /// Add an item to the preload list
  STATUS addPreLoadItem( const DataStoreItem& item ) override
  {
    return call_<const DataStoreItem&>( &IDataProviderSvc::addPreLoadItem, item );
  }
  /// Add an item to the preload list
  STATUS addPreLoadItem( CSTR& item ) override { return call_<CSTR&>( &IDataProviderSvc::addPreLoadItem, item ); }
  /// Remove an item from the preload list
  STATUS removePreLoadItem( const DataStoreItem& item ) override
  {
    return call_<const DataStoreItem&>( &IDataProviderSvc::removePreLoadItem, item );
  }
  /// Add an item to the preload list
  STATUS removePreLoadItem( CSTR& item ) override { return call_<CSTR&>( &IDataProviderSvc::removePreLoadItem, item ); }
  /// Clear the preload list
  STATUS resetPreLoad() override { return call_<>( &IDataProviderSvc::resetPreLoad ); }
  /// load all preload items of the list
  STATUS preLoad() override { return call_<>( &IDataProviderSvc::preLoad ); }
  /// Register object with the data store.
  STATUS registerObject( CSTR& path, OBJECT* pObj ) override { return registerObject( nullptr, path, pObj ); }
  /// Register object with the data store.
  STATUS registerObject( CSTR& parent, CSTR& obj, OBJECT* pObj ) override
  {
    return call_<CSTR&, CSTR&, OBJECT*>( &IDataProviderSvc::registerObject, parent, obj, pObj );
  }
  /// Register object with the data store.
  STATUS registerObject( CSTR& parent, int item, OBJECT* pObj ) override
  {
    return call_<CSTR&, int, OBJECT*>( &IDataProviderSvc::registerObject, parent, item, pObj );
  }
  /// Register object with the data store.
  STATUS registerObject( OBJECT* parent, CSTR& obj, OBJECT* pObj ) override
  {
    return call_<OBJECT*, CSTR&, OBJECT*>( &IDataProviderSvc::registerObject, parent, obj, pObj );
  }
  /// Register object with the data store.
  STATUS registerObject( OBJECT* parent, int obj, OBJECT* pObj ) override
  {
    return call_<OBJECT*, int, OBJECT*>( &IDataProviderSvc::registerObject, parent, obj, pObj );
  }
  /// Unregister object from the data store.
  STATUS unregisterObject( CSTR& path ) override { return call_<CSTR&>( &IDataProviderSvc::unregisterObject, path ); }
  /// Unregister object from the data store.
  STATUS unregisterObject( CSTR& parent, CSTR& obj ) override
  {
    return call_<CSTR&, CSTR&>( &IDataProviderSvc::unregisterObject, parent, obj );
  }
  /// Unregister object from the data store.
  STATUS unregisterObject( CSTR& parent, int obj ) override
  {
    return call_<CSTR&, int>( &IDataProviderSvc::unregisterObject, parent, obj );
  }
  /// Unregister object from the data store.
  STATUS unregisterObject( OBJECT* pObj ) override
  {
    return call_<OBJECT*>( &IDataProviderSvc::unregisterObject, pObj );
  }
  /// Unregister object from the data store.
  STATUS unregisterObject( OBJECT* pObj, CSTR& path ) override
  {
    return call_<OBJECT*, CSTR&>( &IDataProviderSvc::unregisterObject, pObj, path );
  }
  /// Unregister object from the data store.
  STATUS unregisterObject( OBJECT* pObj, int item ) override
  {
    return call_<OBJECT*, int>( &IDataProviderSvc::unregisterObject, pObj, item );
  }
  /// Retrieve object from data store.
  STATUS retrieveObject( IRegistry* parent, CSTR& path, OBJECT*& pObj ) override
  {
    return call_<IRegistry*, CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, path, pObj );
  }
  /// Retrieve object identified by its full path from the data store.
  STATUS retrieveObject( CSTR& path, OBJECT*& pObj ) override
  {
    return call_<CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, path, pObj );
  }
  /// Retrieve object from data store.
  STATUS retrieveObject( CSTR& parent, CSTR& path, OBJECT*& pObj ) override
  {
    return call_<CSTR&, CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, path, pObj );
  }
  /// Retrieve object from data store.
  STATUS retrieveObject( CSTR& parent, int item, OBJECT*& pObj ) override
  {
    return call_<CSTR&, int, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, item, pObj );
  }
  /// Retrieve object from data store.
  STATUS retrieveObject( OBJECT* parent, CSTR& path, OBJECT*& pObj ) override
  {
    return call_<OBJECT*, CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, path, pObj );
  }
  /// Retrieve object from data store.
  STATUS retrieveObject( OBJECT* parent, int item, OBJECT*& pObj ) override
  {
    return call_<OBJECT*, int, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, item, pObj );
  }
  /// Find object identified by its full path in the data store.
  STATUS findObject( CSTR& path, OBJECT*& pObj ) override
  {
    return call_<CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, path, pObj );
  }
  /// Find object identified by its full path in the data store.
  STATUS findObject( IRegistry* parent, CSTR& path, OBJECT*& pObj ) override
  {
    return call_<IRegistry*, CSTR&, OBJECT*&>( &IDataProviderSvc::findObject, parent, path, pObj );
  }
  /// Find object in the data store.
  STATUS findObject( CSTR& parent, CSTR& path, OBJECT*& pObj ) override
  {
    return call_<CSTR&, CSTR&, OBJECT*&>( &IDataProviderSvc::retrieveObject, parent, path, pObj );
  }
  /// Find object in the data store.
  STATUS findObject( CSTR& parent, int item, OBJECT*& pObject ) override
  {
    return call_<CSTR&, int, OBJECT*&>( &IDataProviderSvc::findObject, parent, item, pObject );
  }
  /// Find object in the data store.
  STATUS findObject( OBJECT* parent, CSTR& path, OBJECT*& pObject ) override
  {
    return call_<OBJECT*, CSTR&, OBJECT*&>( &IDataProviderSvc::findObject, parent, path, pObject );
  }
  /// Find object in the data store.
  STATUS findObject( OBJECT* parent, int item, OBJECT*& pObject ) override
  {
    return call_<OBJECT*, int, OBJECT*&>( &IDataProviderSvc::findObject, parent, item, pObject );
  }
  /// Add a link to another object.
  STATUS linkObject( IRegistry* from, CSTR& objPath, OBJECT* to ) override
  {
    return call_<IRegistry*, CSTR&, OBJECT*>( &IDataProviderSvc::linkObject, from, objPath, to );
  }
  /// Add a link to another object.
  STATUS linkObject( CSTR& from, CSTR& objPath, OBJECT* to ) override
  {
    return call_<CSTR&, CSTR&, OBJECT*>( &IDataProviderSvc::linkObject, from, objPath, to );
  }
  /// Add a link to another object.
  STATUS linkObject( OBJECT* from, CSTR& objPath, OBJECT* to ) override
  {
    return call_<OBJECT*, CSTR&, OBJECT*>( &IDataProviderSvc::linkObject, from, objPath, to );
  }
  /// Add a link to another object.
  STATUS linkObject( CSTR& fullPath, OBJECT* to ) override
  {
    return call_<CSTR&, OBJECT*>( &IDataProviderSvc::linkObject, fullPath, to );
  }
  /// Remove a link to another object.
  STATUS unlinkObject( IRegistry* from, CSTR& objPath ) override
  {
    return call_<IRegistry*, CSTR&>( &IDataProviderSvc::unlinkObject, from, objPath );
  }
  /// Remove a link to another object.
  STATUS unlinkObject( CSTR& from, CSTR& objPath ) override
  {
    return call_<CSTR&, CSTR&>( &IDataProviderSvc::unlinkObject, from, objPath );
  }
  /// Remove a link to another object.
  STATUS unlinkObject( OBJECT* from, CSTR& objPath ) override
  {
    return call_<OBJECT*, CSTR&>( &IDataProviderSvc::unlinkObject, from, objPath );
  }
  /// Remove a link to another object.
  STATUS unlinkObject( CSTR& path ) override { return call_<CSTR&>( &IDataProviderSvc::unlinkObject, path ); }
  /// Update object identified by its directory entry.
  STATUS updateObject( IRegistry* pDirectory ) override
  {
    return call_<IRegistry*>( &IDataProviderSvc::updateObject, pDirectory );
  }
  /// Update object.
  STATUS updateObject( CSTR& path ) override { return call_<CSTR&>( &IDataProviderSvc::updateObject, path ); }
  /// Update object.
  STATUS updateObject( OBJECT* pObj ) override { return call_<OBJECT*>( &IDataProviderSvc::updateObject, pObj ); }
  /// Update object.
  STATUS updateObject( CSTR& parent, CSTR& updatePath ) override
  {
    return call_<CSTR&, CSTR&>( &IDataProviderSvc::updateObject, parent, updatePath );
  }
  /// Update object.
  STATUS updateObject( OBJECT* parent, CSTR& updatePath ) override
  {
    return call_<OBJECT*, CSTR&>( &IDataProviderSvc::updateObject, parent, updatePath );
  }

  /// Create a partition object. The name identifies the partition uniquely
  STATUS create( CSTR& nam, CSTR& typ ) override
  {
    IInterface* pPartition = nullptr;
    return create( nam, typ, pPartition );
  }
  /// Create a partition object. The name identifies the partition uniquely
  STATUS create( CSTR& nam, CSTR& typ, IInterface*& pPartition ) override
  {
    if ( get( nam, pPartition ).isSuccess() ) return PARTITION_EXISTS;
    /// @FIXME: In the old implementation the services were "unmanaged" (non-active)
    auto isvc = serviceLocator()->service<IService>( typ );
    if ( !isvc ) return NO_INTERFACE;
    auto dataMgr  = isvc.as<IDataManagerSvc>();
    auto dataProv = isvc.as<IDataProviderSvc>();
    if ( !dataMgr || !dataProv ) return NO_INTERFACE;
    m_partitions.emplace( nam, Partition{dataProv, dataMgr, nam} );
    return STATUS::SUCCESS;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop( CSTR& nam ) override
  {
    auto i = m_partitions.find( nam );
    if ( i == m_partitions.end() ) return PARTITION_NOT_PRESENT;
    if ( i->second.dataManager == m_current.dataManager ) {
      m_current = Partition();
    }
    i->second.dataManager->clearStore().ignore();
    m_partitions.erase( i );
    return STATUS::SUCCESS;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop( IInterface* pPartition ) override
  {
    auto provider = SmartIF<IDataProviderSvc>( pPartition );
    if ( !provider ) return NO_INTERFACE;
    auto i = std::find_if( std::begin( m_partitions ), std::end( m_partitions ),
                           [&]( Partitions::const_reference p ) { return p.second.dataProvider == provider; } );
    if ( i == std::end( m_partitions ) ) return PARTITION_NOT_PRESENT;
    i->second.dataManager->clearStore().ignore();
    m_partitions.erase( i );
    return STATUS::SUCCESS;
  }

  /// Activate a partition object. The name identifies the partition uniquely.
  STATUS activate( CSTR& nam ) override
  {
    auto i = m_partitions.find( nam );
    if ( i != m_partitions.end() ) {
      m_current = i->second;
      return STATUS::SUCCESS;
    }
    m_current = Partition();
    return PARTITION_NOT_PRESENT;
  }

  /// Activate a partition object.
  STATUS activate( IInterface* pPartition ) override
  {
    auto provider = SmartIF<IDataProviderSvc>( pPartition );
    m_current     = Partition();
    if ( !provider ) return NO_INTERFACE;
    auto i = std::find_if( std::begin( m_partitions ), std::end( m_partitions ),
                           [&]( Partitions::const_reference p ) { return p.second.dataProvider == provider; } );
    if ( i == std::end( m_partitions ) ) return PARTITION_NOT_PRESENT;
    m_current = i->second;
    return STATUS::SUCCESS;
  }

  /// Access a partition object. The name identifies the partition uniquely.
  STATUS get( CSTR& nam, IInterface*& pPartition ) const override
  {
    auto i = m_partitions.find( nam );
    if ( i != m_partitions.end() ) {
      pPartition = i->second.dataProvider;
      return STATUS::SUCCESS;
    }
    pPartition = nullptr;
    return PARTITION_NOT_PRESENT;
  }

  /// Access the active partition object.
  StatusCode activePartition( std::string& nam, IInterface*& pPartition ) const override
  {
    if ( m_current.dataProvider ) {
      nam        = m_current.name;
      pPartition = m_current.dataProvider;
      return STATUS::SUCCESS;
    }
    nam.clear();
    pPartition = nullptr;
    return NO_ACTIVE_PARTITION;
  }

  STATUS attachServices()
  {
    // Attach address creator facility
    m_addrCreator = service( m_loader, true );
    if ( !m_addrCreator ) {
      error() << "Failed to retrieve data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return StatusCode::FAILURE;
    }
    // Attach data loader facility
    auto dataLoader = service<IConversionSvc>( m_loader, true );
    if ( !dataLoader ) {
      error() << "Failed to retrieve data loader "
              << "\"" << m_loader << "\"" << endmsg;
      return StatusCode::FAILURE;
    }
    auto sc = setDataLoader( dataLoader.get() );
    if ( !sc.isSuccess() ) {
      error() << "Failed to set data loader "
              << "\"" << m_loader << "\"" << endmsg;
    }
    return sc;
  }

  STATUS detachServices()
  {
    m_addrCreator.reset();
    m_dataLoader.reset();
    return STATUS::SUCCESS;
  }

  /// Service initialisation
  STATUS initialize() override
  {
    // Nothing to do: just call base class initialisation
    STATUS sc = Service::initialize();
    if ( !sc.isSuccess() ) return sc;
    sc = makePartitions();
    if ( !sc.isSuccess() ) {
      error() << "Failed to connect to all store partitions." << endmsg;
      return sc;
    }
    return attachServices();
  }

  /// Service initialisation
  STATUS reinitialize() override
  {
    STATUS sc = Service::reinitialize();
    if ( !sc.isSuccess() ) {
      error() << "Enable to reinitialize base class" << endmsg;
      return sc;
    }
    detachServices();
    sc = attachServices();
    if ( !sc.isSuccess() ) {
      error() << "Failed to attach necessary services." << endmsg;
      return sc;
    }
    sc = makePartitions();
    if ( !sc.isSuccess() ) {
      error() << "Failed to connect to store partitions." << endmsg;
      return sc;
    }
    // return
    return STATUS::SUCCESS;
  }

  /// Service initialisation
  STATUS finalize() override
  {
    setDataLoader( nullptr ).ignore();
    clearStore().ignore();
    clearPartitions().ignore();
    m_current = Partition();
    detachServices();
    return Service::finalize();
  }

  // protected:

  /// Inherited constructor
  using extends::extends;

  /// Standard Destructor
  ~MultiStoreSvc() override
  {
    setDataLoader( nullptr ).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();
    clearPartitions().ignore();
  }

  /// Prepare partition for usage
  STATUS preparePartitions()
  {
    STATUS iret = STATUS::SUCCESS;
    for ( auto& i : m_partitions ) {
      STATUS sc = STATUS::FAILURE;
      switch ( m_root.type ) {
      case address_type:
        if ( m_root.root.address ) {
          ADDRESS* pAdd = nullptr;
          ADDRESS* p    = m_root.root.address;
          sc            = m_addrCreator->createAddress( p->svcType(), p->clID(), p->par(), p->ipar(), pAdd );
          if ( sc.isSuccess() ) {
            sc = i.second.dataManager->setRoot( m_root.path, pAdd );
          }
        }
        break;
      case object_type:
        if ( m_root.root.object ) {
          if ( m_root.root.object->clID() == CLID_DataObject ) {
            sc = i.second.dataManager->setRoot( m_root.path, new DataObject() );
          }
        }
        break;
      default:
        sc = STATUS::FAILURE;
        break;
      }
      if ( !sc.isSuccess() ) iret = sc;
    }
    return iret;
  }

  /// Clear all partitions
  STATUS clearPartitions()
  {
    for ( auto& i : m_partitions ) i.second.dataManager->clearStore().ignore();
    m_partitions.clear();
    return STATUS::SUCCESS;
  }

  /// Create all partitions according to job options
  STATUS makePartitions()
  {
    using Parser = Gaudi::Utils::AttribStringParser;
    std::string typ, nam;
    clearPartitions().ignore();
    for ( auto part : m_partitionDefs ) {
      for ( auto attrib : Parser( std::move( part ) ) ) {
        switch (::toupper( attrib.tag[0] ) ) {
        case 'N':
          nam = std::move( attrib.value );
          break;
        case 'T':
          typ = std::move( attrib.value );
          break;
        }
      }
      STATUS sc = create( nam, typ );
      if ( !sc.isSuccess() ) return sc;
      if ( m_defaultPartition.empty() ) m_defaultPartition = nam;
    }
    return STATUS::SUCCESS;
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( MultiStoreSvc )

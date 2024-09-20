/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
#include <GaudiKernel/AttribStringParser.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IAddressCreator.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IPartitionControl.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/TypeNameString.h>
#include <GaudiKernel/compose.h>
#include <map>
#include <variant>

// Forward declarations
// This class
class MultiStoreSvc;

typedef const std::string CSTR;
typedef IDataStoreAgent   AGENT;
typedef DataObject        OBJECT;
typedef IOpaqueAddress    ADDRESS;

namespace {
  struct Partition final {
    SmartIF<IDataProviderSvc> dataProvider;
    SmartIF<IDataManagerSvc>  dataManager;
    std::string               name;

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

  namespace detail {
    template <typename lambda>
    struct arg_helper : public arg_helper<decltype( &lambda::operator() )> {};
    template <typename T, typename Ret, typename Arg>
    struct arg_helper<Ret ( T::* )( Arg ) const> {
      using type = Arg;
    };

    // given a unary lambda whose argument is of type Arg_t,
    // argument_t<lambda> will be equal to Arg_t
    template <typename lambda>
    using argument_t = typename arg_helper<lambda>::type;
  } // namespace detail
  auto visit = []( auto&& variant, auto&&... lambdas ) -> decltype( auto ) {
    return std::visit( Gaudi::overload( std::forward<decltype( lambdas )>( lambdas )... ),
                       std::forward<decltype( variant )>( variant ) );
  };
} // namespace

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
class MultiStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc, IPartitionControl> {
protected:
  typedef std::vector<std::string>         PartitionDefs;
  typedef std::map<std::string, Partition> Partitions;

  Gaudi::Property<CLID>          m_rootCLID{ this, "RootCLID", 110, "CLID of root entry" };
  Gaudi::Property<std::string>   m_rootName{ this, "RootName", "/Event", "name of root entry" };
  Gaudi::Property<PartitionDefs> m_partitionDefs{ this, "Partitions", {}, "datastore partition definitions" };
  Gaudi::Property<std::string>   m_loader{ this, "DataLoader", "EventPersistencySvc", "data loader name" };
  Gaudi::Property<std::string>   m_defaultPartition{ this, "DefaultPartition", "Default", "default partition name" };

  /// Pointer to data loader service
  SmartIF<IConversionSvc> m_dataLoader;
  /// Reference to address creator
  SmartIF<IAddressCreator> m_addrCreator;
  /// Root type (address or object)
  struct tagROOT {
    std::string                                     path;
    std::variant<std::monostate, ADDRESS*, OBJECT*> root;
  } m_root;
  /// Current partition
  Partition m_current;
  /// Datastore partitions
  Partitions m_partitions;

  // member templates to help writing the function calls
  template <typename Fun>
  StatusCode fwd( Fun f ) {
    auto* svc = m_current.get<std::decay_t<detail::argument_t<Fun>>>();
    return svc ? f( *svc ) : IDataProviderSvc::Status::INVALID_ROOT;
  }

public:
  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override { return m_rootCLID; }
  /// Name for root Event
  const std::string& rootName() const override { return m_rootName; }

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( std::string_view path, ADDRESS* pAddr ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.registerAddress( path, pAddr ); } );
  }
  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parent, std::string_view path, ADDRESS* pAddr ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.registerAddress( parent, path, pAddr ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.unregisterAddress( path ); } );
  }
  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.unregisterAddress( pParent, path ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const OBJECT* pObject, std::vector<IRegistry*>& leaves ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.objectLeaves( pObject, leaves ); } );
  }
  /// Explore the object store: retrieve all leaves attached to the object
  StatusCode objectLeaves( const IRegistry* pObject, std::vector<IRegistry*>& leaves ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.objectLeaves( pObject, leaves ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const OBJECT* pObject, IRegistry*& refpParent ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.objectParent( pObject, refpParent ); } );
  }
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const IRegistry* pObject, IRegistry*& refpParent ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.objectParent( pObject, refpParent ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( std::string_view path ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.clearSubTree( path ); } );
  }
  /// Remove all data objects below the sub tree identified
  StatusCode clearSubTree( OBJECT* pObject ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.clearSubTree( pObject ); } );
  }
  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override {
    for ( auto& i : m_partitions ) { i.second.dataManager->clearStore().ignore(); }
    visit(
        m_root.root,
        []( auto* p ) {
          if ( p ) p->release();
        },
        []( std::monostate ) {} );
    m_root.root = {};
    m_root.path.clear();
    return StatusCode::SUCCESS;
  }
  /// Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( std::string_view path, AGENT* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.traverseSubTree( path, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( OBJECT* pObject, AGENT* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.traverseSubTree( pObject, pAgent ); } );
  }
  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  StatusCode traverseTree( AGENT* pAgent ) override {
    return fwd( [&]( IDataManagerSvc& svc ) { return svc.traverseTree( pAgent ); } );
  }
  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  StatusCode setRoot( std::string path, OBJECT* pObj ) override {
    visit(
        m_root.root,
        []( auto* p ) {
          if ( p ) p->release();
        },
        []( std::monostate ) {} );
    m_root.path = std::move( path );
    m_root.root = pObj;
    if ( auto sc = preparePartitions(); !sc ) return sc;
    return activate( m_defaultPartition );
  }

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  StatusCode setRoot( std::string path, ADDRESS* pAddr ) override {
    visit(
        m_root.root,
        []( auto* p ) {
          if ( p ) p->release();
        },
        []( std::monostate ) {} );
    m_root.path = std::move( path );
    m_root.root = pAddr;
    if ( !pAddr ) return StatusCode::FAILURE;
    pAddr->addRef();
    if ( auto sc = preparePartitions(); !sc ) return sc;
    return activate( m_defaultPartition );
  }
  /// IDataManagerSvc: Pass a default data loader to the service.
  StatusCode setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc = nullptr ) override {
    m_dataLoader = pDataLoader;
    if ( m_dataLoader )
      if ( auto sc = m_dataLoader->setDataProvider( dpsvc ? dpsvc : this ); !sc ) return sc;
    for ( auto& i : m_partitions ) {
      if ( auto sc = i.second.dataManager->setDataLoader( m_dataLoader.get() ); !sc ) return sc;
    }
    return StatusCode::SUCCESS;
  }
  /// Add an item to the preload list
  StatusCode addPreLoadItem( const DataStoreItem& item ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.addPreLoadItem( item ); } );
  }
  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.removePreLoadItem( item ); } );
  }
  /// Clear the preload list
  StatusCode resetPreLoad() override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.resetPreLoad(); } );
  }
  /// load all preload items of the list
  StatusCode preLoad() override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.preLoad(); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( std::string_view parent, std::string_view obj, OBJECT* pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.registerObject( parent, obj, pObj ); } );
  }
  /// Register object with the data store.
  StatusCode registerObject( OBJECT* parent, std::string_view obj, OBJECT* pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.registerObject( parent, obj, pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unregisterObject( path ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( OBJECT* pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unregisterObject( pObj ); } );
  }
  /// Unregister object from the data store.
  StatusCode unregisterObject( OBJECT* pObj, std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unregisterObject( pObj, path ); } );
  }
  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* parent, std::string_view path, OBJECT*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.retrieveObject( parent, path, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( std::string_view path, OBJECT*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.retrieveObject( path, pObj ); } );
  }
  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* parent, std::string_view path, OBJECT*& pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.findObject( parent, path, pObj ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, std::string_view objPath, OBJECT* to ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.linkObject( from, objPath, to ); } );
  }
  /// Add a link to another object.
  StatusCode linkObject( std::string_view fullPath, OBJECT* to ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.linkObject( fullPath, to ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, std::string_view objPath ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( OBJECT* from, std::string_view objPath ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unlinkObject( from, objPath ); } );
  }
  /// Remove a link to another object.
  StatusCode unlinkObject( std::string_view path ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.unlinkObject( path ); } );
  }
  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.updateObject( pDirectory ); } );
  }
  /// Update object.
  StatusCode updateObject( OBJECT* pObj ) override {
    return fwd( [&]( IDataProviderSvc& svc ) { return svc.updateObject( pObj ); } );
  }
  /// Create a partition object. The name identifies the partition uniquely
  StatusCode create( CSTR& nam, CSTR& typ ) override {
    IInterface* pPartition = nullptr;
    return create( nam, typ, pPartition );
  }
  /// Create a partition object. The name identifies the partition uniquely
  StatusCode create( CSTR& nam, CSTR& typ, IInterface*& pPartition ) override {
    if ( get( nam, pPartition ).isSuccess() ) return IPartitionControl::Status::PARTITION_EXISTS;
    /// @FIXME: In the old implementation the services were "unmanaged" (non-active)
    auto isvc = serviceLocator()->service<IService>( typ );
    if ( !isvc ) return IInterface::Status::NO_INTERFACE;
    auto dataMgr  = isvc.as<IDataManagerSvc>();
    auto dataProv = isvc.as<IDataProviderSvc>();
    if ( !dataMgr || !dataProv ) return IInterface::Status::NO_INTERFACE;
    m_partitions.emplace( nam, Partition{ dataProv, dataMgr, nam } );
    return StatusCode::SUCCESS;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  StatusCode drop( CSTR& nam ) override {
    auto i = m_partitions.find( nam );
    if ( i == m_partitions.end() ) return IPartitionControl::Status::PARTITION_NOT_PRESENT;
    if ( i->second.dataManager == m_current.dataManager ) { m_current = Partition(); }
    i->second.dataManager->clearStore().ignore();
    m_partitions.erase( i );
    return StatusCode::SUCCESS;
  }

  /// Drop a partition object. The name identifies the partition uniquely
  StatusCode drop( IInterface* pPartition ) override {
    auto provider = SmartIF<IDataProviderSvc>( pPartition );
    if ( !provider ) return IInterface::Status::NO_INTERFACE;
    auto i = std::find_if( std::begin( m_partitions ), std::end( m_partitions ),
                           [&]( Partitions::const_reference p ) { return p.second.dataProvider == provider; } );
    if ( i == std::end( m_partitions ) ) return IPartitionControl::Status::PARTITION_NOT_PRESENT;
    i->second.dataManager->clearStore().ignore();
    m_partitions.erase( i );
    return StatusCode::SUCCESS;
  }

  /// Activate a partition object. The name identifies the partition uniquely.
  StatusCode activate( CSTR& nam ) override {
    auto i = m_partitions.find( nam );
    if ( i != m_partitions.end() ) {
      m_current = i->second;
      return StatusCode::SUCCESS;
    }
    m_current = {};
    return IPartitionControl::Status::PARTITION_NOT_PRESENT;
  }

  /// Activate a partition object.
  StatusCode activate( IInterface* pPartition ) override {
    auto provider = SmartIF<IDataProviderSvc>( pPartition );
    m_current     = Partition();
    if ( !provider ) return IInterface::Status::NO_INTERFACE;
    auto i = std::find_if( std::begin( m_partitions ), std::end( m_partitions ),
                           [&]( Partitions::const_reference p ) { return p.second.dataProvider == provider; } );
    if ( i == std::end( m_partitions ) ) return IPartitionControl::Status::PARTITION_NOT_PRESENT;
    m_current = i->second;
    return StatusCode::SUCCESS;
  }

  /// Access a partition object. The name identifies the partition uniquely.
  StatusCode get( CSTR& nam, IInterface*& pPartition ) const override {
    auto i = m_partitions.find( nam );
    if ( i != m_partitions.end() ) {
      pPartition = i->second.dataProvider;
      return StatusCode::SUCCESS;
    }
    pPartition = nullptr;
    return IPartitionControl::Status::PARTITION_NOT_PRESENT;
  }

  /// Access the active partition object.
  StatusCode activePartition( std::string& nam, IInterface*& pPartition ) const override {
    if ( m_current.dataProvider ) {
      nam        = m_current.name;
      pPartition = m_current.dataProvider;
      return StatusCode::SUCCESS;
    }
    nam.clear();
    pPartition = nullptr;
    return IPartitionControl::Status::NO_ACTIVE_PARTITION;
  }

  StatusCode attachServices() {
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

  StatusCode detachServices() {
    m_addrCreator.reset();
    m_dataLoader.reset();
    return StatusCode::SUCCESS;
  }

  /// Service initialisation
  StatusCode initialize() override {
    // Nothing to do: just call base class initialisation
    StatusCode sc = Service::initialize();
    if ( !sc.isSuccess() ) return sc;
    sc = makePartitions();
    if ( !sc.isSuccess() ) {
      error() << "Failed to connect to all store partitions." << endmsg;
      return sc;
    }
    return attachServices();
  }

  /// Service initialisation
  StatusCode reinitialize() override {
    StatusCode sc = Service::reinitialize();
    if ( !sc.isSuccess() ) {
      error() << "Enable to reinitialize base class" << endmsg;
      return sc;
    }
    sc = detachServices();
    if ( !sc.isSuccess() ) {
      error() << "Failed to detach necessary services." << endmsg;
      return sc;
    }
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
    return StatusCode::SUCCESS;
  }

  /// Service initialisation
  StatusCode finalize() override {
    setDataLoader( nullptr ).ignore();
    clearStore().ignore();
    clearPartitions().ignore();
    m_current = Partition();
    detachServices().ignore();
    return Service::finalize();
  }

  // protected:

  /// Inherited constructor
  using extends::extends;

  /// Standard Destructor
  ~MultiStoreSvc() override {
    setDataLoader( nullptr ).ignore();
    resetPreLoad().ignore();
    clearStore().ignore();
    clearPartitions().ignore();
  }

  /// Prepare partition for usage
  StatusCode preparePartitions() {
    StatusCode iret = StatusCode::SUCCESS;
    for ( auto& i : m_partitions ) {
      StatusCode sc = visit(
          m_root.root,
          [&]( ADDRESS* address ) -> StatusCode {
            if ( !address ) return StatusCode::FAILURE;
            ADDRESS* pAdd = nullptr;
            ADDRESS* p    = address;
            auto     sc   = m_addrCreator->createAddress( p->svcType(), p->clID(), p->par(), p->ipar(), pAdd );
            return sc.isSuccess() ? i.second.dataManager->setRoot( m_root.path, pAdd ) : sc;
          },
          [&]( OBJECT* object ) -> StatusCode {
            if ( object && object->clID() == CLID_DataObject ) {
              return i.second.dataManager->setRoot( m_root.path, new DataObject() );
            }
            return StatusCode::FAILURE;
          },
          []( std::monostate ) -> StatusCode { return StatusCode::FAILURE; } );
      if ( !sc.isSuccess() ) iret = sc;
    }
    return iret;
  }

  /// Clear all partitions
  StatusCode clearPartitions() {
    for ( auto& i : m_partitions ) i.second.dataManager->clearStore().ignore();
    m_partitions.clear();
    return StatusCode::SUCCESS;
  }

  /// Create all partitions according to job options
  StatusCode makePartitions() {
    using Parser = Gaudi::Utils::AttribStringParser;
    std::string typ, nam;
    clearPartitions().ignore();
    for ( auto part : m_partitionDefs ) {
      for ( auto attrib : Parser( std::move( part ) ) ) {
        switch ( ::toupper( attrib.tag[0] ) ) {
        case 'N':
          nam = std::move( attrib.value );
          break;
        case 'T':
          typ = std::move( attrib.value );
          break;
        }
      }
      StatusCode sc = create( nam, typ );
      if ( !sc.isSuccess() ) return sc;
      if ( m_defaultPartition.empty() ) m_defaultPartition = nam;
    }
    return StatusCode::SUCCESS;
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( MultiStoreSvc )

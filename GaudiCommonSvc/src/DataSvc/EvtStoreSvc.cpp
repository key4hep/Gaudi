#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"

#include "boost/algorithm/string/predicate.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

  class Entry final : public IRegistry {
    std::unique_ptr<DataObject>     m_data;
    std::unique_ptr<IOpaqueAddress> m_addr;
    std::string                     m_identifier;
    static IDataProviderSvc*        s_svc;

  public:
    static void setDataProviderSvc( IDataProviderSvc* p ) { s_svc = p; }

    Entry( std::string id, std::unique_ptr<DataObject> data, std::unique_ptr<IOpaqueAddress> addr = {} ) noexcept
        : m_data{std::move( data )}, m_addr{std::move( addr )}, m_identifier{std::move( id )} {
      if ( m_data ) m_data->setRegistry( this );
      if ( m_addr ) m_addr->setRegistry( this );
    }
    Entry( const Entry& ) = delete;
    Entry& operator=( const Entry& rhs ) = delete;
    Entry( Entry&& rhs ) noexcept
        : m_data{std::move( rhs.m_data )}
        , m_addr{std::move( rhs.m_addr )}
        , m_identifier{std::move( rhs.m_identifier )} {
      if ( m_data ) m_data->setRegistry( this );
      if ( m_addr ) m_addr->setRegistry( this );
    }
    Entry& operator=( Entry&& rhs ) noexcept {
      m_data = std::move( rhs.m_data );
      if ( m_data ) m_data->setRegistry( this );
      m_identifier = std::move( rhs.m_identifier );
      m_addr       = std::move( rhs.m_addr );
      if ( m_addr ) m_addr->setRegistry( this );
      return *this;
    }
    friend void swap( Entry& lhs, Entry& rhs ) noexcept {
      using std::swap;
      swap( lhs.m_data, rhs.m_data );
      if ( lhs.m_data ) lhs.m_data->setRegistry( &lhs );
      if ( rhs.m_data ) rhs.m_data->setRegistry( &rhs );
      swap( lhs.m_identifier, rhs.m_identifier );
      swap( lhs.m_addr, rhs.m_addr );
      if ( lhs.m_addr ) lhs.m_addr->setRegistry( &lhs );
      if ( rhs.m_addr ) rhs.m_addr->setRegistry( &rhs );
    }

    // required by IRegistry...
    unsigned long     addRef() override { return -1; }
    unsigned long     release() override { return -1; }
    const name_type&  name() const override { return m_identifier; } // should really be from last '/' onward...
    const id_type&    identifier() const override { return m_identifier; }
    IDataProviderSvc* dataSvc() const override { return s_svc; }
    DataObject*       object() const override { return const_cast<DataObject*>( m_data.get() ); }
    IOpaqueAddress*   address() const override { return m_addr.get(); }
    void              setAddress( IOpaqueAddress* iAddr ) override { m_addr.reset( iAddr ); }
  };
  IDataProviderSvc* Entry::s_svc = nullptr;

  namespace details {
    template <typename Map>
    class MapStore {
      Map m_store;
      static_assert( std::is_same_v<typename Map::key_type, std::string_view> );

    public:
      const Entry* find( std::string_view k ) const noexcept {
        auto i = m_store.find( k );
        return i != m_store.end() ? &( i->second ) : nullptr;
      }
      const auto& emplace( std::string_view k, std::unique_ptr<DataObject> d ) {
        // tricky way to insert a string_view key which points to the
        // string contained in the mapped type...
        auto [i, b] = m_store.try_emplace( k, std::string{k}, std::move( d ) );
        if ( !b ) throw std::runtime_error( "failed to insert " + std::string{k} );
        auto nh  = m_store.extract( i );
        nh.key() = nh.mapped().identifier(); // "re-point" key to the string contained in the Entry
        auto r   = m_store.insert( std::move( nh ) );
        if ( !r.inserted ) throw std::runtime_error( "failed to insert " + std::string{k} );
        return r.position->second;
      }
      auto erase( std::string_view k ) { return m_store.erase( k ); }
      void clear() noexcept { m_store.clear(); }
      template <typename Predicate>
      void erase_if( Predicate p ) {
        auto i   = m_store.begin();
        auto end = m_store.end();
        while ( i != end ) {
          if ( std::invoke( p, std::as_const( *i ) ) )
            i = m_store.erase( i );
          else
            ++i;
        }
      }
      auto begin() const noexcept { return m_store.begin(); }
      auto end() const noexcept { return m_store.end(); }
    };
  } // namespace details
  using UnorderedStore = details::MapStore<std::unordered_map<std::string_view, Entry>>;
  using MapStore       = details::MapStore<std::map<std::string_view, Entry>>;

  template <typename StoreImpl = UnorderedStore>
  struct Store : private StoreImpl {

    template <typename K>
    const Entry* find( K&& k ) const noexcept {
      return static_cast<const StoreImpl&>( *this ).find( std::forward<K>( k ) );
    }

    template <typename K>
    const DataObject* put( K&& k, std::unique_ptr<DataObject> data ) {
      return this->emplace( std::forward<K>( k ), std::move( data ) ).object();
    }

    template <typename K>
    const DataObject* get( K&& k ) const noexcept {
      const Entry* d = find( std::forward<K>( k ) );
      return d ? d->object() : nullptr;
    }

    void clear() noexcept { static_cast<StoreImpl&>( *this ).clear(); }
    auto erase( std::string_view k ) { return static_cast<StoreImpl&>( *this ).erase( k ); }

    template <typename Predicate>
    void erase_if( Predicate&& p ) {
      static_cast<StoreImpl&>( *this ).erase_if( std::forward<Predicate>( p ) );
    }

    auto begin() const noexcept { return static_cast<const StoreImpl&>( *this ).begin(); }
    auto end() const noexcept { return static_cast<const StoreImpl&>( *this ).begin(); }
  };

  StatusCode dummy( std::string s ) {
    std::string trace;
    System::backTrace( trace, 6, 2 );
    throw std::logic_error{"Unsupported Function Called: " + s + "\n" + trace};
    return StatusCode::FAILURE;
  }

  std::string_view normalize_path( std::string_view path, std::string_view prefix ) {
    if ( path.size() >= prefix.size() && std::equal( prefix.begin(), prefix.end(), path.begin() ) )
      path.remove_prefix( prefix.size() );
    if ( path.size() > 0 && path[0] == '/' ) path.remove_prefix( 1 );
    return path;
  }

} // namespace

/**
 * @class EvtStoreSvc
 *
 * Use a minimal event store implementation, and adds
 * everything required to satisfy the IDataProviderSvc and IDataManagerSvc
 * interfaces by throwing exceptions except when the functionality is really needed...
 *
 * @author Gerhard Raven
 * @version 1.0
 */
class GAUDI_API EvtStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc> {
  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};

  SmartIF<IConversionSvc> m_dataLoader;

  /// Items to be pre-loaded
  std::vector<DataStoreItem> m_preLoads;

  /// The actual store
  /// (TODO: implement IHiveWhiteBoard, and keep a vector of stores, including
  ///        a thread-local index to the 'current' store
  Store<> m_store;

public:
  using extends::extends;

  CLID               rootCLID() const override;
  const std::string& rootName() const override;
  StatusCode         setDataLoader( IConversionSvc* svc, IDataProviderSvc* dpsvc ) override;

  StatusCode objectParent( const DataObject*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectParent( const IRegistry*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const DataObject*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const IRegistry*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }

  StatusCode clearSubTree( std::string_view ) override;
  StatusCode clearSubTree( DataObject* obj ) override {
    return obj && obj->registry() ? clearSubTree( obj->registry()->identifier() ) : StatusCode::FAILURE;
  }
  StatusCode clearStore() override;

  StatusCode traverseSubTree( std::string_view, IDataStoreAgent* ) override;
  StatusCode traverseSubTree( DataObject* obj, IDataStoreAgent* pAgent ) override {
    return ( obj && obj->registry() ) ? traverseSubTree( obj->registry()->identifier(), pAgent ) : StatusCode::FAILURE;
  }
  StatusCode traverseTree( IDataStoreAgent* pAgent ) override { return traverseSubTree( std::string_view{}, pAgent ); }

  StatusCode setRoot( std::string root_name, DataObject* pObject ) override;
  StatusCode setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) override;

  StatusCode unregisterAddress( std::string_view ) override { return dummy( __FUNCTION__ ); };
  StatusCode unregisterAddress( IRegistry*, std::string_view ) override { return dummy( __FUNCTION__ ); };

  StatusCode registerAddress( std::string_view fullPath, IOpaqueAddress* pAddress ) override;
  StatusCode registerAddress( IRegistry* parentObj, std::string_view objectPath, IOpaqueAddress* pAddress ) override;
  StatusCode registerObject( std::string_view parentPath, std::string_view objectPath, DataObject* pObject ) override;
  StatusCode registerObject( DataObject* parentObj, std::string_view objectPath, DataObject* pObject ) override;

  StatusCode unregisterObject( std::string_view ) override;
  StatusCode unregisterObject( DataObject* obj ) override {
    return ( obj && obj->registry() ) ? unregisterObject( obj->registry()->identifier() ) : StatusCode::FAILURE;
  }
  StatusCode unregisterObject( DataObject* obj, std::string_view sr ) override {
    return !obj ? unregisterObject( sr )
                : obj->registry() ? unregisterObject( ( obj->registry()->identifier() + '/' ).append( sr ) )
                                  : StatusCode::FAILURE;
  };

  StatusCode retrieveObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) override;

  StatusCode findObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) override;
  StatusCode findObject( std::string_view fullPath, DataObject*& pObject ) override;

  StatusCode updateObject( IRegistry* ) override { return dummy( __FUNCTION__ ); }
  StatusCode updateObject( DataObject* ) override { return dummy( __FUNCTION__ ); }

  StatusCode addPreLoadItem( const DataStoreItem& ) override;
  StatusCode removePreLoadItem( const DataStoreItem& ) override;
  StatusCode resetPreLoad() override {
    m_preLoads.clear();
    return StatusCode::SUCCESS;
  }
  StatusCode preLoad() override;

  StatusCode linkObject( IRegistry*, std::string_view, DataObject* ) override { return dummy( __FUNCTION__ ); }
  StatusCode linkObject( std::string_view, DataObject* ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( IRegistry*, std::string_view ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( DataObject*, std::string_view ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( std::string_view ) override;

  StatusCode initialize() override {
    Entry::setDataProviderSvc( this );
    extends::initialize().ignore();
    return setDataLoader( serviceLocator()->service( "EventPersistencySvc" ).as<IConversionSvc>().get(), nullptr );
  }
  StatusCode finalize() override {
    setDataLoader( nullptr, nullptr ).ignore(); // release
    return extends::finalize();
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( EvtStoreSvc )

CLID               EvtStoreSvc::rootCLID() const { return m_rootCLID; }
const std::string& EvtStoreSvc::rootName() const { return m_rootName; }
StatusCode         EvtStoreSvc::setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc ) {
  m_dataLoader = pDataLoader;
  if ( m_dataLoader ) m_dataLoader->setDataProvider( dpsvc ? dpsvc : this ).ignore();
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::clearSubTree( std::string_view sr ) {
  m_store.erase_if( [sr]( const auto& value ) { return boost::algorithm::starts_with( value.first, sr ); } );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::clearStore() {
  m_store.clear();
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::traverseSubTree( std::string_view top, IDataStoreAgent* pAgent ) {
  top      = normalize_path( top, rootName() );
  auto cmp = []( const Entry* lhs, const Entry* rhs ) { return lhs->identifier() < rhs->identifier(); };
  std::set<const Entry*, decltype( cmp )> keys{std::move( cmp )};
  for ( const auto& v : m_store ) {
    if ( boost::algorithm::starts_with( v.second.identifier(), top ) ) keys.insert( &v.second );
  }
  auto k = keys.begin();
  while ( k != keys.end() ) {
    const auto& id = ( *k )->identifier();
    always() << "analyzing " << id << endmsg;
    int  level  = std::count( id.begin(), id.end(), '/' );
    bool accept = pAgent->analyse( const_cast<Entry*>( *( k++ ) ), level );
    if ( !accept ) {
      while ( k != keys.end() && boost::algorithm::starts_with( ( *k )->identifier(), id ) ) {
        always() << "skipping " << ( *k )->identifier() << endmsg;
        ++k;
      }
    }
  }
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, DataObject* pObject ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "setRoot( " << root_path << ", (DataObject*)" << (void*)pObject << " )" << endmsg;
  }
  clearStore().ignore();
  return registerObject( nullptr, root_path, pObject );
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "setRoot( " << root_path << ", (IOpaqueAddress*)" << (void*)pRootAddr << " )" << endmsg;
  }
  clearStore().ignore();
  if ( !pRootAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  const std::string* par = pRootAddr->par();
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "par[0]=" << par[0] << endmsg;
    debug() << "par[1]=" << par[1] << endmsg;
  }
  DataObject* pObject = nullptr;
  auto        status  = m_dataLoader->createObj( pRootAddr, pObject ); // Call data loader
  if ( !status.isSuccess() ) return status;
  if ( msgLevel( MSG::DEBUG ) ) { debug() << "Root Object " << root_path << " created " << endmsg; }
  auto dummy = Entry{root_path, std::unique_ptr<DataObject>{}};
  pObject->setRegistry( &dummy );
  pRootAddr->setRegistry( &dummy );
  status  = m_dataLoader->fillObjRefs( pRootAddr, pObject );
  auto sc = registerObject( nullptr, root_path, pObject );
  if ( auto* reg = pObject->registry(); reg ) {
    reg->setAddress( pRootAddr );
  } else {
    delete pRootAddr;
  }
  return sc;
}
StatusCode EvtStoreSvc::registerAddress( std::string_view path, IOpaqueAddress* pAddr ) {
  return registerAddress( nullptr, path, pAddr );
}
StatusCode EvtStoreSvc::registerAddress( IRegistry* pReg, std::string_view path, IOpaqueAddress* pAddr ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerAddress( (IRegistry*)" << (void*)pReg << ", " << path << ", (IOpaqueAddress*)" << pAddr << ")"
            << endmsg;
  }
  if ( !pAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  if ( path.empty() || path[0] != '/' ) return StatusCode::FAILURE;
  DataObject* pObject = nullptr;
  auto        status  = m_dataLoader->createObj( pAddr, pObject );
  if ( !status.isSuccess() ) return status;
  auto fullpath = ( pReg ? pReg->identifier() : m_rootName.value() ) + std::string{path};
  // the data loader expects the path _including_ the root
  auto dummy = Entry{fullpath, std::unique_ptr<DataObject>{}, std::unique_ptr<IOpaqueAddress>( pAddr )};
  pObject->setRegistry( &dummy );
  status = m_dataLoader->fillObjRefs( pAddr, pObject );
  if ( !status.isSuccess() ) return status;
  // note: put will overwrite the registry in pObject to point at the
  //       one actually used -- so we do not dangle, pointing at dummy beyond its
  //       lifetime
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerAddress: " << std::quoted( std::string{normalize_path( fullpath, rootName() )} )
            << " (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
  }
  m_store.put( normalize_path( fullpath, rootName() ), std::unique_ptr<DataObject>( pObject ) );
  [[maybe_unused]] IRegistry* reg = pObject->registry();
  assert( reg != &dummy && reg != nullptr );
  // reg->setAddress( pAddr ); has been deleted! have to retrieve from dummy prior to put
  return status;
}
StatusCode EvtStoreSvc::registerObject( std::string_view parentPath, std::string_view objectPath,
                                        DataObject* pObject ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerObject( " << parentPath << ", " << objectPath << ", " << (void*)pObject << " )" << endmsg;
  }
  return parentPath.empty()
             ? registerObject( nullptr, objectPath, pObject )
             : registerObject( nullptr, std::string{parentPath}.append( "/" ).append( objectPath ), pObject );
}
StatusCode EvtStoreSvc::registerObject( DataObject* parentObj, std::string_view path, DataObject* pObject ) {
  if ( parentObj ) return StatusCode::FAILURE;
  auto path_v = normalize_path( path, rootName() );
  if ( m_forceLeaves ) {
    auto dir = path_v;
    for ( auto i = dir.rfind( '/' ); i != std::string_view::npos; i = dir.rfind( '/' ) ) {
      dir = dir.substr( 0, i );
      if ( !m_store.find( dir ) ) {
        if ( msgLevel( MSG::DEBUG ) ) {
          debug() << "registerObject: adding directory " << std::quoted( std::string{dir} ) << endmsg;
        }
        m_store.put( dir, std::make_unique<DataObject>() );
      }
    }
  }
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerObject: " << std::quoted( std::string{path_v} ) << " (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
  }
  m_store.put( path_v, std::unique_ptr<DataObject>( pObject ) );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::retrieveObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) {
  if ( pDirectory ) return StatusCode::FAILURE;
  auto path_v = normalize_path( path, rootName() );
  pObject     = const_cast<DataObject*>( m_store.get( path_v ) );
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "retrieveObject: " << std::quoted( std::string{path_v} ) << " (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
  }
  return pObject ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
StatusCode EvtStoreSvc::findObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) {
  return retrieveObject( pDirectory, path, pObject );
}
StatusCode EvtStoreSvc::findObject( std::string_view fullPath, DataObject*& pObject ) {
  return retrieveObject( nullptr, fullPath, pObject );
}
StatusCode EvtStoreSvc::unlinkObject( std::string_view sr ) { return unregisterObject( sr ); }
StatusCode EvtStoreSvc::unregisterObject( std::string_view sr ) {
  return m_store.erase( sr ) != 0 ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
StatusCode EvtStoreSvc::addPreLoadItem( const DataStoreItem& item ) {
  auto i = std::find( m_preLoads.begin(), m_preLoads.begin(), item );
  if ( i == m_preLoads.end() ) m_preLoads.push_back( item );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::removePreLoadItem( const DataStoreItem& item ) {
  auto i = std::remove( m_preLoads.begin(), m_preLoads.begin(), item );
  m_preLoads.erase( i, m_preLoads.end() );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::preLoad() {
  for ( const auto& i : m_preLoads ) {
    DataObject* pObj;
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Preloading " << i.path() << endmsg;
    retrieveObject( nullptr, i.path(), pObj ).ignore();
  }
  return StatusCode::SUCCESS;
}

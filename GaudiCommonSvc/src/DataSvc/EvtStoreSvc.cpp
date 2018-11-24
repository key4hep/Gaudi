#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
  static std::string const no_entry_at = "no entry at";

  // void* magic_cast( std::type_index /*targetType*/, std::type_index /*sourceType*/, void* ) { return nullptr; }

  struct VTable {
    void ( *delete_ )( void* );
    void* ( *cast_ )( std::type_index, void* );
    std::optional<size_t> ( *size_ )( const void* );
    std::type_index ( *type_ )();
  };

  template <typename T>
  void delete_( void* self )
  {
    delete static_cast<T*>( self );
  }

  template <>
  void delete_<void>( void* )
  {
  }

  template <typename T>
  std::type_index type_()
  {
    return std::type_index( typeid( T ) );
  }

  template <typename T>
  void* cast_( std::type_index type, void* self )
  {
    return type == std::type_index( typeid( T ) )
               ? static_cast<T*>( self ) // : magic_cast( type, std::type_index( typeid( T ) ), self );
               : nullptr;
  }

  template <>
  void* cast_<void>( std::type_index, void* )
  {
    return nullptr;
  }

  namespace details
  {
    using std::size;

    template <typename T, typename... Args>
    constexpr auto size( const T&, Args&&... ) noexcept
    {
      static_assert( sizeof...( Args ) == 0, "No extra args please" );
      return std::nullopt;
    }
  }

  template <typename T>
  std::optional<size_t> size_( const void* self )
  {
    return details::size( *static_cast<const T*>( self ) );
  }

  template <>
  std::optional<size_t> size_<void>( const void* )
  {
    return std::nullopt;
  }

  template <typename T>
  inline constexpr VTable const vtable_for = {&delete_<T>, &cast_<T>, &size_<T>, &type_<T>};

  class Entry final : public IRegistry
  {
    // for now, optimize for size. Could put selected functions also 'in-situ' to
    // improve optimization (eg. inlining)
    const VTable* m_vtable = &vtable_for<void>;
    void*         m_ptr    = nullptr;
    // TODO: add SBO here, so that eg. int, double, vector<T> payloads fit in-situ
    // WARNING: SBO will invalidate pointers to payload when resizing the store
    //          so _only_ do this _after_ we have a store which can be completely
    //          reserved during initialize, _prior_ to it being populated with content!
    // (note: the above actually depends on the container implementation which is
    //        used to contain 'Entry'...  if the Entry instances inside the container are
    //        stable then obviously SBO doesn't affect the payload stability ;-)
    static IDataProviderSvc* s_svc;
    std::string              m_identifier;
    IOpaqueAddress*          m_addr = nullptr;

  public:
    static void setDataProviderSvc( IDataProviderSvc* p ) { s_svc = p; }

    Entry() = default;

    template <typename T, // TODO: require T to be move constructible
              typename = std::enable_if_t<!std::is_same_v<Entry, std::decay_t<T>>>>
    Entry( std::string identifier, T&& t )
        : m_vtable{&vtable_for<T>}, m_ptr{new T{std::move( t )}}, m_identifier{std::move( identifier )}
    {
    }

    // 'adopt' a unique pointer... (provided it has a default deleter!)
    template <typename T>
    Entry( std::string identifier, std::unique_ptr<T, std::default_delete<T>> t ) noexcept
        : m_vtable{&vtable_for<T>}, m_ptr{t.release()}, m_identifier{std::move( identifier )}
    {
    }

    Entry( std::string identifier, std::unique_ptr<DataObject> d, IOpaqueAddress* addr = nullptr ) noexcept
        : m_vtable{&vtable_for<DataObject>}, m_ptr{d.release()}, m_identifier{std::move( identifier )}, m_addr{addr}
    {
      if ( auto* pd = static_cast<DataObject*>( m_ptr ); pd ) pd->setRegistry( this );
      if ( m_addr ) m_addr->setRegistry( this );
    }

    ~Entry() noexcept { m_vtable->delete_( m_ptr ); }

    Entry( const Entry& ) = delete;
    Entry& operator=( const Entry& rhs ) = delete;

    Entry( Entry&& rhs ) noexcept
        : m_vtable{std::exchange( rhs.m_vtable, &vtable_for<void> )}
        , m_ptr{std::exchange( rhs.m_ptr, nullptr )}
        , m_identifier{std::move( rhs.m_identifier )}
        , m_addr{std::exchange( rhs.m_addr, nullptr )}
    {
      if ( auto* d = object(); d ) d->setRegistry( this );
      if ( m_addr ) m_addr->setRegistry( this );
    }
    Entry& operator=( Entry&& rhs ) noexcept
    {
      m_vtable->delete_( m_ptr );
      m_vtable = std::exchange( rhs.m_vtable, &vtable_for<void> );
      m_ptr    = std::exchange( rhs.m_ptr, nullptr );
      if ( auto* d = object(); d ) d->setRegistry( this );
      m_identifier = std::move( rhs.m_identifier );
      m_addr       = std::exchange( rhs.m_addr, nullptr );
      if ( m_addr ) m_addr->setRegistry( this );
      return *this;
    }

    friend void swap( Entry& lhs, Entry& rhs ) noexcept
    {
      using std::swap;
      swap( lhs.m_vtable, rhs.m_vtable );
      swap( lhs.m_ptr, rhs.m_ptr );
      if ( auto* d = lhs.object(); d ) d->setRegistry( &lhs );
      if ( auto* d = rhs.object(); d ) d->setRegistry( &rhs );
      swap( lhs.m_identifier, rhs.m_identifier );
      swap( lhs.m_addr, rhs.m_addr );
      if ( lhs.m_addr ) lhs.m_addr->setRegistry( &lhs );
      if ( rhs.m_addr ) rhs.m_addr->setRegistry( &rhs );
    }

    template <typename T>
    const T* get_ptr() const
    {
      return static_cast<T*>( m_vtable->cast_( std::type_index( typeid( T ) ), m_ptr ) );
    }

    std::optional<size_t> size() const { return m_vtable->size_( m_ptr ); }

    std::type_index type() const { return m_vtable->type_(); }

    // required by IRegistry...
    unsigned long     addRef() override { return -1; }
    unsigned long     release() override { return -1; }
    const name_type&  name() const override { return m_identifier; } // should really be from last '/' onward...
    const id_type&    identifier() const override { return m_identifier; }
    IDataProviderSvc* dataSvc() const override { return s_svc; }
    DataObject*       object() const override { return const_cast<DataObject*>( get_ptr<DataObject>() ); }
    IOpaqueAddress*   address() const override { return m_addr; }
    void setAddress( IOpaqueAddress* iAddr ) override { m_addr = iAddr; }
  };
  IDataProviderSvc* Entry::s_svc = nullptr;

  class VectorStore
  {
    std::vector<Entry> m_store;

  public:
    VectorStore() = default;

    template <typename K>
    const Entry* find( K&& k ) const noexcept
    {
      auto i = std::find_if( m_store.begin(), m_store.end(),
                             [key = std::forward<K>( k )]( const auto& d ) { return d.identifier() == key; } );
      return i != m_store.end() ? &( *i ) : nullptr;
    }

    template <typename K, typename T>
    const auto& emplace( K&& k, T&& t )
    {
      if ( find( k ) != nullptr ) throw std::runtime_error( "entry " + k + " already exists" );
      return m_store.emplace_back( std::forward<K>( k ), std::forward<T>( t ) );
    }

    void clear() noexcept { m_store.clear(); }
  };

  namespace details
  {
    template <typename Map>
    class MapStore
    {
      Map m_store;
      // static_assert(Map::key_type == std::String_view )

    public:
      const Entry* find( std::string_view k ) const noexcept
      {
        auto i = m_store.find( k );
        return i != m_store.end() ? &( i->second ) : nullptr;
      }
      template <typename T>
      const auto& emplace( std::string_view k, T&& t )
      {
        // tricky way to insert a string_view key which points to the
        // string contained in the mapped type...
        Map dummy;
        auto[i, b] = dummy.try_emplace( "", std::string{k}, std::forward<T>( t ) );
        if ( !b ) throw std::runtime_error( "failed to insert " + std::string{k} );
        auto nh  = dummy.extract( i );
        nh.key() = nh.mapped().identifier();
        auto r   = m_store.insert( std::move( nh ) );
        if ( !r.inserted ) throw std::runtime_error( "failed to insert " + std::string{k} );
        return r.position->second;
      }
      void clear() noexcept { m_store.clear(); }
    };
  }
  using UnorderedStore = details::MapStore<std::unordered_map<std::string_view, Entry>>;
  using MapStore       = details::MapStore<std::map<std::string_view, Entry>>;

  template <typename StoreImpl = UnorderedStore>
  struct Store : private StoreImpl {

    template <typename T, typename K>
    const std::decay_t<T>& put( K&& k, T&& t )
    {
      auto& d = this->emplace( std::forward<K>( k ), std::forward<T>( t ) );
      auto* p = d.template get_ptr<std::decay_t<T>>();
      return *p;
    }

    template <typename T, typename K>
    const std::decay_t<T>& put( K&& k, std::unique_ptr<T> t ) noexcept
    {
      auto& d = this->emplace( std::forward<K>( k ), std::move( t ) );
      auto* p = d.template get_ptr<std::decay_t<T>>();
      return *p;
    }

    template <typename T, typename K>
    const T& get( K&& k ) const
    {
      const Entry* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{no_entry_at + k};
      const auto* p = d->get_ptr<T>();
      if ( !p ) throw std::bad_cast{};
      return *p;
    }

    template <typename T, typename K>
    const T* get_ptr( K&& k ) const
    {
      const Entry* d = this->find( std::forward<K>( k ) );
      if ( !d ) return nullptr;
      const auto* p = d->get_ptr<T>();
      if ( !p ) throw std::bad_cast{};
      return p;
    }

    template <typename K>
    std::optional<size_t> size( K&& k ) const
    {
      const Entry* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{no_entry_at + k};
      return d->size();
    }

    template <typename K>
    const char* type_name( K&& k ) const
    {
      const Entry* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{no_entry_at + k};
      return d->type().name();
    }

    void clear() noexcept { static_cast<StoreImpl*>( this )->clear(); }
  };

  StatusCode dummy( std::string s )
  {
    std::string trace;
    System::backTrace( trace, 6, 2 );
    throw std::logic_error{"Unsupported Function Called: " + s + "\n" + trace};
    return StatusCode::FAILURE;
  }
}

/**
 * @class EvtStoreSvc
 *
 * Use a minimal, non-intrusive event store implementation, and adds
 * everything required to satisfy the IDataProviderSvc and IDataManagerSvc
 * interfaces by throwing exceptions except when the functionality is really needed...
 *
 * @author Gerhard Raven
 * @version 1.0
*/
class GAUDI_API EvtStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc>, Store<>
{
  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};

  SmartIF<IConversionSvc> m_dataLoader;

public:
  using extends::extends;

  CLID               rootCLID() const override;
  const std::string& rootName() const override;
  StatusCode setDataLoader( IConversionSvc* svc, IDataProviderSvc* dpsvc ) override;

  StatusCode objectParent( const DataObject*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectParent( const IRegistry*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const DataObject*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const IRegistry*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }

  StatusCode clearSubTree( boost::string_ref ) override { return dummy( __FUNCTION__ ); }
  StatusCode clearSubTree( DataObject* ) override { return dummy( __FUNCTION__ ); }
  StatusCode clearStore() override;

  StatusCode traverseSubTree( boost::string_ref, IDataStoreAgent* ) override { return dummy( __FUNCTION__ ); }
  StatusCode traverseSubTree( DataObject*, IDataStoreAgent* ) override;
  StatusCode traverseTree( IDataStoreAgent* ) override { return dummy( __FUNCTION__ ); }

  StatusCode setRoot( std::string root_name, DataObject* pObject ) override;
  StatusCode setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) override;

  StatusCode unregisterAddress( boost::string_ref ) override { return dummy( __FUNCTION__ ); };
  StatusCode unregisterAddress( IRegistry*, boost::string_ref ) override { return dummy( __FUNCTION__ ); };

  StatusCode registerAddress( boost::string_ref fullPath, IOpaqueAddress* pAddress ) override;
  StatusCode registerAddress( IRegistry* parentObj, boost::string_ref objectPath, IOpaqueAddress* pAddress ) override;
  StatusCode registerObject( boost::string_ref parentPath, boost::string_ref objectPath, DataObject* pObject ) override;
  StatusCode registerObject( DataObject* parentObj, boost::string_ref objectPath, DataObject* pObject ) override;

  StatusCode unregisterObject( boost::string_ref ) override;
  StatusCode unregisterObject( DataObject* ) override { return dummy( __FUNCTION__ ); };
  StatusCode unregisterObject( DataObject*, boost::string_ref ) override { return dummy( __FUNCTION__ ); };

  StatusCode retrieveObject( IRegistry* pDirectory, boost::string_ref path, DataObject*& pObject ) override;

  StatusCode findObject( IRegistry* pDirectory, boost::string_ref path, DataObject*& pObject ) override;
  StatusCode findObject( boost::string_ref fullPath, DataObject*& pObject ) override;

  StatusCode updateObject( IRegistry* ) override { return dummy( __FUNCTION__ ); }
  StatusCode updateObject( DataObject* ) override { return dummy( __FUNCTION__ ); }

  StatusCode addPreLoadItem( const DataStoreItem& ) override { return dummy( __FUNCTION__ ); }
  StatusCode removePreLoadItem( const DataStoreItem& ) override { return dummy( __FUNCTION__ ); }
  StatusCode resetPreLoad() override { return dummy( __FUNCTION__ ); }
  StatusCode preLoad() override { return dummy( __FUNCTION__ ); }

  StatusCode linkObject( IRegistry*, boost::string_ref, DataObject* ) override { return dummy( __FUNCTION__ ); }
  StatusCode linkObject( boost::string_ref, DataObject* ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( IRegistry*, boost::string_ref ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( DataObject*, boost::string_ref ) override { return dummy( __FUNCTION__ ); }
  StatusCode unlinkObject( boost::string_ref ) override;

  StatusCode initialize() override
  {
    Entry::setDataProviderSvc( this );
    extends::initialize().ignore();
    return setDataLoader( serviceLocator()->service( "EventPersistencySvc" ).as<IConversionSvc>().get(), nullptr );
  }
  StatusCode finalize() override
  {
    setDataLoader( nullptr, nullptr ).ignore(); // release
    return extends::finalize();
  }
};

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( EvtStoreSvc )

CLID               EvtStoreSvc::rootCLID() const { return m_rootCLID; }
const std::string& EvtStoreSvc::rootName() const { return m_rootName; }
StatusCode EvtStoreSvc::setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc )
{
  m_dataLoader = pDataLoader;
  if ( m_dataLoader ) m_dataLoader->setDataProvider( dpsvc ? dpsvc : this ).ignore();
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::clearStore()
{
  this->clear();
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, DataObject* pObject )
{
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "setRoot( " << root_path << ", (DataObject*)" << (void*)pObject << " )" << endmsg;
  }
  clearStore().ignore();
  return registerObject( nullptr, root_path, pObject );
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr )
{
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
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Root Object " << root_path << " created " << endmsg;
  }
  auto dummy = Entry{root_path, std::unique_ptr<DataObject>{}};
  pObject->setRegistry( &dummy );
  pRootAddr->setRegistry( &dummy );
  status = m_dataLoader->fillObjRefs( pRootAddr, pObject );
  return registerObject( nullptr, root_path, pObject );
}
StatusCode EvtStoreSvc::registerAddress( boost::string_ref path, IOpaqueAddress* pAddr )
{
  return registerAddress( nullptr, path, pAddr );
}
StatusCode EvtStoreSvc::registerAddress( IRegistry* pReg, boost::string_ref path, IOpaqueAddress* pAddr )
{
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerAddress( (IRegistry*)" << (void*)pReg << ", " << path << ", (IOpaqueAddress*)" << pAddr << ")"
            << endmsg;
  }
  if ( !pAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  DataObject* pObject = nullptr;
  auto        status  = m_dataLoader->createObj( pAddr, pObject );
  if ( !status.isSuccess() ) return status;
  auto fullpath = ( pReg ? pReg->identifier() : m_rootName.value() ) + path.to_string();
  // the data loader expects the path _including_ the root
  auto dummy    = Entry{fullpath, std::unique_ptr<DataObject>{}, pAddr};
  pObject->setRegistry( &dummy );
  status = m_dataLoader->fillObjRefs( pAddr, pObject );
  if ( !status.isSuccess() ) return status;
  // note: registerObject will overwrite the registry in pObject to point at the
  //       one actually used -- so we do not dangle, pointing at dummy beyond its
  //       lifetime
  status = registerObject( nullptr, fullpath, pObject );
  IRegistry* reg = pObject->registry();
  assert(reg != &dummy && reg != nullptr);
  reg->setAddress(pAddr);
  return status;
}
StatusCode EvtStoreSvc::registerObject( boost::string_ref parentPath, boost::string_ref objectPath,
                                        DataObject* pObject )
{
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerObject( " << parentPath << ", " << objectPath << ", " << (void*)pObject << " )" << endmsg;
  }
  return parentPath.empty()
             ? registerObject( nullptr, objectPath, pObject )
             : registerObject( nullptr, parentPath.to_string().append( "/" ).append( objectPath.to_string() ),
                               pObject );
}
StatusCode EvtStoreSvc::registerObject( DataObject* parentObj, boost::string_ref path, DataObject* pObject )
{
  if ( parentObj ) return StatusCode::FAILURE;
  if ( path.starts_with( m_rootName.value() ) ) path.remove_prefix( m_rootName.size() );
  if ( path.starts_with( "/" ) ) path.remove_prefix( 1 );
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerObject: " << std::quoted( path.to_string() ) << " (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
  }
  this->put( path.to_string(), std::unique_ptr<DataObject>( pObject ) );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::retrieveObject( IRegistry* pDirectory, boost::string_ref path, DataObject*& pObject )
{
  if ( pDirectory ) return StatusCode::FAILURE;
  if ( path.starts_with( m_rootName.value() ) ) path.remove_prefix( m_rootName.size() );
  if ( path.starts_with( "/" ) ) path.remove_prefix( 1 );
  pObject = const_cast<DataObject*>( this->get_ptr<DataObject>( std::string_view{ path.data(), path.size() } ) );
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "retrieveObject: " << std::quoted( path.to_string() ) << " (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
  }
  return pObject ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
StatusCode EvtStoreSvc::findObject( IRegistry* pDirectory, boost::string_ref path, DataObject*& pObject )
{
  return retrieveObject( pDirectory, path, pObject );
}
StatusCode EvtStoreSvc::findObject( boost::string_ref fullPath, DataObject*& pObject )
{
  return retrieveObject( nullptr, fullPath, pObject );
}
StatusCode EvtStoreSvc::unlinkObject( boost::string_ref sr )
{
  warning() << "EvtStoreSvc::unlinkObject(" << sr << "): not doing anything..." << endmsg;
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::unregisterObject( boost::string_ref sr )
{
  warning() << "EvtStoreSvc::unregisterObjecct(" << sr << "): not doing anything..." << endmsg;
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::traverseSubTree( DataObject*, IDataStoreAgent* )
{
  warning() << "EvtStoreSvc::traverseSubTree: not doing anything..." << endmsg;
  return StatusCode::SUCCESS;
}

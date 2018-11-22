#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace
{

  void* magic_cast( std::type_index /*targetType*/, std::type_index /*sourceType*/, void* ) { return nullptr; }

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
    return type == std::type_index( typeid( T ) ) ? static_cast<T*>( self ) // : magic_cast( type, std::type_index( typeid( T ) ), self );
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

  class Data
  {
    // for now, optimize for size. Could put selected functions also 'in-situ' to
    // improve optimization (eg. inlining) as
    const VTable* m_vtable = &vtable_for<void>;
    void*         m_ptr    = nullptr;
    // TODO: add SBO here, so that eg. int, double, vector<T> payloads fit in-situ
    // WARNING: SBO will invalidate pointers to payload when resizing the store
    //          so _only_ do this _after_ we have a store which can be completely
    //          reserved during initialize, _prior_ to it being populated with content!
    // (note: the above actually depends on the container implementation which is
    //        used to contain 'Data'...  if the Data instances inside the container are
    //        stable then obviously SBO doesn't affect the payload stability ;-)
  public:
    Data() = default;

    template <typename T, // TODO: require T to be move constructible
              typename = std::enable_if_t<!std::is_same_v<Data, std::decay_t<T>>>>
    Data( T&& t ) : m_vtable{&vtable_for<T>}, m_ptr{new T{std::move( t )}}
    {
    }

    // 'adopt' a unique pointer...
    template <typename T>
    Data( std::unique_ptr<T, std::default_delete<T>> t ) noexcept : m_vtable{&vtable_for<T>}, m_ptr{t.release()}
    {
    }

    ~Data() noexcept { m_vtable->delete_( m_ptr ); }

    Data( const Data& ) = delete;
    Data& operator=( const Data& rhs ) = delete;

    Data( Data&& rhs ) noexcept
        : m_vtable{std::exchange( rhs.m_vtable, &vtable_for<void> )}, m_ptr{std::exchange( rhs.m_ptr, nullptr )}
    {
    }
    Data& operator=( Data&& rhs ) noexcept
    {
      m_vtable->delete_( m_ptr );
      m_vtable = std::exchange( rhs.m_vtable, &vtable_for<void> );
      m_ptr    = std::exchange( rhs.m_ptr, nullptr );
      return *this;
    }

    friend void swap( Data& lhs, Data& rhs ) noexcept
    {
      using std::swap;
      swap( lhs.m_vtable, rhs.m_vtable );
      swap( lhs.m_ptr, rhs.m_ptr );
    }

    template <typename T>
    const T* get_ptr() const
    {
      return static_cast<T*>( m_vtable->cast_( std::type_index( typeid( T ) ), m_ptr ) );
    }

    std::optional<size_t> size() const { return m_vtable->size_( m_ptr ); }

    std::type_index type() const { return m_vtable->type_(); }
  };

#include <vector>
  template <typename Key>
  class VectorStore
  {
    std::vector<std::pair<Key, Data>> m_store;

  public:
    VectorStore() = default;

    template <typename K>
    const Data* find( K&& k ) const
    {
      auto i = std::find_if( m_store.begin(), m_store.end(),
                             [key = std::forward<K>( k )]( const auto& d ) { return d.first == key; } );
      return i != m_store.end() ? &i->second : nullptr;
    }

    template <typename T, typename K>
    const auto& emplace( K&& k, T&& t )
    {
      if ( find( k ) != nullptr ) throw std::runtime_error( "entry already exists" );
      return m_store.emplace_back( std::forward<K>( k ), std::forward<T>( t ) );
    }

    void clear() { m_store.clear(); }
  };

  namespace details {
      template <typename Map> class MapStore
      {
        Map m_store;

      public:
        MapStore() = default;

        template <typename K>
        const Data* find( K&& k ) const
        {
          auto i = m_store.find( std::forward<K>( k ) );
          return i != m_store.end() ? &i->second : nullptr;
        }

        template <typename T, typename K>
        const auto& emplace( K&& k, T&& t )
        {
          if ( const auto & [ ret, ok ] = m_store.try_emplace( std::forward<K>( k ), std::forward<T>( t ) ); ok ) {
            return *ret;
          }
          throw std::runtime_error( "entry already exists" );
        }
        void clear() { m_store.clear(); }
      };
  }
#include <unordered_map>
  template <typename Key>
  using UnorderedStore = details::MapStore< std::unordered_map<Key,Data> >;

#include <map>
  template <typename Key>
  using MapStore = details::MapStore< std::map<Key,Data,std::less<>> >;


  template <typename Key, template <typename> typename StoreImpl = UnorderedStore>
  struct Store : private StoreImpl<Key> {

    template <typename T, typename K>
    const std::decay_t<T>& put( K&& k, T&& t )
    {
      auto& d = this->emplace( std::forward<K>( k ), std::forward<T>( t ) );
      auto* p = d.second.template get_ptr<std::decay_t<T>>();
      return *p;
    }

    template <typename T, typename K>
    const std::decay_t<T>& put( K&& k, std::unique_ptr<T> t )
    {
      auto& d = this->emplace( std::forward<K>( k ), std::move( t ) );
      auto* p = d.second.template get_ptr<std::decay_t<T>>();
      return *p;
    }

    template <typename T, typename K>
    const T& get( K&& k ) const
    {
      const Data* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{"oops"};
      const auto* p = d->get_ptr<T>();
      if ( !p ) throw std::bad_cast{};
      return *p;
    }

    template <typename T, typename K>
    const T* get_ptr( K&& k ) const
    {
      const Data* d = this->find( std::forward<K>( k ) );
      if ( !d ) return nullptr;
      const auto* p = d->get_ptr<T>();
      if ( !p ) throw std::bad_cast{};
      return p;
    }

    template <typename K>
    std::optional<size_t> size( K&& k ) const
    {
      const Data* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{"oops"};
      return d->size();
    }

    template <typename K>
    const char* type_name( K&& k ) const
    {
      const Data* d = this->find( std::forward<K>( k ) );
      if ( !d ) throw std::out_of_range{"oops"};
      return d->type().name();
    }

    void clear() { static_cast<StoreImpl<Key>*>( this )->clear(); }
  };

  StatusCode dummy( std::string s )
  {
    std::string trace;
    System::backTrace( trace, 6, 2 );
    throw std::logic_error{"Unsupported Function Called: " + s + "\n" + trace};
    return StatusCode::FAILURE;
  }
  class DummyRegEntry final : public IRegistry
  {
    std::string       m_name;
    std::string       m_identifier;
    IDataProviderSvc* m_svc  = nullptr;
    DataObject*       m_data = nullptr;
    IOpaqueAddress*   m_addr = nullptr;

  public:
    DummyRegEntry( std::string name, std::string identifier, IDataProviderSvc* svc, DataObject* obj,
                   IOpaqueAddress* addr = nullptr )
        : m_name{std::move( name )}, m_identifier{std::move( identifier )}, m_svc{svc}, m_data{obj}, m_addr{addr}
    {
    }
    unsigned long addRef() override
    {
      dummy( __FUNCTION__ ).ignore();
      return -1;
    }
    unsigned long release() override
    {
      dummy( __FUNCTION__ ).ignore();
      return -1;
    }
    const name_type&  name() const override { return m_name; }
    const id_type&    identifier() const override { return m_identifier; }
    IDataProviderSvc* dataSvc() const override { return m_svc; }
    DataObject*       object() const override { return m_data; }
    IOpaqueAddress*   address() const override { return m_addr; }
    void              setAddress( IOpaqueAddress* ) override { dummy( __FUNCTION__ ).ignore(); }

    friend std::ostream& operator<<( std::ostream& os, const DummyRegEntry& re )
    {
      return os << "DummyRegEntry{ name = " << re.m_name << ", identifier = " << re.m_identifier << " }\n";
    }
  };
}

/**
 * @class DataSvc DataSvc.h GaudiKernel/DataSvc.h
 *
 * Use a minimal, non-intrusive event store implementation, and adds
 * everything required to satisfy the IDataProviderSvc and IDataManagerSvc
 * interfaces by throwing exceptions except when the functionality is really needed...
 *
 * @author Gerhard Raven
 * @version 1.0
*/
class GAUDI_API EvtStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc>, Store<std::string>
{
  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};

  SmartIF<IConversionSvc> m_dataLoader;

  std::deque<DummyRegEntry> m_regs;

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

  // the *PreLoad* functions could be done at the level of the interface -- except for the fact that
  // they imply a data member which is a vector of DataStoreItems, so it wouldn't be a "pure" interface
  // anymore...
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
    extends::initialize().ignore();
    return setDataLoader( serviceLocator()->service( "EventPersistencySvc" ).as<IConversionSvc>().get(), nullptr );
  }
  StatusCode finalize() override
  {
    setDataLoader( nullptr, nullptr ); // release
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
  m_regs.clear();
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, DataObject* pObject )
{
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "setRoot( " << root_path << ", (DataObject*)" << (void*)pObject << " )" << endmsg;
  clearStore();
  if ( pObject ) this->put( std::move( root_path ), std::unique_ptr<DataObject>( pObject ) );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr )
{
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "setRoot( " << root_path << ", (IOpaqueAddress*)" << (void*)pRootAddr << " )" << endmsg;
  clearStore();
  if ( !pRootAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  const std::string* par = pRootAddr->par();
  if ( msgLevel( MSG::DEBUG ) ) debug() << "par[0]=" << par[0] << endmsg;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "par[1]=" << par[1] << endmsg;
  DataObject* pObject = nullptr;
  auto        status  = m_dataLoader->createObj( pRootAddr, pObject ); // Call data loader
  if ( !status.isSuccess() ) return status;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Object " << root_path << " created" << endmsg;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Filling object refs " << root_path << endmsg;
  auto* dummy = &m_regs.emplace_back( root_path, root_path, this, pObject );
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Providing RegEntry for (DataObject*)" << (void*)pObject << ": " << *dummy << endmsg;
  pObject->setRegistry( dummy );
  pRootAddr->setRegistry( dummy );
  status = m_dataLoader->fillObjRefs( pRootAddr, pObject );
  this->put( "", std::unique_ptr<DataObject>( pObject ) );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::registerAddress( boost::string_ref path, IOpaqueAddress* pAddr )
{
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "registerAddress( " << path << ", (IOpaqueAddress*)" << pAddr << ")" << endmsg;
  return dummy( __FUNCTION__ );
}
StatusCode EvtStoreSvc::registerAddress( IRegistry* pReg, boost::string_ref path, IOpaqueAddress* pAddr )
{
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "registerAddress( (IRegistry*)" << (void*)pReg << ", " << path << ", (IOpaqueAddress*)" << pAddr << ")"
            << endmsg;
  if ( !pAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  DataObject* pObject = nullptr;
  auto        status  = m_dataLoader->createObj( pAddr, pObject );
  if ( !status.isSuccess() ) return status;
  auto  fullpath = ( pReg ? pReg->identifier() : m_rootName.value() ) + path.to_string();
  auto* dummy    = &m_regs.emplace_back( path.to_string(), fullpath, this, pObject, pAddr );
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "providing RegEntry for (DataObject*)" << (void*)pObject << ": " << *dummy << endmsg;
  pObject->setRegistry( dummy );
  pAddr->setRegistry( dummy );
  status = m_dataLoader->fillObjRefs( pAddr, pObject );
  return registerObject( nullptr, fullpath, pObject );
}
StatusCode EvtStoreSvc::registerObject( boost::string_ref parentPath, boost::string_ref objectPath,
                                        DataObject* pObject )
{
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "registerObject( " << parentPath << ", " << objectPath << ", " << (void*)pObject << " )" << endmsg;
  return parentPath.empty() ? registerObject( nullptr, objectPath, pObject ) : dummy( __FUNCTION__ );
}
StatusCode EvtStoreSvc::registerObject( DataObject* parentObj, boost::string_ref path, DataObject* pObject )
{
  if ( parentObj ) return StatusCode::FAILURE;
  if ( path.starts_with( m_rootName.value() ) ) path.remove_prefix( m_rootName.size() );
  if ( path.starts_with( "/" ) ) path.remove_prefix( 1 );
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "registerObject( " << (void*)parentObj << ", " << path << ", (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << " )" << endmsg;
  this->put( path.to_string(), std::unique_ptr<DataObject>( pObject ) );
  return StatusCode::SUCCESS;
}
StatusCode EvtStoreSvc::retrieveObject( IRegistry* pDirectory, boost::string_ref path, DataObject*& pObject )
{
  if ( pDirectory ) return StatusCode::FAILURE;
  if ( path.starts_with( m_rootName.value() ) ) path.remove_prefix( m_rootName.size() );
  if ( path.starts_with( "/" ) ) path.remove_prefix( 1 );
  pObject = const_cast<DataObject*>( this->get_ptr<DataObject>( path.to_string() ) );
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "retrieveObject( " << (void*)pDirectory << ", " << path << ", (DataObject*)" << (void*)pObject
            << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << " )" << endmsg;

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

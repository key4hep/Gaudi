/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators.h>
#include <Gaudi/Arena/Monotonic.h>
#include <GaudiKernel/ConcurrencyFlags.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IHiveWhiteBoard.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/System.h>
#include <tbb/concurrent_queue.h>

#include <ThreadLocalStorage.h>

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <map>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
  using LocalArena = Gaudi::Arena::Monotonic<>;

  template <typename T>
  using LocalAlloc = Gaudi::Allocator::MonotonicArena<T>;

  using pool_string = std::basic_string<char, std::char_traits<char>, LocalAlloc<char>>;

  class Entry final : public IRegistry {
    std::unique_ptr<DataObject>        m_data;
    std::unique_ptr<IOpaqueAddress>    m_addr;
    pool_string                        m_identifierStorage;
    mutable std::optional<std::string> m_identifier;
    static IDataProviderSvc*           s_svc;

  public:
    using allocator_type = LocalAlloc<char>;
    static void setDataProviderSvc( IDataProviderSvc* p ) { s_svc = p; }

    Entry( std::string_view id, std::unique_ptr<DataObject> data, std::unique_ptr<IOpaqueAddress> addr,
           allocator_type alloc ) noexcept
        : m_data{ std::move( data ) }, m_addr{ std::move( addr ) }, m_identifierStorage{ id, alloc } {
      if ( m_data ) m_data->setRegistry( this );
      if ( m_addr ) m_addr->setRegistry( this );
    }
    Entry( const Entry& )                = delete;
    Entry& operator=( const Entry& rhs ) = delete;
    Entry( Entry&& rhs )                 = delete;
    Entry& operator=( Entry&& rhs )      = delete;

    // required by IRegistry...
    unsigned long    addRef() override { return -1; }
    unsigned long    release() override { return -1; }
    const name_type& name() const override {
      // should really be from last '/' onward...
      if ( !m_identifier ) m_identifier.emplace( m_identifierStorage );
      return *m_identifier;
    }
    const id_type& identifier() const override {
      if ( !m_identifier ) m_identifier.emplace( m_identifierStorage );
      return *m_identifier;
    }
    std::string_view  identifierView() const { return m_identifierStorage; }
    IDataProviderSvc* dataSvc() const override { return s_svc; }
    DataObject*       object() const override { return const_cast<DataObject*>( m_data.get() ); }
    IOpaqueAddress*   address() const override { return m_addr.get(); }
    void              setAddress( IOpaqueAddress* iAddr ) override {
      m_addr.reset( iAddr );
      if ( m_addr ) m_addr->setRegistry( this );
    }
  };
  IDataProviderSvc* Entry::s_svc = nullptr;

  using UnorderedMap =
      std::unordered_map<std::string_view, Entry, std::hash<std::string_view>, std::equal_to<std::string_view>,
                         LocalAlloc<std::pair<const std::string_view, Entry>>>;
  using OrderedMap = std::map<std::string_view, Entry>;

  template <typename Map = UnorderedMap>
  class Store {
    LocalArena  m_resource;
    std::size_t m_est_size;
    // Optional purely to make [re]construction simpler, should "always" be valid
    std::optional<Map> m_store{ std::in_place, &m_resource };
    static_assert( std::is_same_v<typename Map::key_type, std::string_view> );

    const auto& emplace( std::string_view k, std::unique_ptr<DataObject> d, std::unique_ptr<IOpaqueAddress> a = {} ) {
      // tricky way to insert a string_view key which points to the
      // string contained in the mapped type...
      auto [i, b] = m_store->try_emplace( k, k, std::move( d ), std::move( a ), &m_resource );
      if ( !b ) throw std::runtime_error( "failed to insert " + std::string{ k } );
      auto nh  = m_store->extract( i );
      nh.key() = nh.mapped().identifierView(); // "re-point" key to the string contained in the Entry
      auto r   = m_store->insert( std::move( nh ) );
      if ( !r.inserted ) throw std::runtime_error( "failed to insert " + std::string{ k } );
      return r.position->second;
    }

  public:
    Store( std::size_t est_size, std::size_t pool_size ) : m_resource{ pool_size }, m_est_size{ est_size } {}
    [[nodiscard]] bool        empty() const { return m_store->empty(); }
    [[nodiscard]] std::size_t size() const { return m_store->size(); }
    [[nodiscard]] std::size_t used_bytes() const noexcept { return m_resource.size(); }
    [[nodiscard]] std::size_t used_blocks() const noexcept { return m_resource.num_blocks(); }
    [[nodiscard]] std::size_t used_buckets() const { return m_store->bucket_count(); }
    [[nodiscard]] std::size_t num_allocations() const noexcept { return m_resource.num_allocations(); }

    void reset() {
      m_store.reset();                            // kill the old map
      m_resource.reset();                         // tell the memory pool it can start re-using its resources
      m_store.emplace( m_est_size, &m_resource ); // initialise the new map with a sane number of buckets
    }

    const DataObject* put( std::string_view k, std::unique_ptr<DataObject> data,
                           std::unique_ptr<IOpaqueAddress> addr = {} ) {
      return emplace( k, std::move( data ), std::move( addr ) ).object();
    }
    const DataObject* get( std::string_view k ) const noexcept {
      const Entry* d = find( k );
      return d ? d->object() : nullptr;
    }
    const Entry* find( std::string_view k ) const noexcept {
      auto i = m_store->find( k );
      return i != m_store->end() ? &( i->second ) : nullptr;
    }

    [[nodiscard]] auto begin() const noexcept { return m_store->begin(); }
    [[nodiscard]] auto end() const noexcept { return m_store->end(); }
    void               clear() noexcept { m_store->clear(); }
    auto               erase( std::string_view k ) { return m_store->erase( k ); }
    template <typename Predicate>
    void erase_if( Predicate p ) {
      auto i   = m_store->begin();
      auto end = m_store->end();
      while ( i != end ) {
        if ( std::invoke( p, std::as_const( *i ) ) )
          i = m_store->erase( i );
        else
          ++i;
      }
    }
  };

  StatusCode dummy( std::string s ) {
    std::string trace;
    System::backTrace( trace, 6, 2 );
    throw std::logic_error{ "Unsupported Function Called: " + s + "\n" + trace };
    return StatusCode::FAILURE;
  }

  std::string_view normalize_path( std::string_view path, std::string_view prefix ) {
    if ( path.size() >= prefix.size() && std::equal( prefix.begin(), prefix.end(), path.begin() ) )
      path.remove_prefix( prefix.size() );
    if ( !path.empty() && path.front() == '/' ) path.remove_prefix( 1 );
    return path;
  }

  std::unique_ptr<DataObject> createObj( IConversionSvc& cnv, IOpaqueAddress& addr ) {
    DataObject* pObject = nullptr;
    auto        status  = cnv.createObj( &addr, pObject ); // Call data loader
    auto        object  = std::unique_ptr<DataObject>( pObject );
    if ( status.isFailure() ) object.reset();
    return object;
  }

  // HiveWhiteBoard helpers
  struct Partition final {
    // Use optional to allow re-constructing in-place without an ugly
    // exception-unsafe placement-new conconction, and also to make it easier
    // to pass constructor arguments to Store<>.
    std::optional<Store<>> store;
    int                    eventNumber = -1;
  };

  template <typename T, typename Mutex = std::recursive_mutex, typename ReadLock = std::scoped_lock<Mutex>,
            typename WriteLock = ReadLock>
  class Synced {
    T             m_obj;
    mutable Mutex m_mtx;

  public:
    template <typename F>
    decltype( auto ) with_lock( F&& f ) {
      WriteLock lock{ m_mtx };
      return f( m_obj );
    }
    template <typename F>
    decltype( auto ) with_lock( F&& f ) const {
      ReadLock lock{ m_mtx };
      return f( m_obj );
    }
  };
  // transform an f(T) into an f(Synced<T>)
  template <typename Fun>
  auto with_lock( Fun&& f ) {
    return [f = std::forward<Fun>( f )]( auto& p ) -> decltype( auto ) { return p.with_lock( f ); };
  }

  TTHREAD_TLS( Synced<Partition>* ) s_current = nullptr;

  template <typename Fun>
  StatusCode fwd( Fun&& f ) {
    return s_current ? s_current->with_lock( std::forward<Fun>( f ) )
                     : StatusCode{ IDataProviderSvc::Status::INVALID_ROOT };
  }

} // namespace

/**
 * @class EvtStoreSvc
 *
 * Use a minimal event store implementation, and adds
 * everything required to satisfy the IDataProviderSvc, IDataManagerSvc and IHiveWhiteBoard
 * interfaces by throwing exceptions except when the functionality is really needed...
 *
 * @author Gerhard Raven
 * @version 1.0
 */
class GAUDI_API EvtStoreSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc, IHiveWhiteBoard> {
  Gaudi::Property<CLID>        m_rootCLID{ this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry" };
  Gaudi::Property<std::string> m_rootName{ this, "RootName", "/Event", "name of root entry" };
  Gaudi::Property<bool>        m_forceLeaves{ this, "ForceLeaves", false,
                                       "force creation of default leaves on registerObject" };
  Gaudi::Property<std::string> m_loader{ this, "DataLoader", "EventPersistencySvc" };
  Gaudi::Property<size_t>      m_slots{ this, "EventSlots", 1, "number of event slots" };
  Gaudi::Property<bool>        m_printPoolStats{ this, "PrintPoolStats", false, "Print memory pool statistics" };
  Gaudi::Property<std::size_t> m_poolSize{ this, "PoolSize", 1024, "Initial per-event memory pool size [KiB]" };
  Gaudi::Property<std::size_t> m_estStoreBuckets{ this, "StoreBuckets", 100,
                                                  "Estimated number of buckets in the store" };
  mutable Gaudi::Accumulators::AveragingCounter<std::size_t> m_usedPoolSize, m_servedPoolAllocations,
      m_usedPoolAllocations, m_storeEntries, m_storeBuckets;

  // Convert to bytes
  std::size_t poolSize() const { return m_poolSize * 1024; }

  void fillStats( Partition& p ) const {
    if ( !m_printPoolStats ) return;
    auto n_allocs = p.store->num_allocations();
    if ( n_allocs ) {
      m_storeEntries += p.store->size();
      m_usedPoolSize += p.store->used_bytes();
      m_storeBuckets += p.store->used_buckets();
      m_usedPoolAllocations += p.store->used_blocks();
      m_servedPoolAllocations += n_allocs;
    }
  }

  void initStore( Partition& p ) const {
    if ( p.store ) {
      // re-use the existing memory pool
      p.store->reset();
    } else {
      p.store.emplace( m_estStoreBuckets, poolSize() );
    }
  }

  SmartIF<IConversionSvc> m_dataLoader;

  /// Items to be pre-loaded
  std::vector<DataStoreItem> m_preLoads;

  /// The actual store(s)
  std::vector<Synced<Partition>> m_partitions;

  tbb::concurrent_queue<size_t> m_freeSlots;

  Gaudi::Property<std::vector<std::string>> m_inhibitPrefixes{
      this,
      "InhibitedPathPrefixes",
      {},
      "Prefixes of TES locations that will not be loaded by the persistency service " };
  Gaudi::Property<bool> m_followLinksToAncestors{
      this, "FollowLinksToAncestors", true,
      "Load objects which reside in files other than the one corresponding to the root of the event store" };
  std::string_view m_onlyThisID; // let's be a bit risky... we 'know' when the underlying string goes out of scope...

public:
  using extends::extends;

  CLID               rootCLID() const override;
  const std::string& rootName() const override;
  StatusCode         setDataLoader( IConversionSvc* svc, IDataProviderSvc* dpsvc ) override;

  size_t     allocateStore( int evtnumber ) override;
  StatusCode freeStore( size_t partition ) override;
  size_t     freeSlots() override { return m_freeSlots.unsafe_size(); }
  StatusCode selectStore( size_t partition ) override;
  StatusCode clearStore() override;
  StatusCode clearStore( size_t partition ) override;
  StatusCode setNumberOfStores( size_t slots ) override;
  size_t     getNumberOfStores() const override { return m_slots; }
  size_t     getPartitionNumber( int eventnumber ) const override;
  bool       exists( const DataObjID& id ) override {
    DataObject* pObject{ nullptr };
    return findObject( id.fullKey(), pObject ).isSuccess();
  }

  StatusCode objectParent( const DataObject*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectParent( const IRegistry*, IRegistry*& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const DataObject*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }
  StatusCode objectLeaves( const IRegistry*, std::vector<IRegistry*>& ) override { return dummy( __FUNCTION__ ); }

  StatusCode clearSubTree( std::string_view ) override;
  StatusCode clearSubTree( DataObject* obj ) override {
    return obj && obj->registry() ? clearSubTree( obj->registry()->identifier() ) : StatusCode::FAILURE;
  }

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
    return !obj              ? unregisterObject( sr )
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
  StatusCode unlinkObject( std::string_view ) override { return dummy( __FUNCTION__ ); }

  StatusCode initialize() override {
    Entry::setDataProviderSvc( this );
    extends::initialize().ignore();
    if ( !setNumberOfStores( m_slots ).isSuccess() ) {
      error() << "Cannot set number of slots" << endmsg;
      return StatusCode::FAILURE;
    }
    m_partitions = std::vector<Synced<Partition>>( m_slots );
    // m_partitions is now full of empty std::optionals, fill them now.
    for ( auto& synced_p : m_partitions ) {
      synced_p.with_lock( [this]( Partition& p ) { initStore( p ); } );
    }
    for ( size_t i = 0; i < m_slots; i++ ) { m_freeSlots.push( i ); }
    selectStore( 0 ).ignore();

    auto loader = serviceLocator()->service( m_loader ).as<IConversionSvc>().get();
    if ( !loader ) {
      error() << "Cannot get IConversionSvc " << m_loader.value() << endmsg;
      return StatusCode::FAILURE;
    }
    return setDataLoader( loader, nullptr );
  }
  StatusCode finalize() override {
    if ( m_printPoolStats ) {
      info() << "Mean memory pool usage: " << float( 1e-3f * float( m_usedPoolSize.mean() ) ) << " KiB serving "
             << float( m_servedPoolAllocations.mean() ) << " allocations from " << float( m_usedPoolAllocations.mean() )
             << " to produce " << float( m_storeEntries.mean() ) << " entries in " << float( m_storeBuckets.mean() )
             << " buckets" << endmsg;
    }
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
/// Allocate a store partition for a given event number
size_t EvtStoreSvc::allocateStore( int evtnumber ) {
  // take next free slot in the list
  size_t slot = std::string::npos;
  if ( m_freeSlots.try_pop( slot ) ) {
    assert( slot != std::string::npos );
    assert( slot < m_partitions.size() );
    [[maybe_unused]] auto prev = m_partitions[slot].with_lock(
        [evtnumber]( Partition& p ) { return std::exchange( p.eventNumber, evtnumber ); } );
    assert( prev == -1 ); // or whatever value represents 'free'
  }
  return slot;
}
/// Set the number of event slots (copies of DataSvc objects).
StatusCode EvtStoreSvc::setNumberOfStores( size_t slots ) {
  if ( slots < size_t{ 1 } ) {
    error() << "Invalid number of slots (" << slots << ")" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( FSMState() == Gaudi::StateMachine::INITIALIZED || FSMState() == Gaudi::StateMachine::RUNNING ) {
    error() << "Too late to change the number of slots!" << endmsg;
    return StatusCode::FAILURE;
  }
  m_slots = slots;
  Gaudi::Concurrency::ConcurrencyFlags::setNumConcEvents( slots );
  return StatusCode::SUCCESS;
}
/// Get the partition number corresponding to a given event
size_t EvtStoreSvc::getPartitionNumber( int eventnumber ) const {
  auto i = std::find_if( begin( m_partitions ), end( m_partitions ),
                         with_lock( [eventnumber]( const Partition& p ) { return p.eventNumber == eventnumber; } ) );
  return i != end( m_partitions ) ? std::distance( begin( m_partitions ), i ) : std::string::npos;
}
/// Activate a partition object. The  identifies the partition uniquely.
StatusCode EvtStoreSvc::selectStore( size_t partition ) {
  s_current = &m_partitions[partition];
  return StatusCode::SUCCESS;
}
/// Free a store partition
StatusCode EvtStoreSvc::freeStore( size_t partition ) {
  assert( partition < m_partitions.size() );
  auto prev = m_partitions[partition].with_lock( []( Partition& p ) { return std::exchange( p.eventNumber, -1 ); } );
  if ( prev == -1 ) return StatusCode::FAILURE; // double free -- should never happen!
  m_freeSlots.push( partition );
  return StatusCode::SUCCESS;
}
/// Remove all data objects in one 'slot' of the data store.
StatusCode EvtStoreSvc::clearStore( size_t partition ) {
  m_onlyThisID = {};
  return m_partitions[partition].with_lock( [this]( Partition& p ) {
    fillStats( p );
    initStore( p ); // replace with a clean store
    return StatusCode::SUCCESS;
  } );
}
StatusCode EvtStoreSvc::clearSubTree( std::string_view top ) {
  top = normalize_path( top, rootName() );
  return fwd( [&]( Partition& p ) {
    p.store->erase_if( [top]( const auto& value ) { return value.first.starts_with( top ); } );
    return StatusCode::SUCCESS;
  } );
}
StatusCode EvtStoreSvc::clearStore() {
  m_onlyThisID = {};
  return fwd( [this]( Partition& p ) {
    fillStats( p );
    initStore( p ); // replace with a clean store
    return StatusCode::SUCCESS;
  } );
}
StatusCode EvtStoreSvc::traverseSubTree( std::string_view top, IDataStoreAgent* pAgent ) {
  return fwd( [&]( Partition& p ) {
    top                              = normalize_path( top, rootName() );
    unsigned int nbSlashesInRootName = std::count( rootName().begin(), rootName().end(), '/' );
    auto         cmp = []( const Entry* lhs, const Entry* rhs ) { return lhs->identifier() < rhs->identifier(); };
    std::set<const Entry*, decltype( cmp )> keys{ std::move( cmp ) };
    for ( const auto& v : *p.store ) {
      if ( v.second.identifier().starts_with( top ) ) keys.insert( &v.second );
    }
    auto k = keys.begin();
    while ( k != keys.end() ) {
      const auto& id     = ( *k )->identifier();
      int         level  = std::count( id.begin(), id.end(), '/' ) + nbSlashesInRootName;
      bool        accept = pAgent->analyse( const_cast<Entry*>( *( k++ ) ), level );
      if ( !accept ) {
        k = std::find_if_not( k, keys.end(), [&id]( const auto& e ) { return e->identifier().starts_with( id ); } );
      }
    }
    return StatusCode::SUCCESS;
  } );
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, DataObject* pObject ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "setRoot( " << root_path << ", (DataObject*)" << (void*)pObject << " )" << endmsg;
  }
  if ( !fwd( []( Partition& p ) {
          return p.store->empty() ? StatusCode::SUCCESS : StatusCode::FAILURE;
        } ).isSuccess() ) {
    throw GaudiException{ "setRoot called with non-empty store", "EvtStoreSvc", StatusCode::FAILURE };
  }
  return registerObject( nullptr, root_path, pObject );
}
StatusCode EvtStoreSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) {
  auto rootAddr = std::unique_ptr<IOpaqueAddress>( pRootAddr );
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "setRoot( " << root_path << ", (IOpaqueAddress*)" << rootAddr.get();
    if ( rootAddr ) debug() << "[ " << rootAddr->par()[0] << ", " << rootAddr->par()[1] << " ]";
    debug() << " )" << endmsg;
  }
  if ( !fwd( []( Partition& p ) {
          return p.store->empty() ? StatusCode::SUCCESS : StatusCode::FAILURE;
        } ).isSuccess() ) {
    throw GaudiException{ "setRoot called with non-empty store", "EvtStoreSvc", StatusCode::FAILURE };
  }
  if ( !rootAddr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  if ( !m_followLinksToAncestors ) m_onlyThisID = rootAddr->par()[0];
  auto object = createObj( *m_dataLoader, *rootAddr ); // Call data loader
  if ( !object ) return Status::INVALID_OBJECT;
  if ( msgLevel( MSG::DEBUG ) ) { debug() << "Root Object " << root_path << " created " << endmsg; }
  LocalArena dummy_arena{ root_path.size() + 1 };
  auto       dummy = Entry{ root_path, {}, {}, &dummy_arena };
  object->setRegistry( &dummy );
  rootAddr->setRegistry( &dummy );
  auto status = m_dataLoader->fillObjRefs( rootAddr.get(), object.get() );
  if ( status.isSuccess() ) {
    auto pObject = object.get();
    status       = registerObject( nullptr, root_path, object.release() );
    if ( status.isSuccess() ) pObject->registry()->setAddress( rootAddr.release() );
  }
  return status;
}
StatusCode EvtStoreSvc::registerAddress( std::string_view path, IOpaqueAddress* pAddr ) {
  return registerAddress( nullptr, path, pAddr );
}
StatusCode EvtStoreSvc::registerAddress( IRegistry* pReg, std::string_view path, IOpaqueAddress* pAddr ) {
  auto addr = std::unique_ptr<IOpaqueAddress>( pAddr );
  if ( !addr ) return Status::INVALID_OBJ_ADDR; // Precondition: Address must be valid
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "registerAddress( (IRegistry*)" << (void*)pReg << ", " << path << ", (IOpaqueAddress*)" << addr.get()
            << "[ " << addr->par()[0] << ", " << addr->par()[1] << " ]"
            << " )" << endmsg;
  }
  if ( path.empty() || path[0] != '/' ) return StatusCode::FAILURE;
  if ( !m_onlyThisID.empty() && addr->par()[0] != m_onlyThisID ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Attempt to load " << addr->par()[1] << " from file " << addr->par()[0] << " blocked -- different file"
              << endmsg;
    return StatusCode::SUCCESS;
  }
  if ( std::any_of(
           m_inhibitPrefixes.begin(), m_inhibitPrefixes.end(),
           [addrPath = addr->par()[1]]( std::string_view prefix ) { return addrPath.starts_with( prefix ); } ) ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Attempt to load " << addr->par()[1] << " from file " << addr->par()[0] << " blocked -- path inhibited"
              << endmsg;
    return StatusCode::SUCCESS;
  }

  auto object = createObj( *m_dataLoader, *addr ); // Call data loader
  if ( !object ) return Status::INVALID_OBJECT;
  auto fullpath = ( pReg ? pReg->identifier() : m_rootName.value() ) + std::string{ path };
  // the data loader expects the path _including_ the root
  LocalArena dummy_arena{ fullpath.size() + 1 };
  auto       dummy = Entry{ fullpath, {}, {}, &dummy_arena };
  object->setRegistry( &dummy );
  addr->setRegistry( &dummy );
  auto status = m_dataLoader->fillObjRefs( addr.get(), object.get() );
  if ( !status.isSuccess() ) return status;
  // note: put will overwrite the registry in pObject to point at the
  //       one actually used -- so we do not dangle, pointing at dummy beyond its
  //       lifetime
  if ( msgLevel( MSG::DEBUG ) ) {
    auto ptr = object.get();
    debug() << "registerAddress: " << std::quoted( normalize_path( fullpath, rootName() ) ) << " (DataObject*)"
            << static_cast<void*>( ptr ) << ( ptr ? " -> " + System::typeinfoName( typeid( *ptr ) ) : std::string{} )
            << endmsg;
  }
  fwd( [&]( Partition& p ) {
    p.store->put( normalize_path( fullpath, rootName() ), std::move( object ), std::move( addr ) );
    return StatusCode::SUCCESS;
  } ).ignore();
  return status;
}
StatusCode EvtStoreSvc::registerObject( std::string_view parentPath, std::string_view objectPath,
                                        DataObject* pObject ) {
  return parentPath.empty()
             ? registerObject( nullptr, objectPath, pObject )
             : registerObject( nullptr, std::string{ parentPath }.append( "/" ).append( objectPath ), pObject );
}
StatusCode EvtStoreSvc::registerObject( DataObject* parentObj, std::string_view path, DataObject* pObject ) {
  if ( parentObj ) return StatusCode::FAILURE;
  return fwd( [&, object = std::unique_ptr<DataObject>( pObject ),
               path = normalize_path( path, rootName() )]( Partition& p ) mutable {
    if ( m_forceLeaves ) {
      auto dir = path;
      for ( auto i = dir.rfind( '/' ); i != std::string_view::npos; i = dir.rfind( '/' ) ) {
        dir = dir.substr( 0, i );
        if ( !p.store->find( dir ) ) {
          if ( msgLevel( MSG::DEBUG ) ) {
            debug() << "registerObject: adding directory " << std::quoted( dir ) << endmsg;
          }
          p.store->put( dir, std::unique_ptr<DataObject>{} );
        }
      }
    }
    if ( msgLevel( MSG::DEBUG ) ) {
      auto ptr = object.get();
      debug() << "registerObject: " << std::quoted( path ) << " (DataObject*)" << static_cast<void*>( ptr )
              << ( ptr ? " -> " + System::typeinfoName( typeid( *ptr ) ) : std::string{} ) << endmsg;
    }
    p.store->put( path, std::move( object ) );
    return StatusCode::SUCCESS;
  } );
}
StatusCode EvtStoreSvc::retrieveObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) {
  if ( pDirectory ) return StatusCode::FAILURE;
  return fwd( [&]( Partition& p ) {
    path    = normalize_path( path, rootName() );
    pObject = const_cast<DataObject*>( p.store->get( path ) );
    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "retrieveObject: " << std::quoted( path ) << " (DataObject*)" << (void*)pObject
              << ( pObject ? " -> " + System::typeinfoName( typeid( *pObject ) ) : std::string{} ) << endmsg;
    }
    return pObject ? StatusCode::SUCCESS : StatusCode::FAILURE;
  } );
}
StatusCode EvtStoreSvc::findObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) {
  return retrieveObject( pDirectory, path, pObject );
}
StatusCode EvtStoreSvc::findObject( std::string_view fullPath, DataObject*& pObject ) {
  return retrieveObject( nullptr, fullPath, pObject );
}
StatusCode EvtStoreSvc::unregisterObject( std::string_view sr ) {
  return fwd( [&]( Partition& p ) { return p.store->erase( sr ) != 0 ? StatusCode::SUCCESS : StatusCode::FAILURE; } );
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

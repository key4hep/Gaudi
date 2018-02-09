//====================================================================
//	RegistryEntry.cpp
//--------------------------------------------------------------------
//
//	Package    : DataSvc ( The LHCb Offline System)
//
//  Description: implementation of the Transient data store
//
//	Author     : M.Frank
//
//====================================================================
// STL include files
#include <algorithm>

// Interfaces
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Framework include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/StatusCode.h"
namespace
{
  constexpr char SEPARATOR{'/'};

  auto make_lt( boost::string_ref sr )
  {
    // skip leading '/' in comparison
    return [sr]( const auto& arg ) { return arg->name().compare( 1, std::string::npos, sr.data(), sr.size() ) < 0; };
  }
  auto make_eq( boost::string_ref sr )
  {
    // skip leading '/' in comparison
    return [sr]( const auto& arg ) { return sr == boost::string_ref{arg->name()}.substr( 1 ); };
  }

  namespace details_log
  {
    template <typename C>
    auto find_( C& c, boost::string_ref arg )
    {
      auto i = std::partition_point( begin( c ), end( c ), make_lt( arg ) );
      return ( i == end( c ) || make_eq( arg )( *i ) ) ? i : end( c );
    }

    template <typename C, typename E>
    void insert_( C& c, E entry )
    {
      const auto& name = entry->name();
      c.insert( std::partition_point( begin( c ), end( c ), [&name]( const auto& arg ) { return arg->name() < name; } ),
                std::move( entry ) );
    }
  }

  namespace details_linear
  {
    template <typename C>
    auto find_( C& c, boost::string_ref arg )
    {
      return std::find_if( begin( c ), end( c ), make_eq( arg ) );
    }

    template <typename C, typename E>
    void insert_( C& c, E entry )
    {
      c.push_back( std::move( entry ) );
    }
  }

  using namespace details_log;
}

/// Standard Constructor
DataSvcHelpers::RegistryEntry::RegistryEntry( std::string path, RegistryEntry* parent )
    : m_path( std::move( path ) ), m_pParent( parent )
{
  auto sep = m_path.rfind( SEPARATOR );
  if ( m_path.front() != SEPARATOR ) m_path.insert( 0, 1, SEPARATOR );
  if ( sep != std::string::npos ) m_path.erase( 0, sep );
  assemblePath( m_fullpath );
}

/// Standard destructor
DataSvcHelpers::RegistryEntry::~RegistryEntry()
{
  if ( m_pObject ) {
    if ( !m_isSoft ) m_pObject->setRegistry( nullptr );
    m_pObject->release();
  }
  if ( m_pAddress ) {
    if ( !m_isSoft ) m_pAddress->setRegistry( nullptr );
    m_pAddress->release();
  }
}

/// Set new parent pointer
void DataSvcHelpers::RegistryEntry::setParent( RegistryEntry* pParent )
{
  m_pParent = pParent;
  m_fullpath.clear();
  assemblePath( m_fullpath );
}

/// Create soft link
void DataSvcHelpers::RegistryEntry::makeSoft( DataObject* pObject )
{
  m_isSoft = true;
  setObject( pObject );
  //  if ( 0 != m_pObject )   { // Useless: This justs sets my own address again...
  //    setAddress(m_pObject->address());
  //  }
}

/// Create soft link
void DataSvcHelpers::RegistryEntry::makeSoft( IOpaqueAddress* pAddress )
{
  m_isSoft = true;
  setAddress( pAddress );
}

/// Create hard link
void DataSvcHelpers::RegistryEntry::makeHard( DataObject* pObject )
{
  makeSoft( pObject );
  m_isSoft = false;
  if ( m_pObject ) m_pObject->setRegistry( this );
  if ( m_pAddress ) m_pAddress->setRegistry( this );
}

/// Create hard link
void DataSvcHelpers::RegistryEntry::makeHard( IOpaqueAddress* pAddress )
{
  m_isSoft = false;
  setAddress( pAddress );
}

/// Update Opaque address of registry entry
void DataSvcHelpers::RegistryEntry::setAddress( IOpaqueAddress* pAddress )
{
  if ( pAddress ) {
    pAddress->addRef();
    pAddress->setRegistry( this );
  }
  if ( m_pAddress ) m_pAddress->release();
  m_pAddress = pAddress;
}

/// Set object pointer of data store item
void DataSvcHelpers::RegistryEntry::setObject( DataObject* pObject )
{
  if ( pObject ) {
    pObject->addRef();
    if ( !isSoft() ) pObject->setRegistry( this );
  }
  if ( m_pObject ) m_pObject->release();
  m_pObject = pObject;
}

/// Remove an object from the container
long DataSvcHelpers::RegistryEntry::remove( IRegistry* obj )
{
  try {
    auto i = std::find_if( m_store.begin(), m_store.end(), [obj]( const auto& j ) { return j.get() == obj; } );
    if ( i != m_store.end() ) m_store.erase( i );
  } catch ( ... ) {
  }
  return m_store.size();
}

/// Remove entry from data store
StatusCode DataSvcHelpers::RegistryEntry::remove( boost::string_ref nam )
{
  if ( nam.front() == SEPARATOR ) nam.remove_prefix( 1 );
  auto i = find_( m_store, nam );
  // if the requested object is not present, this is an error....
  if ( i == m_store.end() ) return StatusCode::FAILURE;
  m_store.erase( i );
  return StatusCode::SUCCESS;
}

/// Internal method to add entries
std::unique_ptr<DataSvcHelpers::RegistryEntry> DataSvcHelpers::RegistryEntry::i_create( std::string nam )
{
  if ( nam.front() != SEPARATOR ) nam.insert( 0, 1, SEPARATOR );
  // if this object is already present, this is an error....
  return find_( m_store, nam ) == m_store.end() ? std::make_unique<RegistryEntry>( std::move( nam ), this )
                                                : std::unique_ptr<RegistryEntry>{};
}

/// Add object to the container
long DataSvcHelpers::RegistryEntry::add( std::unique_ptr<RegistryEntry> pEntry )
{
  // TODO: can we change m_store to std::vector<RegistryEntry> instead?
  // TODO: if so, should make sure that a RegistryEntry can be std::move'ed
  // No: the address of the registry entry is assumed to be stable ;-(
  //     it is eg. exposed in the call to 'setRegistry' below
  try {
    pEntry->setDataSvc( m_pDataProviderSvc );
    pEntry->setParent( this );
    if ( !pEntry->isSoft() && pEntry->address() ) {
      pEntry->address()->setRegistry( pEntry.get() );
    }
    insert_( m_store, std::move( pEntry ) );
  } catch ( ... ) {
  }
  return m_store.size();
}

/// Add entry to the current data store item
StatusCode DataSvcHelpers::RegistryEntry::add( std::string name, DataObject* pObject, bool is_soft )
{
  auto entry = i_create( std::move( name ) );
  if ( !entry ) return StatusCode::FAILURE;
  ( is_soft ) ? entry->makeSoft( pObject ) : entry->makeHard( pObject );
  add( std::move( entry ) );
  return StatusCode::SUCCESS;
}

/// Add entry to the current data store item
StatusCode DataSvcHelpers::RegistryEntry::add( std::string name, IOpaqueAddress* pAddress, bool is_soft )
{
  auto entry = i_create( std::move( name ) );
  if ( !entry ) return StatusCode::FAILURE;
  ( is_soft ) ? entry->makeSoft( pAddress ) : entry->makeHard( pAddress );
  add( std::move( entry ) );
  return StatusCode::SUCCESS;
}

/// Try to find an object identified by its pointer
IRegistry* DataSvcHelpers::RegistryEntry::i_find( const IRegistry* obj ) const
{
  auto i = std::find_if( m_store.begin(), m_store.end(), [obj]( const auto& j ) { return j.get() == obj; } );
  return ( i != m_store.end() ) ? i->get() : nullptr;
}

/// Find identified leaf in this registry node
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_find( boost::string_ref path ) const
{
  if ( path.front() == SEPARATOR ) path.remove_prefix( 1 ); // strip leading '/', if present
  while ( !path.empty() ) {
    // check that the chars of path prior to / are the same as regEnt->name()
    // (i.e. match { nam:"/Ab"  path:"/Ab/C"}
    // but not     { nam:"/Abc" path:"/Ab/C"})
    auto loc1  = path.find( SEPARATOR );
    auto cpath = path.substr( 0, loc1 );
    if ( loc1 != boost::string_ref::npos ) {
      path.remove_prefix( loc1 + 1 );
    } else {
      path.clear();
    }
    auto i = find_( m_store, cpath );
    if ( i != m_store.end() ) return path.empty() ? i->get() : ( *i )->i_find( path );
    // If this node is "/NodeA", this part allows to find "/NodeA/NodeB" as
    // our "/NodeB" child.
    if ( !make_eq( cpath )( this ) ) break;
  }
  return nullptr;
}

/// Find identified leaf in this registry node
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_find( const DataObject* key ) const
{
  if ( key ) {
    if ( key == m_pObject ) return const_cast<RegistryEntry*>( this );
    // Look in the immediate level:
    RegistryEntry* result = i_find( key->registry() );
    if ( result ) return result;
    // Go levels down
    try {
      for ( const auto& i : m_store ) {
        result = i->i_find( key );
        if ( result ) return result;
      }
    } catch ( ... ) {
    }
  }
  return nullptr;
}

// Traverse registry tree
StatusCode DataSvcHelpers::RegistryEntry::traverseTree( IDataStoreAgent* pAgent, int level )
{
  bool go_down = pAgent->analyse( this, level );
  StatusCode status;
  if ( go_down ) {
    try {
      for ( auto& i : m_store ) {
        i->traverseTree( pAgent, level + 1 ).ignore();
      }
    } catch ( ... ) {
      status = StatusCode::FAILURE;
    }
  }
  return status;
}

// Recursive helper to assemble the full path name of the entry
void DataSvcHelpers::RegistryEntry::assemblePath( std::string& buffer ) const
{
  if ( m_pParent ) m_pParent->assemblePath( buffer );
  buffer += m_path;
}

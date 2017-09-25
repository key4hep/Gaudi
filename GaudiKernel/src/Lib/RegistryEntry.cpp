//====================================================================
//	RegistryEntry.cpp
//--------------------------------------------------------------------
//
//	Package    : DataSvc ( The LHCb Offline System)
//
//  Description: implementation of the Transient data store
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// | 03/02/99| Protect dynamic_cast with try-catch clauses  | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define DATASVC_REGISTRYENTRY_CPP

// STL include files
#include <algorithm>

// Interfaces
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Framework include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/RegistryEntry.h"

// If you absolutely need optimization: switch off dynamic_cast.
// This improves access to the data store roughly by more than 10 %
// for balanced trees.
//
// M.Frank
//
#define CAST_REGENTRY( x, y ) dynamic_cast<x>( y )
//#define CAST_REGENTRY(x,y) (x)(y)
constexpr char SEPARATOR{'/'};

/// Standard Constructor
DataSvcHelpers::RegistryEntry::RegistryEntry( std::string path, RegistryEntry* parent )
    : m_path( std::move( path ) ), m_pParent( parent )
{
  std::string::size_type sep = m_path.rfind( SEPARATOR );
  if ( m_path.front() != SEPARATOR ) m_path.insert( 0, 1, SEPARATOR );
  if ( sep != std::string::npos ) m_path.erase( 0, sep );
  assemblePath( m_fullpath );
  addRef();
}

/// Standard destructor
DataSvcHelpers::RegistryEntry::~RegistryEntry()
{
  deleteElements();
  if ( m_pObject ) {
    if ( !m_isSoft ) m_pObject->setRegistry( nullptr );
    m_pObject->release();
  }
  if ( m_pAddress ) {
    if ( !m_isSoft ) m_pAddress->setRegistry( nullptr );
    m_pAddress->release();
  }
}

/// Release entry
unsigned long DataSvcHelpers::RegistryEntry::release()
{
  unsigned long cnt = --m_refCount;
  if ( !m_refCount ) delete this;
  return cnt;
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
    RegistryEntry* pEntry = dynamic_cast<RegistryEntry*>( obj );
    auto i                = std::remove( std::begin( m_store ), std::end( m_store ), pEntry );
    if ( i != std::end( m_store ) ) {
      pEntry->release();
      m_store.erase( i, std::end( m_store ) );
    }
  } catch ( ... ) {
  }
  return m_store.size();
}

/// Remove entry from data store
long DataSvcHelpers::RegistryEntry::remove( const std::string& nam )
{
  if ( nam.front() != SEPARATOR ) return remove( SEPARATOR + nam );
  // if this object is already present, this is an error....
  for ( auto& i : m_store ) {
    if ( nam == i->name() ) {
      remove( i );
      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

/// Internal method to add entries
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_create( std::string nam )
{
  if ( nam.front() != SEPARATOR ) nam.insert( 0, 1, SEPARATOR );
  // if this object is already present, this is an error....
  auto not_present =
      std::none_of( std::begin( m_store ), std::end( m_store ), [&]( IRegistry* i ) { return nam == i->name(); } );
  return not_present ? new RegistryEntry( std::move( nam ), this ) : nullptr;
}

/// Add object to the container
long DataSvcHelpers::RegistryEntry::add( IRegistry* obj )
{
  RegistryEntry* pEntry = CAST_REGENTRY( RegistryEntry*, obj );
  return i_add( pEntry );
}

/// Add object to the container
long DataSvcHelpers::RegistryEntry::i_add( RegistryEntry* pEntry )
{
  // TODO: if this is the sole place where items are added to m_store,
  //      and we know here that they must be RegisteryEntry, can we
  //      drop the dynamic_cast every where else???
  // TODO: if so, can we also change m_store to be std::vector<RegistryEntry*>
  //      instead
  // TODO: if so, can we not make it std::vector<RegistryEntry> instead?
  // TODO: if so, should make sure that a RegistryEntry can be std::move'ed
  try {
    pEntry->setDataSvc( m_pDataProviderSvc );
    m_store.push_back( pEntry );
    pEntry->setParent( this );
    if ( !pEntry->isSoft() && pEntry->address() ) {
      pEntry->address()->setRegistry( pEntry );
    }
  } catch ( ... ) {
  }
  return m_store.size();
}

/// Add entry to the current data store item
long DataSvcHelpers::RegistryEntry::add( const std::string& name, DataObject* pObject, bool is_soft )
{
  RegistryEntry* entry = i_create( name );
  if ( !entry ) return StatusCode::FAILURE;
  ( is_soft ) ? entry->makeSoft( pObject ) : entry->makeHard( pObject );
  i_add( entry );
  return StatusCode::SUCCESS;
}

/// Add entry to the current data store item
long DataSvcHelpers::RegistryEntry::add( const std::string& name, IOpaqueAddress* pAddress, bool is_soft )
{
  RegistryEntry* entry = i_create( name );
  if ( !entry ) return StatusCode::FAILURE;
  ( is_soft ) ? entry->makeSoft( pAddress ) : entry->makeHard( pAddress );
  i_add( entry );
  return StatusCode::SUCCESS;
}

/// Delete recursively all elements pending from the current store item
long DataSvcHelpers::RegistryEntry::deleteElements()
{
  for ( auto& i : m_store ) {
    RegistryEntry* entry = CAST_REGENTRY( RegistryEntry*, i );
    if ( entry ) {
      entry->deleteElements();
      entry->release();
    }
  }
  m_store.erase( m_store.begin(), m_store.end() );
  return 0;
}

/// Try to find an object identified by its pointer
IRegistry* DataSvcHelpers::RegistryEntry::i_find( const IRegistry* obj ) const
{
  auto i = std::find( m_store.begin(), m_store.end(), obj );
  return ( i != m_store.end() ) ? ( *i ) : nullptr;
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
    auto i = std::find_if( std::begin( m_store ), std::end( m_store ), [&]( decltype( m_store )::const_reference reg ) {
      return cpath == boost::string_ref{reg->name()}.substr( 1 );
    } );
    if ( i != std::end( m_store ) ) {
      RegistryEntry* regEnt = CAST_REGENTRY( RegistryEntry*, *i );
      return path.empty() ? regEnt : regEnt->i_find( path );
    }
    // If this node is "/NodeA", this part allows to find "/NodeA/NodeB" as
    // our "/NodeB" child.
    if ( cpath != boost::string_ref{m_path}.substr( 1 ) ) break;
  }
  return nullptr;
}

/// Find identified leaf in this registry node
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_find( const DataObject* key ) const
{
  if ( key ) {
    if ( key == m_pObject ) {
      return const_cast<RegistryEntry*>( this );
    }
    // Look in the immediate level:
    RegistryEntry* result = CAST_REGENTRY( RegistryEntry*, i_find( key->registry() ) );
    if ( result ) return result;
    // Go levels down
    for ( const auto& i : m_store ) {
      try {
        const RegistryEntry* entry = CAST_REGENTRY( RegistryEntry*, i );
        result                     = entry->i_find( key );
        if ( result ) return result;
      } catch ( ... ) {
      }
    }
  }
  return nullptr;
}

// Traverse registry tree
long DataSvcHelpers::RegistryEntry::traverseTree( IDataStoreAgent* pAgent, int level )
{
  bool go_down = pAgent->analyse( this, level );
  long status  = StatusCode::SUCCESS;
  if ( go_down ) {
    for ( auto& i : m_store ) {
      try {
        RegistryEntry* entry = CAST_REGENTRY( RegistryEntry*, i );
        entry->traverseTree( pAgent, level + 1 );
      } catch ( ... ) {
        status = StatusCode::FAILURE;
      }
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

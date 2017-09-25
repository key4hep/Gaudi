#include "MultiFileCatalog.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "createGuidAsString.h"
#include <Gaudi/PluginService.h>
#include <algorithm>
#include <functional>
#include <stdexcept>

using namespace Gaudi;
using namespace std;

namespace
{
  template <typename C, typename F>
  F for_each( C& c, F&& f )
  {
    return std::for_each( std::begin( c ), std::end( c ), std::forward<F>( f ) );
  }
}

DECLARE_COMPONENT( MultiFileCatalog )

// ----------------------------------------------------------------------------
MultiFileCatalog::MultiFileCatalog( const std::string& nam, ISvcLocator* svc ) : base_class( nam, svc )
{
  m_catalogNames.declareUpdateHandler( &Gaudi::MultiFileCatalog::propHandler, this );
}
// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::initialize()
{
  if ( !Service::initialize().isSuccess() ) {
    printError( "Failed to initialize service base class.", false );
    return StatusCode::SUCCESS;
  }
  std::string current;
  try {
    for ( const auto& i : m_catalogNames ) {
      current = i;
      addCatalog( i );
    }
    init();
    return StatusCode::SUCCESS;
  } catch ( const std::exception& /* e */ ) {
    printError( "Cannot add file catalog:" + current, false );
  }
  return StatusCode::FAILURE;
}
// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::finalize()
{
  commit();
  for_each( m_catalogs, []( IFileCatalog* i ) { i->release(); } );
  m_catalogs.clear();
  m_started = false;
  return Service::finalize();
}
// ----------------------------------------------------------------------------
/// Create file identifier using UUID mechanism
std::string MultiFileCatalog::createFID() const { return createGuidAsString(); }
// ----------------------------------------------------------------------------
MultiFileCatalog::CSTR MultiFileCatalog::connectInfo() const
{
  static const string s( "MultiCatalog" );
  return s;
}
// ----------------------------------------------------------------------------
IFileCatalog* MultiFileCatalog::getCatalog( CSTR fid, bool throw_if_not, bool writable, bool prt ) const
{
  for ( const auto& c : m_catalogs ) {
    if ( !c || ( writable && c->readOnly() ) ) continue;
    if ( fid.empty() || ( !fid.empty() && c->existsFID( fid ) ) ) return c;
  }
  if ( prt ) {
    printError( "No writable file catalog found which contains FID:" + fid, throw_if_not );
  } else {
    debug() << "No writable file catalog found which contains FID:" << fid << endmsg;
  }
  return nullptr;
}
// ----------------------------------------------------------------------------
IFileCatalog* MultiFileCatalog::findCatalog( CSTR connect, bool must_be_writable ) const
{
  auto i = std::find_if( m_catalogs.begin(), m_catalogs.end(),
                         [&]( const IFileCatalog* f ) { return connect == f->connectInfo(); } );
  return ( i != m_catalogs.end() && ( !must_be_writable || !( *i )->readOnly() ) ) ? *i : nullptr;
}
// ----------------------------------------------------------------------------
MultiFileCatalog::Catalogs::iterator MultiFileCatalog::i_findCatalog( CSTR connect, bool must_be_writable )
{
  auto i = std::find_if( m_catalogs.begin(), m_catalogs.end(),
                         [&]( const IFileCatalog* f ) { return connect == f->connectInfo(); } );
  if ( i != m_catalogs.end() && must_be_writable && ( *i )->readOnly() ) {
    i = m_catalogs.end();
  }
  return i;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::printError( CSTR msg, bool rethrow ) const
{
  if ( rethrow ) {
    fatal() << msg << endmsg;
    throw runtime_error( "Catalog> " + msg );
  }
  error() << msg << endmsg;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::addCatalog( CSTR con )
{
  if ( !con.empty() ) {
    if ( !findCatalog( con, false ) ) {
      static const string xml_typ = "Gaudi::XMLFileCatalog";
      auto id0                    = con.find( "_" );
      string typ                  = con.substr( 0, id0 );
      string url                  = con.substr( id0 + 1 );
      IInterface* cat             = nullptr;
      if ( strncasecmp( "xml", typ.c_str(), 3 ) == 0 ) {
        cat = IFileCatalog::Factory::create( xml_typ, url, msgSvc().get() );
      } else {
        using Gaudi::PluginService::Details::Registry;
        Registry& registry = Registry::instance();
        if ( registry.getInfo( typ ).type == typeid( Service::Factory::FuncType ).name() ) {
          cat = Service::Factory::create( typ, url, serviceLocator().get() );
        } else if ( registry.getInfo( typ ).type == typeid( IFileCatalog::Factory::FuncType ).name() ) {
          cat = IFileCatalog::Factory::create( typ, url, msgSvc().get() );
        }
      }
      if ( cat ) {
        auto fileCat = SmartIF<IFileCatalog>( cat );
        if ( fileCat ) {
          addCatalog( fileCat.get() ); // addCatalog will take care of the refCount of fileCat...
          return;
        }
      }
      printError( "Failed to create catalog connection:" + con, true );
    }
    /// Catalog is already present ... silently ignore request
    return;
  }
  printError( "Got invalid (empty) catalog connection string.", true );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::addCatalog( IFileCatalog* cat )
{
  if ( cat ) {
    cat->addRef();
    m_catalogs.push_back( cat );
    return;
  }
  printError( "Got invalid catalog to be added to multi catalog.", true );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog( CSTR con )
{
  if ( con.empty() || con == "*" ) {
    for_each( m_catalogs, []( IFileCatalog* i ) { i->release(); } );
    m_catalogs.clear();
    return;
  }
  removeCatalog( findCatalog( con, false ) );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog( const IFileCatalog* cat )
{
  if ( cat ) {
    auto i = find( m_catalogs.begin(), m_catalogs.end(), cat );
    if ( i != m_catalogs.end() ) {
      ( *i )->release();
      m_catalogs.erase( i );
      return;
    }
    printError( "Unknown file catalog -- cannot be removed.", true );
  }
  printError( "Invalid file catalog.", true );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::setWriteCatalog( IFileCatalog* cat )
{
  if ( cat ) {
    if ( !cat->readOnly() ) {
      auto i = find( m_catalogs.begin(), m_catalogs.end(), cat );
      if ( i != m_catalogs.end() ) {
        m_catalogs.erase( i );
        m_catalogs.insert( m_catalogs.begin(), cat );
        return;
      }
      printError( "The catalog " + cat->connectInfo() + " is not known.", true );
    }
    printError( "The catalog " + cat->connectInfo() + " is not writable.", true );
  }
  printError( "Invalid file catalog.", true );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::setWriteCatalog( CSTR connect )
{
  auto i = i_findCatalog( connect, true );
  if ( i == m_catalogs.end() ) {
    addCatalog( connect );
    setWriteCatalog( findCatalog( connect, true ) );
    return;
  }
  setWriteCatalog( *i );
}
// ----------------------------------------------------------------------------
string MultiFileCatalog::getMetaDataItem( CSTR fid, CSTR attr ) const
{
  std::string result;
  for ( const auto& i : m_catalogs ) {
    result = i->getMetaDataItem( fid, attr );
    if ( !result.empty() ) break;
  }
  return result;
}
/// Create a FileID and DOM Node of the PFN with all the attributes
void MultiFileCatalog::registerPFN( CSTR fid, CSTR pfn, CSTR ftype ) const
{
  IFileCatalog* c = getCatalog( fid, false, true, false );
  if ( !c ) c     = getCatalog( "", true, true, true );
  c->registerPFN( fid, pfn, ftype );
}
/// Create a FileID and DOM Node of the LFN with all the attributes
void MultiFileCatalog::registerLFN( CSTR fid, CSTR lfn ) const
{
  IFileCatalog* c = getCatalog( fid, false, true, false );
  if ( !c ) c     = getCatalog( "", true, true, true );
  c->registerLFN( fid, lfn );
}
// ----------------------------------------------------------------------------
bool MultiFileCatalog::readOnly() const
{
  return std::all_of( std::begin( m_catalogs ), std::end( m_catalogs ),
                      []( const IFileCatalog* i ) { return i->readOnly(); } );
}
// ----------------------------------------------------------------------------
bool MultiFileCatalog::dirty() const
{
  return std::any_of( std::begin( m_catalogs ), std::end( m_catalogs ),
                      []( const IFileCatalog* i ) { return i->dirty(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::init()
{
  for_each( m_catalogs, []( IFileCatalog* i ) { i->init(); } );
  m_started = true;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::commit()
{
  for_each( m_catalogs, []( IFileCatalog* i ) { i->commit(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::rollback()
{
  for_each( m_catalogs, []( IFileCatalog* i ) { i->rollback(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::propHandler( Details::PropertyBase& /* p */ )
{
  // not yet initialized
  if ( !m_started ) {
    m_oldNames = m_catalogNames;
    return;
  } // RETURN
  // no real change - no action
  if ( m_catalogNames == m_oldNames ) {
    return;
  }
  m_oldNames = m_catalogNames;
  // remove ALL catalogs
  removeCatalog( "" );
  // add new catalogs
  for ( const auto& inew : m_catalogNames ) addCatalog( inew );
  // start
  init();
  //
  debug() << "New catalogs to be used: " << Gaudi::Utils::toString( m_catalogNames ) << endmsg;
}

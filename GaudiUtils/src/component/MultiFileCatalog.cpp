/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "createGuidAsString.h"

#include <Gaudi/PluginService.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>

#include <GaudiUtils/IFileCatalog.h>
#include <GaudiUtils/IFileCatalogMgr.h>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>

/*
 *   Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class MultiFileCatalog
   *
   *  This class constitutes the core of the
   *  XML based FileCatalog API within Gaudi.
   *  This class manages multiple file catalogs.
   *
   */
  class MultiFileCatalog : public extends<Service, IFileCatalog, IFileCatalogMgr> {
  public:
    // disambiguate between Service::Factory and IFileCatalog::Factory
    using Factory = Service::Factory;

  protected:
    using CSTR         = const std::string&;
    using Catalogs     = std::vector<IFileCatalog*>;
    using CatalogNames = std::vector<std::string>;

    template <class A1, class F>
    std::string _find( A1& arg1, F pmf ) const {
      std::string result;
      for ( const auto& i : m_catalogs ) {
        result = ( i->*pmf )( arg1 );
        if ( !result.empty() ) break;
      }
      return result;
    }
    template <class A1, class F>
    void _collect( A1& arg1, F f ) const {
      A1 tmp;
      for ( const auto& i : m_catalogs ) {
        f( i, tmp ); // tmp = f(i)
        arg1.insert( arg1.end(), tmp.begin(), tmp.end() );
        tmp.clear();
      }
    }
    template <class A2, class A1, class F>
    void _collect( const A2& arg2, A1& arg1, F f ) const {
      A1 tmp;
      for ( const auto& i : m_catalogs ) {
        f( i, arg2, tmp ); // tmp = f(i,arg2)
        arg1.insert( arg1.end(), tmp.begin(), tmp.end() );
        tmp.clear();
      }
    }

  public:
    /// Create a catalog file, initialization of XercesC.
    using extends::extends;

    /** IService implementation                                               */
    /// Finalize service object
    StatusCode initialize() override;
    /// Finalize service object
    StatusCode finalize() override;

    /** Catalog interface                                                     */
    /// Create file identifier using UUID mechanism
    std::string createFID() const override;
    /// Access to connect string
    CSTR connectInfo() const override;
    /// Parse the DOM tree of the XML catalog
    void init() override;
    /// Save DOM catalog to file
    void commit() override;
    /// Save DOM catalog to file
    void rollback() override;
    /// Check if the catalog is read-only
    bool readOnly() const override;
    /// Check if the catalog should be updated
    bool dirty() const override;
    /// Return the status of physical file name
    bool existsPFN( CSTR pfn ) const override { return !lookupPFN( pfn ).empty(); }
    /// Lookup file identifier by physical file name
    std::string lookupPFN( CSTR pfn ) const override { return _find( pfn, &IFileCatalog::lookupPFN ); }
    /// Return the status of a logical file name
    bool existsLFN( CSTR lfn ) const override { return !lookupLFN( lfn ).empty(); }
    /// Lookup file identifier by logical file name
    std::string lookupLFN( CSTR lfn ) const override { return _find( lfn, &IFileCatalog::lookupLFN ); }
    /// Return the status of a FileID
    bool existsFID( CSTR fid ) const override { return 0 != getCatalog( fid, false, false, false ); }
    /// Dump all physical file names of the catalog and their attributes associate to the FileID
    void getPFN( CSTR fid, Files& files ) const override {
      _collect( fid, files, std::mem_fn( &IFileCatalog::getPFN ) );
    }
    /// Dump all logical file names of the catalog associate to the FileID
    void getLFN( CSTR fid, Files& files ) const override {
      _collect( fid, files, std::mem_fn( &IFileCatalog::getLFN ) );
    }

    /// Dump all file Identifiers
    void getFID( std::vector<std::string>& fids ) const override {
      _collect( fids, std::mem_fn( &IFileCatalog::getFID ) );
    }
    /// Delete FileID from the catalog
    void deleteFID( CSTR fid ) const override { writeCatalog( fid )->deleteFID( fid ); }
    /// Create a FileID and DOM Node of the PFN with all the attributes
    void registerPFN( CSTR fid, CSTR pfn, CSTR ftype ) const override;
    /// Create a FileID and DOM Node of the LFN with all the attributes
    void registerLFN( CSTR fid, CSTR lfn ) const override;
    /// Create a FileID and DOM Node
    void registerFID( CSTR fid ) const override { writeCatalog()->registerFID( fid ); }
    /// rename a PFN
    void renamePFN( CSTR pfn, CSTR newpfn ) const override { writeCatalog()->renamePFN( pfn, newpfn ); }
    /// remove a PFN
    void deletePFN( CSTR pfn ) const override { writeCatalog()->deletePFN( pfn ); }
    /// Dump all MetaData of the catalog for a given file ID
    void getMetaData( CSTR fid, Attributes& attr ) const override {
      _collect( fid, attr, std::mem_fn( &IFileCatalog::getMetaData ) );
    }
    /// Access metadata item
    std::string getMetaDataItem( CSTR fid, CSTR name ) const override;
    /// Insert/update metadata item
    void setMetaData( CSTR fid, CSTR attr, CSTR val ) const override {
      writeCatalog( fid )->setMetaData( fid, attr, val );
    }
    /// Drop all metadata of one FID
    void dropMetaData( CSTR fid ) const override { writeCatalog( fid )->dropMetaData( fid ); }
    /// Drop specified metadata item
    void dropMetaData( CSTR fid, CSTR attr ) const override { writeCatalog( fid )->dropMetaData( fid, attr ); }

    /** Catalog management                                                    */
    /// Find catalog by connect string
    IFileCatalog* findCatalog( CSTR connect, bool must_be_writable ) const override;
    /// Add new catalog identified by name to the existing ones
    void addCatalog( CSTR connect ) override;
    /// Add new catalog identified by reference to the existing ones
    void addCatalog( IFileCatalog* cat ) override;
    /// Remove catalog identified by name from the existing ones
    void removeCatalog( CSTR connect ) override;
    /// Remove catalog identified by reference from the existing ones
    void removeCatalog( const IFileCatalog* cat ) override;
    /// Access catalog container
    Catalogs& catalogs() override { return m_catalogs; }
    /// Access catalog container (CONST)
    const Catalogs& catalogs() const override { return m_catalogs; }
    /// Access to the (first) writable file catalog
    IFileCatalog* writeCatalog( CSTR fid = "" ) const override { return getCatalog( fid, true, true, false ); }
    /// Define the writable catalog identified by reference
    void setWriteCatalog( IFileCatalog* cat ) override;
    /// Define the writable catalog identified by name
    void setWriteCatalog( CSTR connect ) override;

  private:
    /// Find catalog containing a given file identifier
    IFileCatalog* getCatalog( CSTR fid, bool throw_if_not, bool writable = true, bool prt = true ) const;
    /// Find catalog by connect string
    Catalogs::iterator i_findCatalog( CSTR connect, bool must_be_writable );

    /// simple property handle to allow interactive modification of
    /// list of the file catalogs
    void propHandler();

    void        printError( CSTR msg, bool throw_exc = true ) const;
    std::string lookupFID( CSTR lfn ) const;

    /// Container with references to known catalogs
    Catalogs m_catalogs;

    Gaudi::Property<CatalogNames> m_catalogNames{
        this, "Catalogs", { { "xmlcatalog_file:test_catalog.xml" } }, &MultiFileCatalog::propHandler, "catalog names" };

    /// Flag to indicate if catalog is started
    bool m_started = false;
    /// BACKUP:: Container with catalog names
    CatalogNames m_oldNames;
  };
} /* End namespace Gaudi                 */

using namespace Gaudi;

DECLARE_COMPONENT( MultiFileCatalog )

// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::initialize() {
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
  } catch ( const std::exception& /* e */ ) { printError( "Cannot add file catalog:" + current, false ); }
  return StatusCode::FAILURE;
}
// ----------------------------------------------------------------------------
StatusCode MultiFileCatalog::finalize() {
  commit();
  std::ranges::for_each( m_catalogs, []( IFileCatalog* i ) { i->release(); } );
  m_catalogs.clear();
  m_started = false;
  return Service::finalize();
}
// ----------------------------------------------------------------------------
/// Create file identifier using UUID mechanism
std::string MultiFileCatalog::createFID() const { return createGuidAsString(); }
// ----------------------------------------------------------------------------
MultiFileCatalog::CSTR MultiFileCatalog::connectInfo() const {
  static const std::string s( "MultiCatalog" );
  return s;
}
// ----------------------------------------------------------------------------
IFileCatalog* MultiFileCatalog::getCatalog( CSTR fid, bool throw_if_not, bool writable, bool prt ) const {
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
IFileCatalog* MultiFileCatalog::findCatalog( CSTR connect, bool must_be_writable ) const {
  auto i = std::find_if( m_catalogs.begin(), m_catalogs.end(),
                         [&]( const IFileCatalog* f ) { return connect == f->connectInfo(); } );
  return ( i != m_catalogs.end() && ( !must_be_writable || !( *i )->readOnly() ) ) ? *i : nullptr;
}
// ----------------------------------------------------------------------------
MultiFileCatalog::Catalogs::iterator MultiFileCatalog::i_findCatalog( CSTR connect, bool must_be_writable ) {
  auto i = std::find_if( m_catalogs.begin(), m_catalogs.end(),
                         [&]( const IFileCatalog* f ) { return connect == f->connectInfo(); } );
  if ( i != m_catalogs.end() && must_be_writable && ( *i )->readOnly() ) { i = m_catalogs.end(); }
  return i;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::printError( CSTR msg, bool rethrow ) const {
  if ( rethrow ) {
    fatal() << msg << endmsg;
    throw std::runtime_error( "Catalog> " + msg );
  }
  error() << msg << endmsg;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::addCatalog( CSTR con ) {
  if ( !con.empty() ) {
    if ( !findCatalog( con, false ) ) {
      static const std::string xml_typ = "Gaudi::XMLFileCatalog";
      auto                     id0     = con.find( "_" );
      std::string              typ     = con.substr( 0, id0 );
      std::string              url     = con.substr( id0 + 1 );
      IInterface*              cat     = nullptr;
      if ( strncasecmp( "xml", typ.c_str(), 3 ) == 0 ) {
        cat = IFileCatalog::Factory::create( xml_typ, url, msgSvc().get() ).release();
      } else {
        using Gaudi::PluginService::Details::Registry;
        Registry& registry = Registry::instance();
        if ( registry.getInfo( typ ).factory.type() == typeid( Service::Factory::FactoryType ) ) {
          cat = Service::Factory::create( typ, url, serviceLocator().get() ).release();
        } else if ( registry.getInfo( typ ).factory.type() == typeid( IFileCatalog::Factory::FactoryType ) ) {
          cat = IFileCatalog::Factory::create( typ, url, msgSvc().get() ).release();
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
void MultiFileCatalog::addCatalog( IFileCatalog* cat ) {
  if ( cat ) {
    cat->addRef();
    m_catalogs.push_back( cat );
    return;
  }
  printError( "Got invalid catalog to be added to multi catalog.", true );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog( CSTR con ) {
  if ( con.empty() || con == "*" ) {
    std::ranges::for_each( m_catalogs, []( IFileCatalog* i ) { i->release(); } );
    m_catalogs.clear();
    return;
  }
  removeCatalog( findCatalog( con, false ) );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::removeCatalog( const IFileCatalog* cat ) {
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
void MultiFileCatalog::setWriteCatalog( IFileCatalog* cat ) {
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
void MultiFileCatalog::setWriteCatalog( CSTR connect ) {
  auto i = i_findCatalog( connect, true );
  if ( i == m_catalogs.end() ) {
    addCatalog( connect );
    setWriteCatalog( findCatalog( connect, true ) );
    return;
  }
  setWriteCatalog( *i );
}
// ----------------------------------------------------------------------------
std::string MultiFileCatalog::getMetaDataItem( CSTR fid, CSTR attr ) const {
  std::string result;
  for ( const auto& i : m_catalogs ) {
    result = i->getMetaDataItem( fid, attr );
    if ( !result.empty() ) break;
  }
  return result;
}
/// Create a FileID and DOM Node of the PFN with all the attributes
void MultiFileCatalog::registerPFN( CSTR fid, CSTR pfn, CSTR ftype ) const {
  IFileCatalog* c = getCatalog( fid, false, true, false );
  if ( !c ) c = getCatalog( "", true, true, true );
  c->registerPFN( fid, pfn, ftype );
}
/// Create a FileID and DOM Node of the LFN with all the attributes
void MultiFileCatalog::registerLFN( CSTR fid, CSTR lfn ) const {
  IFileCatalog* c = getCatalog( fid, false, true, false );
  if ( !c ) c = getCatalog( "", true, true, true );
  c->registerLFN( fid, lfn );
}
// ----------------------------------------------------------------------------
bool MultiFileCatalog::readOnly() const {
  return std::all_of( std::begin( m_catalogs ), std::end( m_catalogs ),
                      []( const IFileCatalog* i ) { return i->readOnly(); } );
}
// ----------------------------------------------------------------------------
bool MultiFileCatalog::dirty() const {
  return std::any_of( std::begin( m_catalogs ), std::end( m_catalogs ),
                      []( const IFileCatalog* i ) { return i->dirty(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::init() {
  std::ranges::for_each( m_catalogs, []( IFileCatalog* i ) { i->init(); } );
  m_started = true;
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::commit() {
  std::ranges::for_each( m_catalogs, []( IFileCatalog* i ) { i->commit(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::rollback() {
  std::ranges::for_each( m_catalogs, []( IFileCatalog* i ) { i->rollback(); } );
}
// ----------------------------------------------------------------------------
void MultiFileCatalog::propHandler() {
  // not yet initialized
  if ( !m_started ) {
    m_oldNames = m_catalogNames;
    return;
  } // RETURN
  // no real change - no action
  if ( m_catalogNames == m_oldNames ) { return; }
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

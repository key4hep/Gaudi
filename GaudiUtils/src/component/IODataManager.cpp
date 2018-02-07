// Framework include files
#include "IODataManager.h"
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/Debugger.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiUtils/IFileCatalog.h"

#include <set>

namespace
{

  constexpr struct select2nd_t {
    template <typename S, typename T>
    const T& operator()( const std::pair<S, T>& p ) const
    {
      return p.second;
    }
  } select2nd{};

  template <typename InputIterator, typename OutputIterator, typename UnaryOperation, typename UnaryPredicate>
  OutputIterator transform_copy_if( InputIterator first, InputIterator last, OutputIterator result, UnaryOperation op,
                                    UnaryPredicate pred )
  {
    while ( first != last ) {
      auto val                     = op( *first );
      if ( pred( val ) ) *result++ = std::move( val );
      ++first;
    }
    return result;
  }
}

using namespace Gaudi;

DECLARE_COMPONENT( IODataManager )

static std::set<std::string> s_badFiles;

/// IService implementation: Db event selector override
StatusCode IODataManager::initialize()
{
  // Initialize base class
  StatusCode status = Service::initialize();
  MsgStream log( msgSvc(), name() );
  if ( !status.isSuccess() ) {
    log << MSG::ERROR << "Error initializing base class Service!" << endmsg;
    return status;
  }
  // Retrieve conversion service handling event iteration
  m_catalog = serviceLocator()->service( m_catalogSvcName );
  if ( !m_catalog ) {
    log << MSG::ERROR << "Unable to localize interface IFileCatalog from service:" << m_catalogSvcName << endmsg;
    return StatusCode::FAILURE;
  }
  m_incSvc = serviceLocator()->service( "IncidentSvc" );
  if ( !m_incSvc ) {
    log << MSG::ERROR << "Error initializing IncidentSvc Service!" << endmsg;
    return status;
  }
  return status;
}

/// IService implementation: finalize the service
StatusCode IODataManager::finalize()
{
  m_catalog = nullptr; // release
  return Service::finalize();
}

/// Small routine to issue exceptions
StatusCode IODataManager::error( CSTR msg, bool rethrow )
{
  MsgStream log( msgSvc(), name() );
  log << MSG::ERROR << "Error: " << msg << endmsg;
  if ( rethrow ) System::breakExecution();
  return StatusCode::FAILURE;
}

/// Get connection by owner instance (0=ALL)
IODataManager::Connections IODataManager::connections( const IInterface* owner ) const
{
  Connections conns;
  transform_copy_if( std::begin( m_connectionMap ), std::end( m_connectionMap ), std::back_inserter( conns ),
                     []( ConnectionMap::const_reference i ) { return i.second->connection; },
                     [&]( const IDataConnection* c ) { return !owner || c->owner() == owner; } );
  return conns;
}

/// Connect data file for writing
StatusCode IODataManager::connectRead( bool keep_open, Connection* con )
{
  if ( !establishConnection( con ) ) {
    return connectDataIO( UNKNOWN, Connection::READ, con->name(), "UNKNOWN", keep_open, con );
  }
  std::string dsn = con ? con->name() : std::string( "Unknown" );
  return error( "Failed to connect to data:" + dsn, false );
}

/// Connect data file for reading
StatusCode IODataManager::connectWrite( Connection* con, IoType mode, CSTR doctype )
{
  if ( !establishConnection( con ) ) {
    return connectDataIO( UNKNOWN, mode, con->name(), doctype, true, con );
  }
  std::string dsn = con ? con->name() : std::string( "Unknown" );
  return error( "Failed to connect to data:" + dsn, false );
}

/// Read raw byte buffer from input stream
StatusCode IODataManager::read( Connection* con, void* const data, size_t len )
{
  return establishConnection( con ).isSuccess() ? con->read( data, len ) : StatusCode::FAILURE;
}

/// Write raw byte buffer to output stream
StatusCode IODataManager::write( Connection* con, const void* data, int len )
{
  return establishConnection( con ).isSuccess() ? con->write( data, len ) : StatusCode::FAILURE;
}

/// Seek on the file described by ioDesc. Arguments as in ::seek()
long long int IODataManager::seek( Connection* con, long long int where, int origin )
{
  return establishConnection( con ).isSuccess() ? con->seek( where, origin ) : -1;
}

StatusCode IODataManager::disconnect( Connection* con )
{
  if ( con ) {
    std::string dataset = con->name();
    std::string dsn     = dataset;
    StatusCode sc       = con->disconnect();
    if (::strncasecmp( dsn.c_str(), "FID:", 4 ) == 0 )
      dsn = dataset.substr( 4 );
    else if (::strncasecmp( dsn.c_str(), "LFN:", 4 ) == 0 )
      dsn = dataset.substr( 4 );
    else if (::strncasecmp( dsn.c_str(), "PFN:", 4 ) == 0 )
      dsn = dataset.substr( 4 );

    auto j = m_fidMap.find( dataset );
    if ( j != m_fidMap.end() ) {
      std::string fid       = j->second;
      std::string gfal_name = "gfal:guid:" + fid;
      auto i                = m_connectionMap.find( fid );
      m_fidMap.erase( j );
      if ( ( j = m_fidMap.find( fid ) ) != m_fidMap.end() ) m_fidMap.erase( j );
      if ( ( j = m_fidMap.find( gfal_name ) ) != m_fidMap.end() ) m_fidMap.erase( j );
      if ( i != m_connectionMap.end() && i->second ) {
        IDataConnection* c = i->second->connection;
        if ( ( j = m_fidMap.find( c->pfn() ) ) != m_fidMap.end() ) m_fidMap.erase( j );
        if ( c->isConnected() ) {
          MsgStream log( msgSvc(), name() );
          c->disconnect();
          log << MSG::INFO << "Disconnect from dataset " << dsn << " [" << fid << "]" << endmsg;
        }
        delete i->second;
        m_connectionMap.erase( i );
      }
    }
    return sc;
  }
  return StatusCode::FAILURE;
}

StatusCode IODataManager::reconnect( Entry* e )
{
  StatusCode sc = StatusCode::FAILURE;
  if ( e && e->connection ) {
    switch ( e->ioType ) {
    case Connection::READ:
      sc = e->connection->connectRead();
      break;
    case Connection::UPDATE:
    case Connection::CREATE:
    case Connection::RECREATE:
      sc = e->connection->connectWrite( e->ioType );
      break;
    default:
      return StatusCode::FAILURE;
    }
    if ( sc.isSuccess() && e->ioType == Connection::READ ) {
      std::vector<Entry*> to_retire;
      e->connection->resetAge();
      transform_copy_if( std::begin( m_connectionMap ), std::end( m_connectionMap ), std::back_inserter( to_retire ),
                         select2nd, [&]( Entry* i ) {
                           IDataConnection* c = i->connection;
                           return e->connection != c && c->isConnected() && !i->keepOpen && c->ageFile() > m_ageLimit;
                         } );
      if ( !to_retire.empty() ) {
        MsgStream log( msgSvc(), name() );
        std::for_each( std::begin( to_retire ), std::end( to_retire ), [&]( Entry* j ) {
          IDataConnection* c = j->connection;
          c->disconnect();
          log << MSG::INFO << "Disconnect from dataset " << c->pfn() << " [" << c->fid() << "]" << endmsg;
        } );
      }
    }
  }
  return sc;
}

/// Retrieve known connection
IIODataManager::Connection* IODataManager::connection( CSTR dataset ) const
{
  auto j = m_fidMap.find( dataset );
  if ( j == m_fidMap.end() ) return nullptr;
  auto i = m_connectionMap.find( j->second );
  return ( i != m_connectionMap.end() ) ? i->second->connection : nullptr;
}

StatusCode IODataManager::establishConnection( Connection* con )
{
  if ( !con ) return error( "Severe logic bug: No connection object avalible.", true );

  if ( con->isConnected() ) {
    con->resetAge();
    return StatusCode::SUCCESS;
  }
  auto i = m_connectionMap.find( con->name() );
  if ( i != m_connectionMap.end() ) {
    Connection* c = i->second->connection;
    if ( c != con ) {
      m_incSvc->fireIncident( Incident( con->name(), IncidentType::FailInputFile ) );
      return error( "Severe logic bug: Twice identical connection object for DSN:" + con->name(), true );
    }
    if ( reconnect( i->second ).isSuccess() ) return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode IODataManager::connectDataIO( int typ, IoType rw, CSTR dataset, CSTR technology, bool keep_open,
                                         Connection* connection )
{
  MsgStream log( msgSvc(), name() );
  std::string dsn = dataset;
  try {
    StatusCode sc( StatusCode::SUCCESS, true );
    if (::strncasecmp( dsn.c_str(), "FID:", 4 ) == 0 )
      dsn = dataset.substr( 4 ), typ = FID;
    else if (::strncasecmp( dsn.c_str(), "LFN:", 4 ) == 0 )
      dsn = dataset.substr( 4 ), typ = LFN;
    else if (::strncasecmp( dsn.c_str(), "PFN:", 4 ) == 0 )
      dsn = dataset.substr( 4 ), typ = PFN;
    else if ( typ == UNKNOWN )
      return connectDataIO( PFN, rw, dsn, technology, keep_open, connection );

    if ( std::find( s_badFiles.begin(), s_badFiles.end(), dsn ) != s_badFiles.end() ) {
      m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
      return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
    }
    if ( typ == FID ) {
      auto fi = m_connectionMap.find( dsn );
      if ( fi == m_connectionMap.end() ) {
        IFileCatalog::Files files;
        m_catalog->getPFN( dsn, files );
        if ( files.empty() ) {
          if ( !m_useGFAL ) {
            if ( m_quarantine ) s_badFiles.insert( dsn );
            m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
            error( "connectDataIO> failed to resolve FID:" + dsn, false ).ignore();
            return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
          } else if ( dsn.length() == 36 && dsn[8] == '-' && dsn[13] == '-' ) {
            std::string gfal_name = "gfal:guid:" + dsn;
            m_fidMap[dsn] = m_fidMap[dataset] = m_fidMap[gfal_name] = dsn;
            sc = connectDataIO( PFN, rw, gfal_name, technology, keep_open, connection );
            if ( sc.isSuccess() ) return sc;
            if ( m_quarantine ) s_badFiles.insert( dsn );
          }
          if ( m_quarantine ) s_badFiles.insert( dsn );
          m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
          error( "connectDataIO> Failed to resolve FID:" + dsn, false ).ignore();
          return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
        }
        // keep track of the current return code before we start iterating over
        // replicas
        auto appmgr        = serviceLocator()->as<IProperty>();
        int origReturnCode = Gaudi::getAppReturnCode( appmgr );
        for ( auto i = files.cbegin(); i != files.cend(); ++i ) {
          std::string pfn = i->first;
          if ( i != files.cbegin() ) {
            log << MSG::WARNING << "Attempt to connect dsn:" << dsn << " with next entry in data federation:" << pfn
                << "." << endmsg;
          }
          sc = connectDataIO( PFN, rw, pfn, technology, keep_open, connection );
          if ( !sc.isSuccess() ) {
            if ( m_quarantine ) s_badFiles.insert( pfn );
            m_incSvc->fireIncident( Incident( pfn, IncidentType::FailInputFile ) );
          } else {
            m_fidMap[dsn] = m_fidMap[dataset] = m_fidMap[pfn] = dsn;
            // we found a working replica, let's reset the return code to the old value
            Gaudi::setAppReturnCode( appmgr, origReturnCode, true ).ignore();
            return sc;
          }
        }
        log << MSG::ERROR << "Failed to open dsn:" << dsn << " Federated file could not be resolved from "
            << files.size() << " entries." << endmsg;
        return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
      }
      return StatusCode::FAILURE;
    }
    std::string fid;
    auto j = m_fidMap.find( dsn );
    if ( j == m_fidMap.end() ) {
      IFileCatalog::Files files;
      switch ( typ ) {
      case LFN:
        fid = m_catalog->lookupLFN( dsn );
        if ( fid.empty() ) {
          m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
          log << MSG::ERROR << "Failed to resolve LFN:" << dsn << " Cannot access this dataset." << endmsg;
          return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
        }
        break;
      case PFN:
        fid = m_catalog->lookupPFN( dsn );
        if ( !fid.empty() ) m_catalog->getPFN( fid, files );
        if ( files.empty() ) {
          if ( rw == Connection::CREATE || rw == Connection::RECREATE ) {
            if ( fid.empty() ) fid = m_catalog->createFID();
            m_catalog->registerPFN( fid, dsn, technology );
            log << MSG::INFO << "Referring to dataset " << dsn << " by its file ID:" << fid << endmsg;
          } else {
            fid = dsn;
          }
        }
        break;
      }
    } else {
      fid = j->second;
    }
    if ( typ == PFN ) {
      // Open PFN
      auto fi = m_connectionMap.find( fid );
      if ( fi == m_connectionMap.end() ) {
        connection->setFID( fid );
        connection->setPFN( dsn );
        auto e = new Entry( technology, keep_open, rw, connection );
        // Here we open the file!
        if ( !reconnect( e ).isSuccess() ) {
          delete e;
          if ( m_quarantine ) s_badFiles.insert( dsn );
          m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
          error( "connectDataIO> Cannot connect to database: PFN=" + dsn + " FID=" + fid, false ).ignore();
          return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
        }
        fid               = connection->fid();
        m_fidMap[dataset] = m_fidMap[dsn] = m_fidMap[fid] = fid;
        if ( !( rw == Connection::CREATE || rw == Connection::RECREATE ) ) {
          if ( !m_disablePFNWarning && strcasecmp( dsn.c_str(), fid.c_str() ) == 0 ) {
            log << MSG::ERROR << "Referring to existing dataset " << dsn << " by its physical name." << endmsg;
            log << "You may not be able to navigate back to the input file"
                << " -- processing continues" << endmsg;
          }
        }
        m_connectionMap.emplace( fid, e ); // note: only if we disconnect does e get deleted??
        return StatusCode::SUCCESS;
      }
      // Here we open the file!
      if ( !reconnect( ( *fi ).second ).isSuccess() ) {
        if ( m_quarantine ) s_badFiles.insert( dsn );
        m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
        error( "connectDataIO> Cannot connect to database: PFN=" + dsn + " FID=" + fid, false ).ignore();
        return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
      }
      return StatusCode::SUCCESS;
    }
    sc = connectDataIO( FID, rw, fid, technology, keep_open, connection );
    if ( !sc.isSuccess() && m_quarantine ) {
      s_badFiles.insert( fid );
    } else if ( typ == LFN ) {
      m_fidMap[dataset] = fid;
    }
    return sc;
  } catch ( std::exception& e ) {
    error( std::string( "connectDataIO> Caught exception:" ) + e.what(), false ).ignore();
  } catch ( ... ) {
    error( std::string( "connectDataIO> Caught unknown exception" ), false ).ignore();
  }
  m_incSvc->fireIncident( Incident( dsn, IncidentType::FailInputFile ) );
  error( "connectDataIO> The dataset " + dsn + " cannot be opened.", false ).ignore();
  s_badFiles.insert( dsn );
  return StatusCode( IDataConnection::BAD_DATA_CONNECTION );
}

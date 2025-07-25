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
//  RootCnvSvc implementation
//--------------------------------------------------------------------
//
//  Description: Implementation of the ROOT data storage
//
//  Author     : M.Frank
//
//====================================================================

// Framework include files
#include "RootUtils.h"
#include <GaudiKernel/DataIncident.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/IUpdateable.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/KeyedContainer.h>
#include <GaudiKernel/LinkManager.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/System.h>
#include <RootCnv/RootAddress.h>
#include <RootCnv/RootCnvSvc.h>
#include <RootCnv/RootConverter.h>
#include <RootCnv/RootDataConnection.h>
#include <RootCnv/RootDatabaseCnv.h>
#include <RootCnv/RootDirectoryCnv.h>
#include <RootCnv/RootNTupleCnv.h>
#include <RootCnv/RootRefs.h>

using namespace std;
using namespace Gaudi;
typedef const string& CSTR;

#define S_OK StatusCode::SUCCESS
#define S_FAIL StatusCode::FAILURE
namespace GaudiRoot {
  bool patchStreamers( MsgStream& log );
}

namespace {
  static map<string, TClass*> s_classesNames;
  static map<CLID, TClass*>   s_classesClids;
} // namespace
#define MBYTE ( 1024 * 1024 )
#define kBYTE 1024
// Standard constructor
RootCnvSvc::RootCnvSvc( CSTR nam, ISvcLocator* svc )
    : ConversionSvc( nam, svc, ROOT_StorageType ), m_setup{ new RootConnectionSetup() } {
  m_setup->cacheBranches.push_back( "*" );
  declareProperty( "LoadSection", m_setup->loadSection = "Event" );

  // ROOT Read parameters: must be shared for the entire file!
  declareProperty( "CacheSize", m_setup->cacheSize = 10 * MBYTE );
  declareProperty( "LearnEntries", m_setup->learnEntries = 10 );
  declareProperty( "CacheBranches", m_setup->cacheBranches );
  declareProperty( "VetoBranches", m_setup->vetoBranches );

  declareProperty( m_setup->produceReproducibleFiles );
  m_setup->produceReproducibleFiles.setOwnerType<RootCnvSvc>();

  declareProperty( m_setup->root630ForwardCompatibility );
  m_setup->root630ForwardCompatibility.setOwnerType<RootCnvSvc>();
}

// Small routine to issue exceptions
StatusCode RootCnvSvc::error( CSTR msg ) {
  if ( m_log ) {
    log() << MSG::ERROR << "Error: " << msg << endmsg;
    return S_FAIL;
  }
  MsgStream m( msgSvc(), name() );
  m << MSG::ERROR << "Error: " << msg << endmsg;
  return S_FAIL;
}

// Initialize the Db data persistency service
StatusCode RootCnvSvc::initialize() {
  string     cname;
  StatusCode status = ConversionSvc::initialize();
  if ( !status.isSuccess() ) { return error( "Failed to initialize ConversionSvc base class." ); }
  m_log.reset( new MsgStream( msgSvc(), name() ) );
  if ( !m_compression.empty() ) {
    log() << MSG::INFO << "Setting global ROOT compression to:" << m_compression.value() << endmsg;
    if ( !( status = RootConnectionSetup::setCompression( m_compression.value() ) ).isSuccess() ) {
      return error( "Unable to interprete ROOT compression encoding:" + m_compression );
    }
  }
  m_ioMgr = service( "IODataManager" );
  if ( !m_ioMgr ) return error( "Unable to localize interface from service:IODataManager" );
  m_incidentSvc = service( "IncidentSvc" );
  if ( !m_incidentSvc ) return error( "Unable to localize interface from service:IncidentSvc" );
  m_setup->setMessageSvc( new MsgStream( msgSvc(), name() ) );
  m_setup->setIncidentSvc( m_incidentSvc.get() );
  GaudiRoot::patchStreamers( log() );
  cname     = System::typeinfoName( typeid( DataObject ) );
  m_classDO = TClass::GetClass( cname.c_str() );
  if ( !m_classDO ) return error( "Unable to load class description for DataObject" );
  cname       = System::typeinfoName( typeid( RootObjectRefs ) );
  m_classRefs = TClass::GetClass( cname.c_str() );
  if ( !m_classRefs ) return error( "Unable to load class description for ObjectRefs" );
  return S_OK;
}

// Finalize the Db data persistency service
StatusCode RootCnvSvc::finalize() {
  log() << MSG::INFO;
  if ( m_ioMgr ) {
    IIODataManager::Connections cons = m_ioMgr->connections( nullptr );
    for ( auto& i : cons ) {
      auto pc = dynamic_cast<RootDataConnection*>( i );
      if ( pc ) {
        if ( pc->owner() == this && !m_ioPerfStats.empty() ) { pc->saveStatistics( m_ioPerfStats.value() ); }
        if ( pc->lookupClient( this ) ) {
          size_t num_clients = pc->removeClient( this );
          if ( num_clients == 0 ) {
            if ( m_ioMgr->disconnect( pc ).isSuccess() ) {
              log() << "Disconnected data IO:" << pc->fid() << " [" << pc->pfn() << "]" << endmsg;
              delete pc;
            }
          }
        }
      }
    }
    m_ioMgr.reset();
  }
  m_log.reset();
  m_incidentSvc.reset();
  m_setup->setIncidentSvc( nullptr );
  return ConversionSvc::finalize();
}

// ConversionSvc overload: Create new Converter using factory
IConverter* RootCnvSvc::createConverter( long typ, const CLID& wanted, const ICnvFactory* ) {
  if ( wanted == CLID_StatisticsFile )
    return new RootDatabaseCnv( typ, wanted, serviceLocator().get(), this );
  else if ( wanted == CLID_StatisticsDirectory )
    return new RootDirectoryCnv( typ, wanted, serviceLocator().get(), this );
  else if ( wanted == CLID_RowWiseTuple )
    return new RootNTupleCnv( typ, wanted, serviceLocator().get(), this );
  else if ( wanted == CLID_ColumnWiseTuple )
    return new RootNTupleCnv( typ, wanted, serviceLocator().get(), this );
  else
    return new RootConverter( typ, wanted, serviceLocator().get(), this );
}

// ConversionSvc overload: Load the class (dictionary) for the converter
void RootCnvSvc::loadConverter( DataObject* pObject ) {
  if ( pObject ) {
    string cname = System::typeinfoName( typeid( *pObject ) );
    if ( log().level() <= MSG::DEBUG )
      log() << MSG::DEBUG << "Trying to 'Autoload' dictionary for class " << cname << endmsg;
    TClass* cl = s_classesNames[cname];
    if ( nullptr == cl ) {
      cl = TClass::GetClass( cname.c_str() );
      if ( cl ) {
        s_classesNames[cname]           = cl;
        s_classesClids[pObject->clID()] = cl;
      }
    }
  }
}

// Helper: Get TClass for a given DataObject pointer
TClass* RootCnvSvc::getClass( DataObject* pObject ) {
  auto i = s_classesClids.find( pObject->clID() );
  if ( i != s_classesClids.end() ) return i->second;
  loadConverter( pObject );
  i = s_classesClids.find( pObject->clID() );
  if ( i != s_classesClids.end() ) return i->second;

  string cname = System::typeinfoName( typeid( *pObject ) );
  throw runtime_error( "Unknown ROOT class for object:" + cname );
  return nullptr;
}

// Connect the output file to the service with open mode.
StatusCode RootCnvSvc::connectOutput( CSTR dsn, CSTR openMode ) {
  StatusCode sc = S_FAIL;
  m_current     = nullptr;
  m_currSection.clear();
  if ( ::strncasecmp( openMode.c_str(), "RECREATE", 3 ) == 0 )
    sc = connectDatabase( dsn, IDataConnection::RECREATE, &m_current );
  else if ( ::strncasecmp( openMode.c_str(), "NEW", 1 ) == 0 )
    sc = connectDatabase( dsn, IDataConnection::CREATE, &m_current );
  else if ( ::strncasecmp( openMode.c_str(), "CREATE", 1 ) == 0 )
    sc = connectDatabase( dsn, IDataConnection::CREATE, &m_current );
  else if ( ::strncasecmp( openMode.c_str(), "UPDATE", 1 ) == 0 )
    sc = connectDatabase( dsn, IDataConnection::UPDATE, &m_current );
  if ( sc.isSuccess() && m_current && m_current->isConnected() ) { return S_OK; }
  m_incidentSvc->fireIncident( Incident( dsn, IncidentType::FailOutputFile ) );
  log() << MSG::ERROR << "The dataset " << dsn << " cannot be opened in mode " << openMode << ". [Invalid mode]"
        << endmsg;
  return sc;
}

// Connect the output file to the service with open mode.
StatusCode RootCnvSvc::connectDatabase( CSTR dataset, int mode, RootDataConnection** con ) {
  try {
    IDataConnection* c             = m_ioMgr->connection( dataset );
    bool             fire_incident = false;
    *con                           = nullptr;
    if ( !c ) {
      auto       connection = std::make_unique<RootDataConnection>( this, dataset, m_setup );
      StatusCode sc         = ( mode != IDataConnection::READ )
                                  ? m_ioMgr->connectWrite( connection.get(), IDataConnection::IoType( mode ), "ROOT" )
                                  : m_ioMgr->connectRead( false, connection.get() );
      c                     = sc.isSuccess() ? m_ioMgr->connection( dataset ) : nullptr;
      if ( c ) {
        bool writable = 0 != ( mode & ( IDataConnection::UPDATE | IDataConnection::RECREATE ) );
        fire_incident = m_incidentEnabled && ( 0 != ( mode & ( IDataConnection::UPDATE | IDataConnection::READ ) ) );
        if ( writable ) {
          m_incidentSvc->fireIncident(
              ContextIncident<TFile*>( connection->pfn(), "CONNECTED_OUTPUT", connection->file() ) );
        }
        if ( 0 != ( mode & IDataConnection::READ ) ) {
          m_incidentSvc->fireIncident(
              ContextIncident<TFile*>( connection->pfn(), "CONNECTED_INPUT", connection->file() ) );
          if ( !m_ioPerfStats.empty() ) { connection->enableStatistics( m_setup->loadSection ); }
        }
        connection.release();
      } else {
        m_incidentSvc->fireIncident( Incident(
            dataset, mode == IDataConnection::READ ? IncidentType::FailInputFile : IncidentType::FailOutputFile ) );
        // An error message was already printed by the IODataManager. no need to do it here!
        return StatusCode::FAILURE;
      }
    }
    RootDataConnection* pc = dynamic_cast<RootDataConnection*>( c );
    if ( pc ) {
      if ( !pc->isConnected() )
        if ( auto sc = pc->connectRead(); !sc ) return sc;
      *con = pc;
      pc->resetAge();
      pc->addClient( this );
    }
    if ( *con ) {
      if ( fire_incident ) {
        IOpaqueAddress* pAddr   = nullptr;
        string          fid     = pc->fid();
        string          section = m_recordName[0] == '/' ? m_recordName.value().substr( 1 ) : m_recordName.value();
        TBranch*        b       = pc->getBranch( section, m_recordName.value() );
        if ( b ) {
          const string  par[2]  = { fid, m_recordName };
          unsigned long ipar[2] = { (unsigned long)( *con ), (unsigned long)b->GetEntries() - 1 };
          for ( int i = 0; i < b->GetEntries(); ++i ) {
            ipar[1] = i;
            if ( !pc->mergeFIDs().empty() ) fid = pc->mergeFIDs()[i];
            if ( !createAddress( repSvcType(), CLID_DataObject, par, ipar, pAddr ).isSuccess() ) {
              if ( log().level() <= MSG::VERBOSE )
                log() << MSG::VERBOSE << "Failed to create address for " << m_recordName << " in:" << fid << " ["
                      << pc->fid() << "][" << i << "]" << endmsg;
              continue;
            }
            if ( log().level() <= MSG::VERBOSE )
              log() << MSG::VERBOSE << "Prepare " << m_recordName << " " << fid << " [" << par[0] << "][" << i << "]"
                    << endmsg;
            m_incidentSvc->fireIncident( ContextIncident<IOpaqueAddress*>( fid, "FILE_OPEN_READ", pAddr ) );
          }
        } else {
          if ( log().level() <= MSG::VERBOSE )
            log() << MSG::VERBOSE << "No valid Records " << m_recordName << " present in:" << pc->fid() << endmsg;
        }
      }

      // We can remove retired connections, which are no longer used....
      for ( auto& i : m_ioMgr->connections( this ) ) {
        if ( i != *con && !i->isConnected() ) {
          RootDataConnection* pc = dynamic_cast<RootDataConnection*>( i );
          if ( pc && pc->lookupClient( this ) ) {
            size_t num_client = pc->removeClient( this );
            if ( num_client == 0 ) {
              if ( m_ioMgr->disconnect( pc ).isSuccess() ) {
                log() << MSG::INFO << "Removed disconnected IO  stream:" << pc->fid() << " [" << pc->pfn() << "]"
                      << endmsg;
                delete pc;
              }
            }
          }
        }
      }
      return S_OK;
    }
    m_incidentSvc->fireIncident( Incident( dataset, IncidentType::FailOutputFile ) );
    return S_FAIL;
  } catch ( exception& e ) {
    m_incidentSvc->fireIncident( Incident( dataset, IncidentType::FailOutputFile ) );
    return error( string( "connectDatabase> Caught exception:" ) + e.what() );
  } catch ( ... ) {
    m_incidentSvc->fireIncident( Incident( dataset, IncidentType::FailOutputFile ) );
    return error( "connectDatabase> Unknown Fatal Exception for " + dataset );
  }
}

// Conect output stream (valid until overwritten)
StatusCode RootCnvSvc::connectOutput( CSTR db_name ) { return connectOutput( db_name, "NEW" ); }

// Commit pending output on open container
StatusCode RootCnvSvc::commitOutput( CSTR dsn, bool /* doCommit */ ) {
  if ( m_current ) {
    size_t   len     = m_currSection.find( '/', 1 );
    string   section = m_currSection.substr( 1, len == string::npos ? string::npos : len - 1 );
    TBranch* b       = m_current->getBranch( section, m_currSection );
    if ( b ) {
      Long64_t   evt = b->GetEntries();
      TTree*     t   = b->GetTree();
      TObjArray* a   = t->GetListOfBranches();
      Int_t      nb  = a->GetEntriesFast();
      /// fill NULL pointers to all branches, which have less entries than the section branch
      for ( Int_t i = 0; i < nb; ++i ) {
        TBranch* br_ptr = (TBranch*)a->UncheckedAt( i );
        Long64_t br_evt = br_ptr->GetEntries();
        if ( br_evt < evt ) {
          Long64_t num = evt - br_evt;
          br_ptr->SetAddress( nullptr );
          while ( num > 0 ) {
            br_ptr->Fill();
            --num;
          }
          if ( log().level() <= MSG::DEBUG )
            log() << MSG::DEBUG << "commit: Added " << long( evt - br_evt ) << " Section: " << evt
                  << " Branch: " << br_ptr->GetEntries() << " RefNo: " << br_ptr->GetEntries() - 1
                  << " NULL entries to:" << br_ptr->GetName() << endmsg;
        }
      }

      b->GetTree()->SetEntries( evt );
      if ( log().level() <= MSG::DEBUG )
        log() << MSG::DEBUG << "Set section entries of " << m_currSection << " to " << long( evt ) << " entries."
              << endmsg;
    } else {
      return error( "commitOutput> Failed to update entry numbers on " + dsn );
    }
  }
  return S_OK;
}

// Disconnect from an existing data stream.
StatusCode RootCnvSvc::disconnect( CSTR dataset ) {
  IDataConnection* c = m_ioMgr->connection( dataset );
  return c ? m_ioMgr->disconnect( c ) : S_FAIL;
}

// IAddressCreator implementation: Address creation
StatusCode RootCnvSvc::createAddress( long typ, const CLID& clid, const string* par, const unsigned long* ip,
                                      IOpaqueAddress*& refpAddress ) {
  refpAddress = new RootAddress( typ, clid, par[0], par[1], ip[0], ip[1] );
  return S_OK;
}

// Insert null marker for not existent transient object
StatusCode RootCnvSvc::createNullRep( const std::string& path ) {
  size_t len     = path.find( '/', 1 );
  string section = path.substr( 1, len == string::npos ? string::npos : len - 1 );
  m_current->saveObj( section, path, nullptr, nullptr, m_minBufferSize, m_maxBufferSize, m_approxEventsPerBasket,
                      m_splitLevel );
  return S_OK;
}

// Insert null marker for not existent transient object
StatusCode RootCnvSvc::createNullRef( const std::string& path ) {
  RootObjectRefs*          refs    = nullptr;
  size_t                   len     = path.find( '/', 1 );
  string                   section = path.substr( 1, len == string::npos ? string::npos : len - 1 );
  pair<int, unsigned long> ret = m_current->save( section, path + "#R", nullptr, refs, m_minBufferSize, m_maxBufferSize,
                                                  m_approxEventsPerBasket, m_splitLevel );
  if ( log().level() <= MSG::VERBOSE )
    log() << MSG::VERBOSE << "Writing object:" << path << " " << ret.first << " " << hex << ret.second << dec
          << " [NULL]" << endmsg;
  return S_OK;
}

// Mark an object for write given an object reference
StatusCode RootCnvSvc::i__createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) {
  refpAddr = nullptr;
  if ( !pObj ) return error( "createRep> Current Database is invalid!" );
  CLID                     clid = pObj->clID();
  IRegistry*               pR   = pObj->registry();
  string                   p[2] = { m_current->fid(), pR->identifier() };
  TClass*                  cl   = ( clid == CLID_DataObject ) ? m_classDO : getClass( pObj );
  size_t                   len  = p[1].find( '/', 1 );
  string                   sect = p[1].substr( 1, len == string::npos ? string::npos : len - 1 );
  pair<int, unsigned long> ret  = m_current->saveObj( sect, p[1], cl, pObj, m_minBufferSize, m_maxBufferSize,
                                                      m_approxEventsPerBasket, m_splitLevel, true );
  if ( ret.first > 1 || ( clid == CLID_DataObject && ret.first == 1 ) ) {
    unsigned long ip[2] = { 0, ret.second };
    if ( m_currSection.empty() ) m_currSection = p[1];
    return createAddress( repSvcType(), clid, p, ip, refpAddr );
  }
  return error( "Failed to write object data for:" + p[1] );
}

// Save object references to data file
StatusCode RootCnvSvc::i__fillRepRefs( IOpaqueAddress* /* pA */, DataObject* pObj ) {
  if ( pObj ) {
    typedef vector<IRegistry*> Leaves;
    Leaves                     leaves;
    RootObjectRefs             refs;
    IRegistry*                 pR      = pObj->registry();
    auto                       dataMgr = SmartIF<IDataManagerSvc>{ pR->dataSvc() };
    if ( dataMgr ) {
      StatusCode status = dataMgr->objectLeaves( pObj, leaves );
      if ( status.isSuccess() ) {
        RootRef       ref;
        const string& id     = pR->identifier();
        size_t        len    = id.find( '/', 1 );
        string        sect   = id.substr( 1, len == string::npos ? string::npos : len - 1 );
        LinkManager*  pLinks = pObj->linkMgr();
        for ( auto& i : leaves ) {
          if ( i->address() ) {
            m_current->makeRef( *i, ref );
            ref.entry = i->address()->ipar()[1];
            refs.refs.push_back( ref );
          }
        }
        for ( int i = 0, n = pLinks->size(); i < n; ++i ) {
          const LinkManager::Link* lnk     = pLinks->link( i );
          int                      link_id = m_current->makeLink( lnk->path() );
          refs.links.push_back( link_id );
        }
        pair<int, unsigned long> ret = m_current->save( sect, id + "#R", m_classRefs, &refs, m_minBufferSize,
                                                        m_maxBufferSize, m_approxEventsPerBasket, m_splitLevel, true );
        if ( ret.first > 1 ) {
          if ( log().level() <= MSG::DEBUG )
            log() << MSG::DEBUG << "Writing object:" << id << " " << ret.first << " " << hex << ret.second << dec
                  << endmsg;
          return S_OK;
        }
      }
    }
  }
  return S_FAIL;
}

// Read existing object. Open transaction in read mode if not active
StatusCode RootCnvSvc::i__createObj( IOpaqueAddress* pA, DataObject*& refpObj ) {
  refpObj = nullptr;
  if ( !pA ) return S_FAIL;
  RootDataConnection* con  = nullptr;
  const string*       par  = pA->par();
  unsigned long*      ipar = const_cast<unsigned long*>( pA->ipar() );
  StatusCode          sc   = connectDatabase( par[0], IDataConnection::READ, &con );
  if ( sc.isSuccess() ) {
    ipar[0]             = (unsigned long)con;
    DataObject* pObj    = nullptr;
    size_t      len     = par[1].find( '/', 1 );
    string      section = par[1].substr( 1, len == string::npos ? string::npos : len - 1 );

    int nb = con->loadObj( section, par[1], ipar[1], pObj );
    if ( nb > 1 || ( nb == 1 && pObj->clID() == CLID_DataObject ) ) {
      refpObj = pObj;
      return S_OK;
    }
    delete pObj;
  }
  string tag = par[0] + ":" + par[1];

  const auto [itr, new_bad] = m_badFiles.insert( tag );
  if ( new_bad ) { return error( "createObj> Cannot access the object:" + tag ); }
  return S_FAIL;
}

// Resolve the references of the created transient object.
StatusCode RootCnvSvc::i__fillObjRefs( IOpaqueAddress* pA, DataObject* pObj ) {
  if ( !pA || !pObj ) return error( "read> Cannot read object -- no valid object address present " );

  const unsigned long* ipar = pA->ipar();
  RootDataConnection*  con  = (RootDataConnection*)ipar[0];
  if ( con ) {
    RootObjectRefs refs;
    const string*  par     = pA->par();
    size_t         len     = par[1].find( '/', 1 );
    string         section = par[1].substr( 1, len == string::npos ? string::npos : len - 1 );
    int            nb      = con->loadRefs( section, par[1], ipar[1], refs );
    if ( nb >= 1 ) {
      string        npar[3];
      unsigned long nipar[2];
      IRegistry*    pR      = pObj->registry();
      auto          dataMgr = SmartIF<IDataManagerSvc>{ pR->dataSvc() };
      LinkManager*  mgr     = pObj->linkMgr();
      for ( const auto& i : refs.links ) mgr->addLink( con->getLink( i ), nullptr );
      for ( auto& r : refs.refs ) {
        npar[0]             = con->getDb( r.dbase );
        npar[1]             = con->getCont( r.container );
        npar[2]             = con->getLink( r.link );
        nipar[0]            = 0;
        nipar[1]            = r.entry;
        IOpaqueAddress* nPA = nullptr;
        StatusCode      sc  = addressCreator()->createAddress( r.svc, r.clid, npar, nipar, nPA );
        if ( sc.isSuccess() ) {
          if ( log().level() <= MSG::VERBOSE )
            log() << MSG::VERBOSE << dataMgr.as<IService>()->name() << " -> Register:" << pA->registry()->identifier()
                  << "#" << npar[2] << "[" << r.entry << "]" << endmsg;
          sc = dataMgr->registerAddress( pA->registry(), npar[2], nPA );
          if ( sc.isSuccess() ) continue;
        }
        log() << MSG::ERROR << con->fid() << ": Failed to create address!!!!" << endmsg;
        return S_FAIL;
      }
      return pObj->update();
    } else if ( nb < 0 ) {
      string tag = par[0] + ":" + par[1];

      const auto [itr, new_bad] = m_badFiles.insert( tag );
      if ( new_bad ) { return error( "createObj> Cannot access the object:" + tag + " [Corrupted file]" ); }
    }
  }
  return S_FAIL;
}

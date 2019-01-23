#include "HistorySvc.h"

#include <functional>

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include "GaudiKernel/AlgToolHistory.h"
#include "GaudiKernel/AlgorithmHistory.h"
#include "GaudiKernel/DataHistory.h"
#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/ServiceHistory.h"

#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/System.h"
#include <Gaudi/Algorithm.h>

#include "GaudiKernel/IAlgContextSvc.h"

#include "boost/algorithm/string/predicate.hpp"
namespace ba = boost::algorithm;

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DECLARE_COMPONENT( HistorySvc )

using namespace std;

//
///////////////////////////////////////////////////////////////////////////
//

namespace
{
  template <typename MAP, typename SET>
  inline void map_to_set( const MAP& m, SET& s )
  {
    std::transform( std::begin( m ), std::end( m ), std::inserter( s, s.begin() ),
                    std::mem_fn( &MAP::value_type::second ) );
  }
}

struct DHH {
  CLID        id;
  std::string key;

  DHH( const CLID& i, std::string k ) : id( i ), key( std::move( k ) ) {}

  bool operator<( DHH const& rhs ) const
  {
    if ( id != rhs.id ) {
      return ( id < rhs.id );
    } else {
      return ( key < rhs.key );
    }
  }
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::reinitialize()
{

  clearState();
  m_state = Gaudi::StateMachine::OFFLINE;
  return initialize();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace
{
  constexpr struct clear_t {
    template <typename T1, typename T2>
    void operator()( std::pair<const T1* const, T2*>& p )
    {
      const_cast<T1*>( p.first )->release();
      delete p.second;
    }
  } clear_{};
  template <typename M>
  void clear( M& m )
  {
    std::for_each( std::begin( m ), std::end( m ), clear_ );
    m.clear();
  }
}
void HistorySvc::clearState()
{
  clear( m_algmap );

  m_ialgtools.clear();
  clear( m_algtoolmap );

  clear( m_svcmap );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::initialize()
{

  StatusCode status = Service::initialize();
  if ( status.isFailure() ) {
    ON_DEBUG
    debug() << "Failed to initialize the base class (Service)" << endmsg;
    return status;
  }

  ON_DEBUG
  debug() << "Initializing HistorySvc" << endmsg;

  if ( !m_activate ) return StatusCode::SUCCESS;

  static const bool CREATEIF( true );

  if ( service( "AlgContextSvc", p_algCtxSvc, CREATEIF ).isFailure() ) {
    error() << "unable to get the AlgContextSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( service( "IncidentSvc", m_incidentSvc, CREATEIF ).isFailure() ) {
    error() << "unable to get the IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // create a weak dependency on the ToolSvc, so that it doesn't get deleted
  // before we're done with it in finalize
  m_toolSvc = serviceLocator()->service( "ToolSvc" );
  if ( !m_toolSvc ) {
    error() << "could not retrieve the ToolSvc handle !" << endmsg;
    return StatusCode::FAILURE;
  }

  // add listener to be triggered by first BeginEvent with low priority
  // so it gets called first
  const bool rethrow = false;
  const bool oneShot = true; // make the listener called only once
  m_incidentSvc->addListener( this, IncidentType::BeginEvent, std::numeric_limits<long>::min(), rethrow, oneShot );

  m_outputFileTypeXML = ba::iends_with( m_outputFile.value(), ".xml" );
  ON_DEBUG if ( m_outputFileTypeXML ) { debug() << "output format is XML" << endmsg; }

  m_isInitialized = true;

  return StatusCode::SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::captureState()
{

  if ( !m_jobHistory ) {
    m_jobHistory.reset( new JobHistory );
    IJobOptionsSvc* jo;
    if ( service( "JobOptionsSvc", jo ).isFailure() ) {
      error() << "Could not get jobOptionsSvc - "
              << "not adding properties to JobHistory" << endmsg;
    } else {

      bool foundAppMgr( false );

      for ( auto& client : jo->getClients() ) {
        if ( client == "ApplicationMgr" ) foundAppMgr = true;
        for ( auto prop : *jo->getProperties( client ) ) {
          m_jobHistory->addProperty( client, prop );
        }
      }

      if ( !foundAppMgr ) {
        auto ap = service<IProperty>( "ApplicationMgr" );
        if ( !ap ) {
          error() << "could not get the ApplicationMgr" << endmsg;
        } else {
          for ( auto prop : ap->getProperties() ) {
            m_jobHistory->addProperty( "ApplicationMgr", prop );
          }
        }
      }
    }
  }

  /// Get all the Algorithms

  auto algMgr = Gaudi::svcLocator()->as<IAlgManager>();
  if ( !algMgr ) {
    error() << "Could not get AlgManager" << endmsg;
    return StatusCode::FAILURE;
  }

  size_t count = 0;
  for ( auto ialg : algMgr->getAlgorithms() ) {
    Gaudi::Algorithm* alg = dynamic_cast<Gaudi::Algorithm*>( ialg );
    if ( !alg ) {
      warning() << "Algorithm " << ialg->name() << " does not inherit from Gaudi::Algorithm. Not registering it."
                << endmsg;
    } else {
      ++count;
      registerAlg( *alg ).ignore();
    }
  }

  info() << "Registered " << count << " Algorithms" << endmsg;

  /// Get all the AlgTools

  m_isInitialized = true;
  for ( auto algtool : m_ialgtools ) {
    ( const_cast<IAlgTool*>( algtool ) )->addRef();
    registerAlgTool( *algtool ).ignore();
  }

  info() << "Registered " << m_algtoolmap.size() << " AlgTools" << endmsg;

  /// Get all the Services

  for ( auto svc : Gaudi::svcLocator()->getServices() ) {
    svc->addRef();
    registerSvc( *svc ).ignore();
  }

  info() << "Registered " << Gaudi::svcLocator()->getServices().size() << " Services" << endmsg;

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::stop()
{

  if ( !m_activate ) return StatusCode::SUCCESS;

  if ( m_dump ) {
    listProperties().ignore();
  }

  if ( !m_outputFile.empty() ) {
    std::ofstream ofs( m_outputFile );
    if ( !ofs ) {
      error() << "Unable to open output file \"m_outputFile\"" << endmsg;
    } else {
      //      dumpProperties(ofs);
      dumpState( ofs );
    }
  }

  clearState();

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::finalize()
{

  ON_VERBOSE
  verbose() << "HistorySvc::finalize()" << endmsg;

  clearState();

  StatusCode status = Service::finalize();

  if ( status.isSuccess() ) info() << "Service finalised successfully" << endmsg;

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::registerAlg( const Gaudi::Algorithm& alg )
{

  JobHistory* job = getJobHistory();
  if ( m_algmap.find( &alg ) != m_algmap.end() ) {
    warning() << "Algorithm " << alg.name() << " already registered with HistorySvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  ( const_cast<Gaudi::Algorithm*>( &alg ) )->addRef();

  m_algmap[&alg] = new AlgorithmHistory( alg, job );

  ON_DEBUG
  {
    auto& log = debug();
    log << "Registering algorithm: ";
    log.setColor( MSG::CYAN );
    log << alg.name() << endmsg;
    log.resetColor();
  }

  return StatusCode( StatusCode::SUCCESS, true );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::listProperties( const Gaudi::Algorithm& alg ) const
{

  info() << "Dumping properties for " << alg.name() << endl;

  AlgorithmHistory* hist = getAlgHistory( alg );

  if ( !hist ) {
    return StatusCode::FAILURE;
  }

  info() << alg.name() << " --> " << endl << *hist << endmsg;

  return StatusCode( StatusCode::SUCCESS, true );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpProperties( const Gaudi::Algorithm& alg, std::ofstream& ofs ) const
{

  AlgorithmHistory* hist = getAlgHistory( alg );

  if ( !hist ) {
    return;
  }

  PropertyList::const_iterator itr;
  for ( auto prop : hist->properties() ) {
    ofs << alg.name() << "  " << dumpProp( prop ) << std::endl;
  }
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AlgorithmHistory* HistorySvc::getAlgHistory( const Gaudi::Algorithm& alg ) const
{

  auto itr = m_algmap.find( &alg );
  if ( itr == m_algmap.end() ) {
    warning() << "Algorithm " << alg.name() << " not registered" << endmsg;
    return nullptr;
  }
  return itr->second;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::getAlgHistory( std::set<AlgorithmHistory*>& algs ) const { map_to_set( m_algmap, algs ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HistorySvc::registerJob() { return StatusCode( StatusCode::SUCCESS, true ); }
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::listProperties() const
{

  auto& log = info();

  log.setColor( MSG::CYAN );
  log << "Dumping properties for all Algorithms (" << m_algmap.size() << ")" << endmsg;

  for ( auto& alg : m_algmap ) {
    listProperties( *alg.first ).ignore();
  }

  log << MSG::INFO;
  log.setColor( MSG::CYAN );
  log << "Dumping properties for all AlgTools (" << m_algtoolmap.size() << ")" << endmsg;

  for ( auto& algtool : m_algtoolmap ) {
    ON_DEBUG
    debug() << " --> " << algtool.second->algtool_name() << endmsg;
    listProperties( *algtool.first ).ignore();
  }

  log << MSG::INFO;
  log.setColor( MSG::CYAN );
  log << "Dumping properties for all Services (" << m_svcmap.size() << ")" << endmsg;

  for ( auto& svc : m_svcmap ) {
    listProperties( *svc.first ).ignore();
  }

  log << MSG::INFO;
  log.setColor( MSG::CYAN );
  log << "Dumping properties for Job";
  log.resetColor();

  log << std::endl << *m_jobHistory << endmsg;

  return StatusCode( StatusCode::SUCCESS, true );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpProperties( std::ofstream& ofs ) const
{

  ofs << "GLOBAL" << std::endl;
  for ( const auto& prop : m_jobHistory->propertyPairs() ) {
    ofs << prop.first << "  " << dumpProp( prop.second ) << std::endl;
  }

  ofs << std::endl << "SERVICES" << std::endl;
  for ( const auto& s : m_svcmap ) dumpProperties( *s.first, ofs );

  ofs << std::endl << "ALGORITHMS" << std::endl;
  for ( const auto& alg : m_algmap ) dumpProperties( *alg.first, ofs );

  ofs << std::endl << "ALGTOOLS" << std::endl;
  for ( const auto& tool : m_algtoolmap ) dumpProperties( *tool.first, ofs );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

JobHistory* HistorySvc::getJobHistory() const { return m_jobHistory.get(); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
IAlgorithm* HistorySvc::getCurrentIAlg() const
{
  if ( p_algCtxSvc ) return p_algCtxSvc->currentAlg();
  warning() << "trying to create DataHistoryObj before "
            << "HistorySvc has been initialized" << endmsg;
  return nullptr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DataHistory* HistorySvc::createDataHistoryObj( const CLID& id, const std::string& key,
                                               const std::string& /* storeName */ )
{

  if ( !m_activate ) return nullptr;

  AlgorithmHistory* algHist = nullptr;

  IAlgorithm* ialg = getCurrentIAlg();
  if ( !ialg ) {
    ON_DEBUG
    debug() << "Could not discover current Algorithm:" << endl
            << "          object CLID: " << id << "  key: \"" << key << "\"" << endmsg;
    algHist = nullptr;
  } else {
    Gaudi::Algorithm* alg = dynamic_cast<Gaudi::Algorithm*>( ialg );
    if ( alg ) {
      algHist = getAlgHistory( *alg );
    } else {
      warning() << "Could not extract concrete Algorithm:" << endl
                << "          object CLID: " << id << "  key: \"" << key << "\"" << endmsg;
      algHist = nullptr;
    }
  }
  return new DataHistory( id, key, algHist );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HistorySvc::registerDataHistory( const CLID& id, const std::string& key, const std::string& storeName )
{

  DHH dhh( id, key );

  auto boundaries = m_datMap.equal_range( dhh );
  auto match      = boundaries.second;

  if ( boundaries.first != boundaries.second ) {
    // there is something in the map, let's look for the specific entry

    std::string algName;
    IAlgorithm* ialg = getCurrentIAlg();
    if ( ialg ) {
      algName = ialg->name();
    } else {
      algName = "UNKNOWN";
    }

    match = std::find_if( boundaries.first, boundaries.second,
                          [&algName]( decltype( boundaries )::first_type::reference p ) -> bool {
                            return p.second->algorithmHistory()->algorithm_name() == algName;
                          } );
  }

  if ( match == boundaries.second ) { // not found, crete the entry
    DataHistory* dh = createDataHistoryObj( id, key, storeName );
    m_datMap.insert( pair<DHH, DataHistory*>( dhh, dh ) );
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DataHistory* HistorySvc::getDataHistory( const CLID& id, const std::string& key,
                                         const std::string& /*storeName*/ ) const
{

  DHH dhh( id, key );

  auto mitr = m_datMap.equal_range( dhh );
  return ( mitr.first != mitr.second ) ? mitr.first->second : nullptr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int HistorySvc::getDataHistory( const CLID& id, const std::string& key, const std::string& /*storeName*/,
                                std::list<DataHistory*>& dhlist ) const
{

  DHH dhh( id, key );

  int n( 0 );

  auto mitr = m_datMap.equal_range( dhh );
  for ( auto itr = mitr.first; itr != mitr.second; ++itr ) {
    dhlist.push_back( itr->second );
    ++n;
  }
  return n;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::registerSvc( const IService& svc )
{

  if ( svc.name() == "HistoryStore" ) {
    return StatusCode( StatusCode::SUCCESS, true );
  }

  JobHistory*     job  = getJobHistory();
  const IService* psvc = &svc;
  auto            itr  = m_svcmap.find( psvc );
  if ( itr == m_svcmap.end() ) {

    ON_DEBUG
    {
      auto& log = debug();
      log << "Registering Service: ";
      log.setColor( MSG::CYAN );
      log << svc.name() << endmsg;
      log.resetColor();
    }

    m_svcmap[psvc] = new ServiceHistory( &svc, job );

    ( const_cast<IService*>( psvc ) )->addRef();
  }

  return StatusCode( StatusCode::SUCCESS, true );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory* HistorySvc::getServiceHistory( const IService& svc ) const
{

  const IService* psvc = &svc;
  auto            itr  = m_svcmap.find( psvc );
  if ( itr != m_svcmap.end() ) return itr->second;

  warning() << "Service " << svc.name() << " not registered" << endmsg;
  return nullptr;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::getServiceHistory( std::set<ServiceHistory*>& svcs ) const { map_to_set( m_svcmap, svcs ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::listProperties( const IService& svc ) const
{

  info() << "Dumping properties for " << svc.name() << endl;

  ServiceHistory* hist = getServiceHistory( svc );

  if ( !hist ) return StatusCode::FAILURE;

  info() << svc.name() << " --> " << endl << *hist << endmsg;

  return StatusCode( StatusCode::SUCCESS, true );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpProperties( const IService& svc, std::ofstream& ofs ) const
{

  ServiceHistory* hist = getServiceHistory( svc );

  if ( !hist ) return;

  for ( auto& prop : hist->properties() ) {
    ofs << svc.name() << "  " << dumpProp( prop ) << std::endl;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::registerAlgTool( const IAlgTool& ialg )
{

  if ( !m_isInitialized ) {
    if ( !p_algCtxSvc ) {
      if ( service( "AlgContextSvc", p_algCtxSvc, true ).isFailure() ) {
        error() << "unable to get the AlgContextSvc" << endmsg;
        return StatusCode::FAILURE;
      }
    }
    m_ialgtools.insert( &ialg );
    return StatusCode::SUCCESS;
  }

  const AlgTool* alg = dynamic_cast<const AlgTool*>( &ialg );
  if ( !alg ) {
    error() << "Could not dcast IAlgTool \"" << ialg.name() << "\" to an AlgTool" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_algtoolmap.find( alg ) != m_algtoolmap.end() ) {
    warning() << "AlgTool " << ialg.name() << " already registered in HistorySvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  const JobHistory* job = getJobHistory();
  m_algtoolmap[alg]     = new AlgToolHistory( *alg, job );

  ON_DEBUG
  {
    auto& log = debug();
    log << "Registering algtool: ";
    log.setColor( MSG::CYAN );
    log << alg->name() << endmsg;
    log.resetColor();
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::listProperties( const IAlgTool& alg ) const
{

  info() << "Dumping properties for " << alg.name() << endl;

  AlgToolHistory* hist = getAlgToolHistory( alg );

  if ( !hist ) return StatusCode::FAILURE;

  info() << alg.name() << " --> " << endl << *hist << endmsg;

  return StatusCode::SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpProperties( const IAlgTool& alg, std::ofstream& ofs ) const
{

  AlgToolHistory* hist = getAlgToolHistory( alg );

  if ( !hist ) return;

  for ( auto& prop : hist->properties() ) {
    ofs << alg.name() << "  " << dumpProp( prop ) << std::endl;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AlgToolHistory* HistorySvc::getAlgToolHistory( const IAlgTool& alg ) const
{

  const AlgTool* palg = dynamic_cast<const AlgTool*>( &alg );
  auto           itr  = m_algtoolmap.find( palg );
  if ( itr == m_algtoolmap.end() ) {
    warning() << "AlgTool " << alg.name() << " not registered" << endmsg;
    return nullptr;
  }
  return itr->second;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::getAlgToolHistory( std::set<AlgToolHistory*>& algs ) const { map_to_set( m_algtoolmap, algs ); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::handle( const Incident& incident )
{

  if ( incident.type() == IncidentType::BeginEvent ) {
    if ( captureState().isFailure() ) {
      warning() << "Error capturing state." << endl << "Will try again at next BeginEvent incident" << endmsg;
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string HistorySvc::dumpProp( const Gaudi::Details::PropertyBase* prop, bool isXML, int ind ) const
{
  std::ostringstream ost;
  if ( isXML ) {
    while ( ind > 0 ) {
      ost << " ";
      ind--;
    }
    ost << "<PROPERTY name=\"" << prop->name() << "\" value=\"" << HistoryObj::convert_string( prop->toString() )
        << "\" documentation=\"" << HistoryObj::convert_string( prop->documentation() ) << "\">";
  } else {
    prop->fillStream( ost );
  }
  return ost.str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpState( std::ofstream& ofs ) const
{

  if ( m_outputFileTypeXML ) {
    // xml header
    ofs << "<?xml version=\"1.0\" ?> " << std::endl;
    ofs << "<!--Test-xml-->" << std::endl;
    ofs << "<SETUP>" << std::endl;
    ofs << "  <GLOBAL>" << std::endl;
  } else {
    ofs << "GLOBAL" << std::endl;
  }

  std::string client_currently_open = "start";
  for ( auto& item : m_jobHistory->propertyPairs() ) {
    // client is the name of the component of the current property
    const std::string&                  client = item.first;
    const Gaudi::Details::PropertyBase* prp    = item.second;

    if ( m_outputFileTypeXML ) {

      if ( client != client_currently_open ) {
        if ( client_currently_open != "start" ) ofs << "    </COMPONENT>" << endl;
        ofs << "    <COMPONENT name=\"" << client << "\" class=\"undefined\">" << std::endl;
      }
    } else {
      ofs << client << "  ";
    }

    ofs << dumpProp( prp, m_outputFileTypeXML, 6 ) << endl;

    client_currently_open = client;

    if ( m_outputFileTypeXML ) ofs << "    </COMPONENT>" << endl;
  }

  if ( m_outputFileTypeXML ) {
    ofs << "</GLOBAL>" << endl << "<SERVICES>" << endl;
  } else {
    ofs << "SERVICES" << std::endl;
  }

  // helper to dump monitored components sorted by name
  auto sortedDump = [&ofs, this]( const auto& map ) {
    std::map<std::string, const INamedInterface*> sorted;
    for ( const auto& item : map ) sorted[item.first->name()] = item.first;
    for ( const auto& item : sorted ) dumpState( item.second, ofs );
  };

  sortedDump( m_svcmap );

  if ( m_outputFileTypeXML ) {
    ofs << "</SERVICES>" << endl << "<ALGORITHMS> " << endl;
  } else {
    ofs << "ALGORITHMS" << std::endl;
  }

  sortedDump( m_algmap );

  if ( m_outputFileTypeXML ) {
    ofs << "</ALGORITHMS>" << endl << "<ALGTOOLS> " << endl;
  } else {
    ofs << "ALGTOOLS" << std::endl;
  }

  sortedDump( m_algtoolmap );

  if ( m_outputFileTypeXML ) {
    ofs << "</ALGTOOLS>" << endl << "</SETUP>" << endl;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistorySvc::dumpState( const INamedInterface* in, std::ofstream& ofs ) const
{

  HistoryObj*      hist  = nullptr;
  IVersHistoryObj* vhist = nullptr;

  const IService*         is = nullptr;
  const Gaudi::Algorithm* ia = nullptr;
  const IAlgTool*         it = nullptr;
  if ( ( is = dynamic_cast<const IService*>( in ) ) != nullptr ) {
    ON_VERBOSE
    verbose() << in->name() << " is Service" << endmsg;
    ServiceHistory* o = getServiceHistory( *is );
    hist              = dynamic_cast<HistoryObj*>( o );
    vhist             = dynamic_cast<IVersHistoryObj*>( o );
  } else if ( ( ia = dynamic_cast<const Gaudi::Algorithm*>( in ) ) != nullptr ) {
    ON_VERBOSE
    verbose() << in->name() << " is Alg" << endmsg;
    AlgorithmHistory* o = getAlgHistory( *ia );
    hist                = dynamic_cast<HistoryObj*>( o );
    vhist               = dynamic_cast<IVersHistoryObj*>( o );
  } else if ( ( it = dynamic_cast<const IAlgTool*>( in ) ) != nullptr ) {
    ON_VERBOSE
    verbose() << in->name() << " is AlgTool" << endmsg;
    AlgToolHistory* o = getAlgToolHistory( *it );
    hist              = dynamic_cast<HistoryObj*>( o );
    vhist             = dynamic_cast<IVersHistoryObj*>( o );
  } else {
    error() << "Could not dcast interface to accepted History Obj type for " << in->name() << endmsg;
    return;
  }

  if ( !hist || !vhist ) {
    error() << "Could not dcast recognized object to HistoryObj or IVersHistoryObj. This should never happen."
            << endmsg;
    return;
  }

  if ( m_outputFileTypeXML ) {
    hist->dump( ofs, true );
  } else {
    ofs << ">> " << vhist->name() << endl << *hist << endl;
  }
}

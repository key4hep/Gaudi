#include "IncidentProcAlg.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/ThreadLocalContext.h"
DECLARE_COMPONENT( IncidentProcAlg )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

namespace {
  // ==========================================================================
  static const std::string s_unknown = "<unknown>";
  // Helper to get the name of the listener
  inline const std::string& getListenerName( IIncidentListener* lis ) {
    SmartIF<INamedInterface> iNamed( lis );
    return iNamed ? iNamed->name() : s_unknown;
  }
  // ==========================================================================
} // namespace

IncidentProcAlg::IncidentProcAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {}

StatusCode IncidentProcAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if ( sc.isFailure() ) return sc;
  m_incSvc = service( "IncidentSvc", true );
  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentProcAlg::execute() {
  const EventContext& context = Gaudi::Hive::currentContext();
  auto                incPack = m_incSvc->getIncidents( &context );
  MsgStream           log( msgSvc(), name() );
  log << MSG::DEBUG << " Number of Incidents to process = " << incPack.incidents.size()
      << " Context= " << Gaudi::Hive::currentContext() << endmsg;
  while ( incPack.incidents.size() ) {
    if ( incPack.incidents.size() != incPack.listeners.size() ) {
      log << MSG::WARNING << " Size of fired incidents and listeners do not match!" << endmsg;
    }
    for ( size_t t = 0; t < incPack.incidents.size(); t++ ) {
      auto& inc = incPack.incidents.at( t );
      auto& lis = incPack.listeners.at( t );
      for ( auto& l : lis ) {
        log << MSG::DEBUG << "Calling '" << getListenerName( l ) << "' for incident [" << inc->type() << "]" << endmsg;

        // handle exceptions if they occur
        try {
          l->handle( *inc );
        } catch ( const GaudiException& exc ) {
          error() << "Exception with tag=" << exc.tag()
                  << " is caught"
                     " handling incident"
                  << inc->type() << endmsg;
          error() << exc << endmsg;
        } catch ( const std::exception& exc ) {
          error() << "Standard std::exception is caught"
                     " handling incident"
                  << inc->type() << endmsg;
          error() << exc.what() << endmsg;
        } catch ( ... ) { error() << "UNKNOWN Exception is caught handling incident" << inc->type() << endmsg; }
        // check wheter one of the listeners is singleShot
      }
    }
    incPack = m_incSvc->getIncidents( &context );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentProcAlg::finalize() {
  info() << "Finalize" << endmsg;
  return Algorithm::finalize();
}

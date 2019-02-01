#include "IncidentListenerTestAlg.h"

#include "IncidentListenerTest.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

// Static Factory declaration
DECLARE_COMPONENT( IncidentListenerTestAlg )

std::string IncidentListenerTestAlg::s_incidentType = "TestIncident";

std::string& IncidentListenerTestAlg::incident() { return s_incidentType; }

//=============================================================================
StatusCode IncidentListenerTestAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  // get a the incident service
  m_incSvc = service( "IncidentSvc", true );

  // instantiate listeners
  m_listener[0].reset( new IncidentListenerTest( "Listener 0", serviceLocator() ) );
  m_listener[1].reset( new IncidentListenerTest( "Listener 1", serviceLocator(), 3 ) );
  m_listener[2].reset( new IncidentListenerTest( "Listener 2", serviceLocator() ) );
  m_listener[3].reset( new IncidentListenerTest( "Listener 3", serviceLocator() ) );
  m_listener[4].reset( new IncidentListenerTest( "EventBoundary", serviceLocator() ) );
  m_listener[5].reset( new IncidentListenerTest( "Generic Listener", serviceLocator() ) );

  info() << "Registering incident listeners" << endmsg;

  const bool rethrow    = false;
  const bool singleShot = true;
  const long priority   = -10;
  m_incSvc->addListener( m_listener[3].get(), incident(), priority, rethrow, singleShot );

  m_incSvc->addListener( m_listener[0].get(), incident() );
  m_incSvc->addListener( m_listener[1].get(), incident() );
  m_incSvc->addListener( m_listener[2].get(), incident() );

  m_incSvc->addListener( m_listener[4].get(), IncidentType::BeginEvent );
  m_incSvc->addListener( m_listener[4].get(), IncidentType::EndEvent );

  m_incSvc->addListener( m_listener[5].get() );

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentListenerTestAlg::execute() {
  info() << "Firing incident" << endmsg;
  m_incSvc->fireIncident( Incident( name(), incident() ) );
  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentListenerTestAlg::finalize() {
  info() << "Finalize" << endmsg;
  return GaudiAlgorithm::finalize();
}

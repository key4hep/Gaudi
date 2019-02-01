// Include files

// from Gaudi
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"

// local
#include "AbortEventAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AbortEventAlg
//
// Nov 16, 2007 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( AbortEventAlg )

//=============================================================================
// Initialization
//=============================================================================
StatusCode AbortEventAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_incidentSvc = service( "IncidentSvc", true );

  m_counter = 0;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AbortEventAlg::execute() {

  debug() << "==> Execute" << endmsg;

  if ( ++m_counter == m_count ) { m_incidentSvc->fireIncident( Incident( name(), IncidentType::AbortEvent ) ); }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AbortEventAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

//=============================================================================

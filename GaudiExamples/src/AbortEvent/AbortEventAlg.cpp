// $Id: AbortEventAlg.cpp,v 1.1 2007/11/16 18:34:56 marcocle Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/Incident.h" 
#include "GaudiKernel/IIncidentSvc.h" 

// local
#include "AbortEventAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AbortEventAlg
//
// Nov 16, 2007 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AbortEventAlg )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AbortEventAlg::AbortEventAlg( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
  m_counter(0)
{
  declareProperty("AbortedEventNumber", m_count = 3,
                  "At which event to trigger an abort");
}
//=============================================================================
// Destructor
//=============================================================================
AbortEventAlg::~AbortEventAlg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode AbortEventAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_incidentSvc = svc<IIncidentSvc>("IncidentSvc",true);

  m_counter = 0;
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AbortEventAlg::execute() {

  debug() << "==> Execute" << endmsg;
  
  if ( ++m_counter == m_count ){
    m_incidentSvc->fireIncident(Incident(name(),IncidentType::AbortEvent));
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AbortEventAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================

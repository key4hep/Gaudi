// Include files 
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include <iostream>
#include <climits>

// local
#include "JemallocProfileSvc.h"

// including jemmalloc.h is difficult as the malloc signature is not exactly identical
// to the system one (issue with throw).
// We therefore declare mallctl here.
extern "C" 
{
  int mallctl(const char *name, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
}

//-----------------------------------------------------------------------------
// Implementation file for class : JemallocProfileSvc
//
// 2016-01-11 : Ben Couturier
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
JemallocProfileSvc::JemallocProfileSvc(const std::string& name, ISvcLocator* svcLoc):
  base_class(name, svcLoc), m_eventNumber(0), m_profiling(false) {
  
  declareProperty("StartFromEventN", m_nStartFromEvent = 1,
                  "After what event we start profiling. "
                  );

  declareProperty("StopAtEventN", m_nStopAtEvent = 0,
                  "After what event we stop profiling. "
                  "If 0 than we also profile finalization stage. Default = 0."
                  );

  declareProperty("DumpPeriod", m_dumpPeriod = 100,
                  "Period for dumping head to a file. Default=100"
                  );

}

//=============================================================================
// Initializer
//=============================================================================
StatusCode JemallocProfileSvc::initialize() {
  StatusCode sc = base_class::initialize();
  if (sc.isFailure()) return sc;

  // register to the incident service
  static const std::string serviceName = "IncidentSvc";
  m_incidentSvc = serviceLocator()->service(serviceName);
  if ( ! m_incidentSvc ) {
    error() << "Cannot retrieve " << serviceName << endmsg;
    return StatusCode::FAILURE;
  }
  
  debug() << "Register to the IncidentSvc" << endmsg;
  m_incidentSvc->addListener(this, IncidentType::BeginEvent);
  m_incidentSvc->addListener(this, IncidentType::EndEvent);

  // Resetting the event counter
  m_eventNumber = 0;
  m_profiling = false;

  return StatusCode::SUCCESS;
}

// Finalization of the service.
StatusCode JemallocProfileSvc::finalize() {

  // unregistering from the IncidentSvc
  m_incidentSvc->removeListener(this, IncidentType::BeginEvent);
  m_incidentSvc->removeListener(this, IncidentType::EndEvent);
  m_incidentSvc.reset();
  return base_class::finalize();
}


// Handler for incidents
void JemallocProfileSvc::handle(const Incident& incident) 
{
  if (IncidentType::BeginEvent == incident.type())
  {
    handleBegin();
  } else if (IncidentType::EndEvent == incident.type()) 
  {
    handleEnd();
  }  
}


// Handler for incidents
// Called on at begin events
inline void JemallocProfileSvc::handleBegin() 
{ 
  m_eventNumber += 1;
  
  if (m_eventNumber == m_nStartFromEvent)
  {
    m_profiling = true;
    info() << "Starting Jemalloc profile at event "
           <<  m_eventNumber << endmsg;
    mallctl("prof.dump", NULL, NULL, NULL, 0);
  }
}

// Handler for incidents
// Called on at End events
inline void JemallocProfileSvc::handleEnd()
{
  if (m_profiling
      && m_eventNumber != m_nStartFromEvent
      && ((m_eventNumber - m_nStartFromEvent) % m_dumpPeriod == 0)) 
  {
    info() << "Jemalloc Dumping heap at event "
           <<  m_eventNumber << endmsg;
    mallctl("prof.dump", NULL, NULL, NULL, 0);
  }

  if (m_eventNumber ==  m_nStopAtEvent) 
  {
    m_profiling = false;
    info() << "Stopping Jemalloc profile at event " 
           <<  m_eventNumber << endmsg;
    mallctl("prof.dump", NULL, NULL, NULL, 0);
  } 
}

//=============================================================================
// Destructor

//=============================================================================
 JemallocProfileSvc::~JemallocProfileSvc() {}

//=============================================================================
// Declaration of the factory
DECLARE_COMPONENT(JemallocProfileSvc)

#ifndef JEMALLOC_JEMALLOCPROFILESVC_H 
#define JEMALLOC_JEMALLOCPROFILESVC_H 1

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"

/** @class JemallocProfileSvc JemallocProfileSvc.h jemalloc/JemallocProfileSvc.h
 *  
 * Service that enables the Jemalloc profiler on demand
 *
 *  @author Ben Couturier
 *  @date   2016-01-12
 */
class JemallocProfileSvc: public extends1<Service, IIncidentListener> {

public: 

/// Standard constructor
JemallocProfileSvc(const std::string& name, ISvcLocator* svcLoc); 

/// Initializer
StatusCode initialize();

/// Finalizer
StatusCode finalize();

// Handler for incident
void handle(const Incident& incident);

 ///< Destructor
virtual ~JemallocProfileSvc( );

protected:
void handleBegin();
void handleEnd();


private:

/// Start, End event and counter
int m_nStartFromEvent; // Event to start profiling at
int m_nStopAtEvent;  // Event to stop profiling at
int m_dumpPeriod;  // Period at which to dump the heap
int m_eventNumber;   // Current event number

/// Status of the profiling
bool m_profiling;

/// Pointer to the incident service.
SmartIF<IIncidentSvc> m_incidentSvc;

};
#endif // JEMALLOC_JEMALLOCPROFILESVC_H


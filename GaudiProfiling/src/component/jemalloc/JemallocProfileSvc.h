#ifndef JEMALLOC_JEMALLOCPROFILESVC_H
#define JEMALLOC_JEMALLOCPROFILESVC_H 1

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"

/** @class JemallocProfileSvc JemallocProfileSvc.h jemalloc/JemallocProfileSvc.h
 *
 * Service that enables the Jemalloc profiler on demand.
 * By default, the profiling is diabled, the user needs
 * to set incidents at which to start/stop the profiling,
 * or specify incidents for that purpose.
 *
 *  @author Ben Couturier
 *  @date   2016-01-12
 */
class JemallocProfileSvc : public extends<Service,
                                          IIncidentListener> {

public:
  /// Standard constructor
  JemallocProfileSvc(const std::string &name, ISvcLocator *svcLoc);

  /// Initializer
  StatusCode initialize() override;

  /// Finalizer
  StatusCode finalize() override;

  // Handler for incident
  void handle(const Incident &incident) override;

  ///< Destructor
  ~JemallocProfileSvc() override;

protected:
  void handleBegin();
  void handleEnd();
  void startProfiling();
  void stopProfiling();
  void dumpProfile();
  

private:
  /// Start, End event and counter
  int m_nStartFromEvent; // Event to start profiling at
  int m_nStopAtEvent;    // Event to stop profiling at
  int m_dumpPeriod;      // Period at which to dump the heap
  int m_eventNumber;     // Current event number
  std::vector<std::string> m_startFromIncidents; // Incidents to use as trigger
  std::vector<std::string> m_stopAtIncidents; // Incidents to use as trigger
  bool m_hasStartIncident;
  bool m_hasStopIncident;
  

  /// Status of the profiling
  bool m_profiling;

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;
};
#endif // JEMALLOC_JEMALLOCPROFILESVC_H

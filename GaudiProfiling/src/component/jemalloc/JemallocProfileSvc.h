/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef JEMALLOC_JEMALLOCPROFILESVC_H
#define JEMALLOC_JEMALLOCPROFILESVC_H 1

// Include files
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Service.h"

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
class JemallocProfileSvc : public extends<Service, IIncidentListener> {

public:
  /// Standard constructor
  using extends::extends;

  /// Initializer
  StatusCode initialize() override;

  /// Finalizer
  StatusCode finalize() override;

  // Handler for incident
  void handle( const Incident& incident ) override;

protected:
  void handleBegin();
  void handleEnd();
  void startProfiling();
  void stopProfiling();
  void dumpProfile();

private:
  Gaudi::Property<int> m_nStartFromEvent{ this, "StartFromEventN", 0, "After what event we start profiling." };
  Gaudi::Property<std::vector<std::string>> m_startFromIncidents{
      this, "StartFromIncidents", {}, "Incidents that trigger profiling start" };
  Gaudi::Property<int> m_nStopAtEvent{
      this, "StopAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0." };
  Gaudi::Property<std::vector<std::string>> m_stopAtIncidents{
      this, "StopAtIncidents", {}, "Incidents that trigger profiling start" };
  Gaudi::Property<int> m_dumpPeriod{ this, "DumpPeriod", 100, "Period for dumping head to a file. Default=100" };

  /// Current event number
  int m_eventNumber = 0;

  bool m_hasStartIncident = false;
  bool m_hasStopIncident  = false;

  /// Status of the profiling
  bool m_profiling = false;

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;
};
#endif // JEMALLOC_JEMALLOCPROFILESVC_H

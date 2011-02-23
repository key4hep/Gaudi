/*
 * StalledEventMonitor.h
 *
 *  Created on: Apr 19, 2010
 *      Author: Marco Clemencic
 */

#ifndef STALLEDEVENTMONITOR_H_
#define STALLEDEVENTMONITOR_H_

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include <memory>

// Forward declarations
class WatchdogThread;
class IIncidentSvc;

/** 
 *  Service that monitor the time taken by processing of single events using a
 *  separate thread.
 *
 *  @see WatchdogThread
 *
 *  @author Marco Clemencic
 *  @date   Apr 19, 2010
 */
class StalledEventMonitor: public extends1<Service, IIncidentListener> {
public:
  /// Constructor
  StalledEventMonitor(const std::string& name, ISvcLocator* svcLoc);

  /// Destructor
  virtual ~StalledEventMonitor();

  /// Initialization of the service.
  /// Prepare the watchdog thread and configures it.
  virtual StatusCode initialize();

  /// Start the watchdog thread (before entering the event loop).
  virtual StatusCode start();

  /// Notify the watchdog thread for a new event.
  virtual void handle(const Incident&  /* incident */);

  /// Stop the watchdog thread (after the event loop).
  virtual StatusCode stop();

  /// Finalization of the service.
  virtual StatusCode finalize();

private:
  /// Number of seconds allowed to process a single event
  unsigned int m_eventTimeout;

  /// Pointer to the watchdog thread that checks for the event timeout.
  std::auto_ptr<WatchdogThread> m_watchdog;

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;

};

#endif // STALLEDEVENTMONITOR_H_

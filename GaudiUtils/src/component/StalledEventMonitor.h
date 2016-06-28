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
class StalledEventMonitor: public extends<Service,
                                          IIncidentListener> {
public:
  /// Constructor
  StalledEventMonitor(const std::string& name, ISvcLocator* svcLoc);

  /// Destructor
  ~StalledEventMonitor() override = default;

  /// Initialization of the service.
  /// Prepare the watchdog thread and configures it.
  StatusCode initialize() override;

  /// Start the watchdog thread (before entering the event loop).
  StatusCode start() override;

  /// Notify the watchdog thread for a new event.
  void handle(const Incident&  /* incident */) override;

  /// Stop the watchdog thread (after the event loop).
  StatusCode stop() override;

  /// Finalization of the service.
  StatusCode finalize() override;

private:

  PropertyWithValue<unsigned int>  m_eventTimeout {this, "EventTimeout",  600,  "Number of seconds allowed to process a single event (0 to disable the check)."};
  IntegerProperty  m_maxTimeoutCount {this, "MaxTimeoutCount",  0,  "Number timeouts before aborting the execution (0 means never abort)."};
  BooleanProperty  m_stackTrace {this, "StackTrace",  false,  "Whether to print the stack-trace on timeout."};

  /// Pointer to the watchdog thread that checks for the event timeout.
  std::unique_ptr<WatchdogThread> m_watchdog;

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;

};

#endif // STALLEDEVENTMONITOR_H_

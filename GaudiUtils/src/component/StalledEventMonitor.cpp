/*
 * StalledEventMonitor.cpp
 *
 *  Created on: Apr 19, 2010
 *      Author: Marco Clemencic
 */

// Include files
#include "StalledEventMonitor.h"

#include "GaudiKernel/WatchdogThread.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Memory.h"

namespace {
  /// Specialized watchdog to monitor the event loop and spot possible infinite loops.
  class EventWatchdog: public WatchdogThread {
  public:
    EventWatchdog(const SmartIF<IMessageSvc> &msgSvc,
                  const std::string &name,
                  boost::posix_time::time_duration timeout,
                  bool autostart = false):
        WatchdogThread(timeout, autostart),
        log(msgSvc, name),
        m_counter(0) {}
    virtual ~EventWatchdog() {}
  private:
    MsgStream log;
    long m_counter;
    void action() {
      if (!m_counter) {
        log << MSG::WARNING << "More than " << getTimeout().total_seconds()
            << "s since the last " << IncidentType::BeginEvent << endmsg;
      } else {
        log << MSG::WARNING << "Other " << getTimeout().total_seconds()
            << "s passed" << endmsg;
      }
      log << MSG::INFO << "Current memory usage is"
          " virtual size = " << System::virtualMemory() / 1024. << " MB"
          ", resident set size = " << System::pagedMemory() / 1024.<< " MB"
          << endmsg;
      ++m_counter;
    }
    void onPing() {
      if (m_counter) {
        if (m_counter >= 3)
          log << MSG::INFO << "Starting a new event after ~"
              << m_counter * getTimeout().total_seconds() << "s" << endmsg;
        m_counter = 0;
      }
    }
    void onStop() {
      if (m_counter >= 3)
        log << MSG::INFO << "The last event took ~"
        << m_counter * getTimeout().total_seconds() << "s" << endmsg;
    }
  };
}

// Constructor
StalledEventMonitor::StalledEventMonitor(const std::string& name, ISvcLocator* svcLoc):
  base_class(name, svcLoc) {

  declareProperty("EventTimeout", m_eventTimeout = 600,
                  "Number of seconds allowed to process a single event (0 to disable the check)");

}

// Destructor
StalledEventMonitor::~StalledEventMonitor(){

}

// Initialization of the service.
StatusCode StalledEventMonitor::initialize() {
  StatusCode sc = base_class::initialize();
  if (sc.isFailure()) return sc;


  if (m_eventTimeout) {
    // create the watchdog thread
    m_watchdog = std::auto_ptr<WatchdogThread>(
        new EventWatchdog(msgSvc(),
            "EventWatchdog",
            boost::posix_time::seconds(m_eventTimeout)));

    // register to the incident service
    std::string serviceName = "IncidentSvc";
    m_incidentSvc = serviceLocator()->service(serviceName);
    if ( ! m_incidentSvc ) {
      error() << "Cannot retrieve " << serviceName << endmsg;
      return StatusCode::FAILURE;
    }
    debug() << "Register to the IncidentSvc" << endmsg;
    m_incidentSvc->addListener(this, IncidentType::BeginEvent);
  } else {
    warning() << "StalledEventMonitor/" << name()
       << " instantiated with 0 time-out: no monitoring performed" << endmsg;
  }

  return StatusCode::SUCCESS;
}

// Start the monitoring.
StatusCode StalledEventMonitor::start() {
  if (m_watchdog.get()) m_watchdog->start();
  return StatusCode::SUCCESS;
}

// Notify the watchdog that a new event has been started
void StalledEventMonitor::handle(const Incident& /* incident */) {
  if (m_watchdog.get()) m_watchdog->ping();
}

// Start the monitoring.
StatusCode StalledEventMonitor::stop() {
  if (m_watchdog.get()) m_watchdog->stop();
  return StatusCode::SUCCESS;
}

// Finalization of the service.
StatusCode StalledEventMonitor::finalize() {
  // destroy the watchdog thread (if any)
  m_watchdog.reset();
  // unregistering from the IncidentSvc
  m_incidentSvc->removeListener(this, IncidentType::BeginEvent);
  m_incidentSvc.reset();
  return base_class::finalize();
}

// Declaration of the factory
#include "GaudiKernel/SvcFactory.h"
DECLARE_SERVICE_FACTORY(StalledEventMonitor)

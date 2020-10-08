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
/*
 * StalledEventMonitor.cpp
 *
 *  Created on: Apr 19, 2010
 *      Author: Marco Clemencic
 */

// Include files
#include "StalledEventMonitor.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Memory.h"
#include "GaudiKernel/WatchdogThread.h"

#include "TSystem.h"

#include <csignal>

namespace {
  /// Specialized watchdog to monitor the event loop and spot possible infinite loops.
  class EventWatchdog : public WatchdogThread {
  public:
    EventWatchdog( const SmartIF<IMessageSvc>& msgSvc, const std::string& name,
                   boost::posix_time::time_duration timeout, bool stackTrace = false, long maxCount = 0,
                   bool autostart = false )
        : WatchdogThread( timeout, autostart )
        , log( msgSvc, name )
        , m_maxCount( maxCount )
        , m_stackTrace( stackTrace ) {}

  private:
    /// message stream used to report problems
    MsgStream log;
    /// internal counter of the occurrences of consecutive timeouts
    long m_counter = 0;
    /// how many timeouts before aborting (0 means never abort)
    long m_maxCount = 0;
    /// whether to dump a stack trace when the timeout is reached
    bool m_stackTrace = false;
    /// main watchdog function
    void action() override {
      if ( !m_counter ) {
        log << MSG::WARNING << "More than " << getTimeout().total_seconds() << "s since the last "
            << IncidentType::BeginEvent << endmsg;
      } else {
        log << MSG::WARNING << "Other " << getTimeout().total_seconds() << "s passed" << endmsg;
      }
      log << MSG::INFO
          << "Current memory usage is"
             " virtual size = "
          << System::virtualMemory() / 1024.
          << " MB"
             ", resident set size = "
          << System::pagedMemory() / 1024. << " MB" << endmsg;
      if ( m_stackTrace && gSystem ) {
        // TSystem::StackTrace() prints on the standard error, so we
        std::cerr << "=== Stalled event: current stack trace ===" << std::endl;
        gSystem->StackTrace();
      }
      ++m_counter;
      if ( m_maxCount > 0 && m_counter >= m_maxCount ) {
        log << MSG::FATAL << "too much time on a single event: aborting process" << endmsg;
        std::raise( SIGQUIT );
      }
    }
    void onPing() override {
      if ( m_counter ) {
        if ( m_counter >= 3 )
          log << MSG::INFO << "Starting a new event after ~" << m_counter * getTimeout().total_seconds() << "s"
              << endmsg;
        m_counter = 0;
      }
    }
    void onStop() override {
      if ( m_counter >= 3 )
        log << MSG::INFO << "The last event took ~" << m_counter * getTimeout().total_seconds() << "s" << endmsg;
    }
  };
} // namespace

// Initialization of the service.
StatusCode StalledEventMonitor::initialize() {
  StatusCode sc = base_class::initialize();
  if ( sc.isFailure() ) return sc;

  if ( m_eventTimeout ) {
    // create the watchdog thread
    m_watchdog = std::make_unique<EventWatchdog>( msgSvc(), "EventWatchdog",
                                                  boost::posix_time::seconds( m_eventTimeout.value() ), m_stackTrace,
                                                  m_maxTimeoutCount );

    // register to the incident service
    static const std::string serviceName = "IncidentSvc";
    m_incidentSvc                        = serviceLocator()->service( serviceName );
    if ( !m_incidentSvc ) {
      error() << "Cannot retrieve " << serviceName << endmsg;
      return StatusCode::FAILURE;
    }
    debug() << "Register to the IncidentSvc" << endmsg;
    m_incidentSvc->addListener( this, IncidentType::BeginEvent );
  } else {
    warning() << "StalledEventMonitor/" << name() << " instantiated with 0 time-out: no monitoring performed" << endmsg;
  }

  return StatusCode::SUCCESS;
}

// Start the monitoring.
StatusCode StalledEventMonitor::start() {
  if ( m_watchdog ) m_watchdog->start();
  return StatusCode::SUCCESS;
}

// Notify the watchdog that a new event has been started
void StalledEventMonitor::handle( const Incident& /* incident */ ) {
  if ( m_watchdog ) m_watchdog->ping();
}

// Start the monitoring.
StatusCode StalledEventMonitor::stop() {
  if ( m_watchdog ) m_watchdog->stop();
  return StatusCode::SUCCESS;
}

// Finalization of the service.
StatusCode StalledEventMonitor::finalize() {
  // destroy the watchdog thread (if any)
  m_watchdog.reset();
  // unregistering from the IncidentSvc
  m_incidentSvc->removeListener( this, IncidentType::BeginEvent );
  m_incidentSvc.reset();
  return base_class::finalize();
}

// Declaration of the factory
DECLARE_COMPONENT( StalledEventMonitor )

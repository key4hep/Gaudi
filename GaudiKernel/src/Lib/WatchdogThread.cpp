/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * Implementation of the class WatchdogThread.
 */

#include <GaudiKernel/WatchdogThread.h>
#include <thread>

WatchdogThread::WatchdogThread( std::chrono::seconds timeout, bool autostart ) : m_timeout( std::move( timeout ) ) {
  // Start the thread immediately if requested.
  if ( autostart ) start();
}

WatchdogThread::~WatchdogThread() {
  // Make sure the thread is stopped before exiting.
  stop();
}

void WatchdogThread::start() {
  if ( !m_thread.joinable() ) { // can be started only if the thread is not yet started
    // call user-defined function
    onStart();
    // Initialize the first "last ping"
    ping();
    // Start a new thread telling it to call the member function i_run()
    m_thread = std::thread{ [this, is_stopped = m_stop_thread.get_future()]() {
      // Copy of the last ping
      auto lastPing = getLastPing();

      // set initial check time
      auto nextCheck = lastPing + getTimeout();

      // enter infinite loop
      // Wait until the next check point time is reached.
      while ( is_stopped.wait_until( nextCheck ) == std::future_status::timeout ) {
        // Check if there was a ping while we were sleeping
        if ( lastPing == getLastPing() ) { // no further accesses
          action();
          // schedule the next check for now + dt (seems a good estimate)
          nextCheck = clock::now() + getTimeout();
        } else { // there was a ping
          // schedule the next check for last_access + dt
          lastPing  = getLastPing();
          nextCheck = lastPing + getTimeout();
        }
      }
    } };
  }
}

void WatchdogThread::stop() {
  if ( m_thread.joinable() ) {
    m_stop_thread.set_value(); // tell the thread to stop
    m_thread.join();           // wait for it
    // call user-defined function
    onStop();
  }
}

// Default implementation: empty
void WatchdogThread::action() {}

// Default implementation: empty
void WatchdogThread::onPing() {}

// Default implementation: empty
void WatchdogThread::onStart() {}

// Default implementation: empty
void WatchdogThread::onStop() {}

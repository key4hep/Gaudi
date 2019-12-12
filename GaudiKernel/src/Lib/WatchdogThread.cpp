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
 * Implementation of the class WatchdogThread.
 */

#include "GaudiKernel/WatchdogThread.h"
#include "GaudiKernel/Sleep.h"

#include <functional>

#include <boost/thread/thread.hpp>

WatchdogThread::WatchdogThread( boost::posix_time::time_duration timeout, bool autostart )
    : m_timeout( std::move( timeout ) ), m_running( false ) {
  // Start the thread immediately if requested.
  if ( autostart ) start();
}

WatchdogThread::~WatchdogThread() {
  // Make sure the thread is stopped before exiting.
  stop();
}

void WatchdogThread::start() {
  if ( !m_thread ) { // can be started only if the thread is not yet started
    m_running = true;
    // call user-defined function
    onStart();
    // Initialize the first "last ping"
    ping();
    // Start a new thread telling it to call the member function i_run()
    m_thread = std::make_unique<boost::thread>( std::mem_fn( &WatchdogThread::i_run ), this );
  }
}

void WatchdogThread::stop() {
  if ( m_thread ) {
    m_running = false;           // mark the thread as stopped (interrupt doesn't work if the thread is not sleeping)
    Gaudi::NanoSleep( 1000000 ); // Wait a bit (1ms) to be sure that the interrupt happens during the sleep
    m_thread->interrupt();       // tell the thread to stop (if it is waiting)
    m_thread->join();            // wait for it
    m_thread.reset();            // delete it
    // call user-defined function
    onStop();
  }
}

void WatchdogThread::i_run() {
  // Copy of the last ping
  boost::system_time lastPing = getLastPing();

  // set initial check time
  boost::system_time nextCheck = lastPing + getTimeout();

  try {
    // enter infinite loop
    while ( m_running ) {
      // Wait until the next check point time is reached.
      // An early exit must be triggered by a call to this->interrupt(), which
      // will produce an exception during the sleep.
      boost::thread::sleep( nextCheck );
      // Check if there was a ping while we were sleeping
      if ( lastPing == getLastPing() ) { // no further accesses
        action();
        // schedule the next check for now + dt (seems a good estimate)
        nextCheck = boost::get_system_time() + getTimeout();
      } else { // there was a ping
        // schedule the next check for last_access + dt
        nextCheck = lastPing = getLastPing();
        nextCheck += getTimeout();
      }
    }
  }
  // Ignore the exception since it is used only to exit from the loop.
  catch ( boost::thread_interrupted& ) {}
}

// Default implementation: empty
void WatchdogThread::action() {}

// Default implementation: empty
void WatchdogThread::onPing() {}

// Default implementation: empty
void WatchdogThread::onStart() {}

// Default implementation: empty
void WatchdogThread::onStop() {}

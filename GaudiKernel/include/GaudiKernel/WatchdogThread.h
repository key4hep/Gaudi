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
#pragma once

#include <GaudiKernel/Kernel.h> // for GAUDI_API
#include <atomic>
#include <chrono>
#include <future>

/** @class WatchdogThread
 *  Simple class for asynchronous check of time-out.
 *
 *  The user must provide a callable with the action to be performed when the
 *  time-out occurs.
 *
 *  @author Marco Clemencic
 *  @date   2010-02-23
 */
class GAUDI_API WatchdogThread {
public:
  using clock      = std::chrono::system_clock;
  using time_point = std::chrono::time_point<clock>;

  /// Constructor.
  //  @param timeout the time span that can occur between two pings.
  //  @param autostart if set to true, the second thread is started automatically
  //                   on construction, otherwise the user have to call start().
  WatchdogThread( std::chrono::seconds timeout, bool autostart = false );

  /// Destructor.
  //  Stop the thread if not done earlier.
  virtual ~WatchdogThread();

  /// Start the watchdog thread
  void start();

  /// Signal the watchdog thread to stop and wait for it.
  void stop();

  /// Function to call to notify the watchdog thread that we are still alive.
  inline void ping() {
    m_lastPing.store( clock::now() );
    onPing();
  }

  /// Change the duration of the time-out.
  inline void setTimeout( std::chrono::seconds timeout ) { m_timeout = timeout; }

  /// Get the current time-out value.
  inline std::chrono::seconds getTimeout() const { return m_timeout; }

  /// Get the time of latest ping.
  inline time_point getLastPing() const { return m_lastPing.load(); }

protected:
  /// User implemented function that will be called if the time-out is reached.
  virtual void action();

  /// User implemented function that will be called when ping is called.
  virtual void onPing();

  /// User implemented function that will be called when starting.
  virtual void onStart();

  /// User implemented function that will be called when stopping.
  virtual void onStop();

private:
  /// Number of seconds allowed between pings.
  std::chrono::seconds m_timeout;

  /// When the last ping was received.
  std::atomic<time_point> m_lastPing;

  /// Running thread;
  std::thread m_thread;

  /// Flag to mark the thread as running/stopped (avoid possible race conditions).
  std::promise<void> m_stop_thread;
};

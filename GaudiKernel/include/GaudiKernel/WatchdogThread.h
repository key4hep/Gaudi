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
#ifndef GAUDIKERNEL_WATCHDOGTHREAD_H_
#define GAUDIKERNEL_WATCHDOGTHREAD_H_

#include <memory>
#include <mutex>

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/thread/thread_time.hpp"

// for GAUDI_API
#include "GaudiKernel/Kernel.h"

// forward declaration
namespace boost {
  class thread;
}

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
  /// Constructor.
  //  @param timeout the time span that can occur between two pings.
  //  @param autostart if set to true, the second thread is started automatically
  //                   on construction, otherwise the user have to call start().
  WatchdogThread( boost::posix_time::time_duration timeout, bool autostart = false );

  /// Destructor.
  //  Stop the thread of not done earlier.
  virtual ~WatchdogThread();

  /// Start the watchdog thread
  void start();

  /// Signal the watchdog thread to stop and wait for it.
  void stop();

  /// Function to call to notify the watchdog thread that we are still alive.
  inline void ping() {
    auto lock  = std::scoped_lock{m_lastPingMutex};
    m_lastPing = boost::get_system_time();
    onPing();
  }

  /// Change the duration of the time-out.
  inline void setTimeout( boost::posix_time::time_duration timeout ) { m_timeout = timeout; }

  /// Get the current time-out value.
  inline boost::posix_time::time_duration getTimeout() const { return m_timeout; }

  /// Get the time of latest ping.
  inline boost::system_time getLastPing() const {
    auto lock = std::scoped_lock{m_lastPingMutex};
    return m_lastPing;
  }

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
  boost::posix_time::time_duration m_timeout;

  /// When the last ping was received.
  boost::system_time m_lastPing;

  /// Pointer to the running thread;
  std::unique_ptr<boost::thread> m_thread;

  /// Flag to mark the thread as running/stopped (avoid possible race conditions).
  bool m_running;

  /// Core function of the secondary thread.
  //  Waits for the time-out and if there was not a ping in the mean time, calls
  //  i_action().
  void i_run();

  /// Mutex for the access to the m_lastPing data member.
  mutable std::mutex m_lastPingMutex;
};

#endif /* WATCHDOGTHREAD_H_ */

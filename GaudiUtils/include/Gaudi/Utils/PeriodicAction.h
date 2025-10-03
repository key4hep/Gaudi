/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <thread>

namespace Gaudi::Utils {
  /** Helper to periodically run asynchronous tasks.
   *
   * An instance of this starts a dedicated thread that periodically runs the user
   * specified callback.
   *
   * The thread starts as soon as the instance is created and it's automatically terminated
   * (and joined) on destruction.
   * It's also possible to defer the start (if requested at construction time),
   * anticipate the stop and specify a maximum number of repetitions (if >0).
   */
  class PeriodicAction {
  public:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using callback_t = std::function<void()>;

    PeriodicAction( callback_t callback, std::chrono::milliseconds period_duration, bool autostart = true,
                    unsigned int repetitions = 0 );
    PeriodicAction( PeriodicAction&& )            = default;
    PeriodicAction& operator=( PeriodicAction&& ) = default;

    ~PeriodicAction();

    void start();
    void stop();

  private:
    std::thread               m_thread;
    std::promise<void>        m_stop_thread;
    callback_t                m_callback;
    std::chrono::milliseconds m_period_duration;
    unsigned int              m_repetitions;
  };
} // namespace Gaudi::Utils

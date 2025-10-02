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
#include <Gaudi/Utils/PeriodicAction.h>
#include <utility>

using Gaudi::Utils::PeriodicAction;

PeriodicAction::PeriodicAction( callback_t callback, std::chrono::milliseconds period_duration, bool autostart )
    : m_callback{ std::move( callback ) }, m_period_duration{ period_duration } {
  if ( autostart ) start();
}

PeriodicAction::~PeriodicAction() { stop(); }

void PeriodicAction::start() {
  if ( !m_thread.joinable() && m_period_duration.count() > 0 ) {
    // Note: we can move the callback because we are not going to use it
    // outside of the thread
    m_thread = std::thread{ [period_duration = m_period_duration, callback = std::move( m_callback ),
                             stop_signal = m_stop_thread.get_future()] {
      auto next_call = clock::now() + period_duration;
      while ( stop_signal.wait_until( next_call ) == std::future_status::timeout ) {
        callback();
        // ensure the next call is at a multiple
        // of m_period_duration after the last one
        const auto now = clock::now();
        while ( next_call < now ) next_call += period_duration;
      }
    } };
  }
}

void PeriodicAction::stop() {
  if ( m_thread.joinable() ) {
    m_stop_thread.set_value();
    m_thread.join();
  }
}

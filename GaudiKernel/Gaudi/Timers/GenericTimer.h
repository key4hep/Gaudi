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
#pragma once

#include "Gaudi/Chrono/Counters.h"

#include <atomic>
#include <chrono>

namespace Gaudi {
  namespace Timers {
    /**
     * A generic timer based on std::chrono and Gaudi::Accumulators
     *
     * The timer is templated with a std::chrono compatible clock and the Unit
     * of precision (ms, us, ...). It accumulates all time measurements in a
     * Gaudi::Accumulators::StatCounter. A time measurement is done via the RAII helper
     * class created by calling operator().
     *
     * Usually one would use one of the predefined Timer types from Gaudi/Timers.h:
     *
     * @code
     *   // .h
     *   Gaudi::Timer m_timer;
     *
     *   // .cpp
     *   {
     *     auto timeit = m_timer();
     *     // code to be timed
     *   }
     *   info() << m_timer.stats() << endmsg;
     * @endcode
     */
    template <typename Clock, typename Unit>
    class GenericTimer {
    public:
      /// Type of Counter used for accumulating time measurements
      typedef Gaudi::Accumulators::StatCounter<Unit> Stats_t;

      /**
       * A scoped timer that starts/stops on con/de-struction
       */
      class ScopeTimer {
      public:
        /// Start Scoped timer accumulating into stat
        ScopeTimer( Stats_t& stat ) : m_stats( stat ), m_t0( Clock::now() ) {}

        /// Destructor stopping timer
        ~ScopeTimer() { stop(); }

        /// Return the elapsed time without stopping the timer
        /// @return std::chrono::duration<Unit>
        auto elapsed() const {
          // Fenced according to https://codereview.stackexchange.com/q/196245
          std::atomic_thread_fence( std::memory_order_relaxed );
          auto dt = Clock::now() - m_t0;
          std::atomic_thread_fence( std::memory_order_relaxed );
          return std::chrono::duration_cast<Unit>( dt );
        }

      private:
        /// Stop the timer, accumulate elapsed time and return current measurement
        void stop() const { m_stats += elapsed(); }

        Stats_t&                   m_stats; ///< reference to statistics counter
        typename Clock::time_point m_t0;    ///< start time of timer
      };

      /// Default constructor
      GenericTimer() = default;

      /// Constructor attaching the statistics counter to an owner
      template <class OWNER>
      GenericTimer( OWNER* o, const std::string& name ) {
        o->declareCounter( name, m_stats );
      }

      /// No copy
      GenericTimer( const GenericTimer& ) = delete;

      /// Create (and start) a ScopeTimer
      [[nodiscard]] auto operator()() const { return ScopeTimer( m_stats ); }

      /// Return accumulated timing statistics
      const Stats_t& stats() const { return m_stats; }

    private:
      mutable Stats_t m_stats; ///< statistics counter
    };

  } // namespace Timers
} // namespace Gaudi

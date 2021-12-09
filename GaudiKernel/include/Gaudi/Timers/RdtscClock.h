/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#ifndef __x86_64__
#  error "<Gaudi/Timers/RdtscClock.h> is only supported on x86"
#endif // not __x86_64__

#include <chrono>
#include <functional>
#include <ratio>
#include <thread>
#include <x86intrin.h>

namespace Gaudi {
  namespace Timers {
    /**
     * @brief A std::chrono compatible Clock using rdtsc as its timing source
     *
     * This clock is not directly related to wall time but meant as a low-overhead
     * time interval measurement. The Precision template parameter determines the
     * internal precision of the clock (e.g. `std::milli`, `std::micro`). On first
     * invocation of `now()` the timer is calibrated against a wall-time clock,
     * which will have an overhead of several milliseconds. This can be avoided by
     * calling `calibrate()` explicitly before the first measurement.
     *
     */
    template <typename Precision = std::chrono::microseconds>
    class RdtscClock {
    public:
      // to meet requirements of TrivialClock:
      typedef typename Precision::rep             rep;
      typedef typename Precision::period          period;
      typedef std::chrono::duration<rep, period>  duration;
      typedef std::chrono::time_point<RdtscClock> time_point;

      static constexpr bool is_steady{ true };

      // some sanity checks:
      static_assert( period::num == 1, "The Precision of RdtscClock must be reducible to 1/N" );
      static_assert( std::ratio_less_equal<period, std::milli>(),
                     "The Precision of RdtscClock must be at least std::chrono::milliseconds" );

      /// Calibrate the RDTSC clock against wall time
      static rep calibrate() noexcept { return ticks_per_unit(); }

      static time_point now() noexcept { return time_point{ duration( __rdtsc() / ticks_per_unit() ) }; }

    private:
      static rep ticks_per_unit() noexcept {
        static rep ticks_per_unit = do_calibrate(); // local static guarantees thread-safety
        return ticks_per_unit;
      }

      static rep do_calibrate() noexcept {
        // Calibration time and conversion factor to unit of Precision
        static constexpr auto calibTime = std::chrono::milliseconds( 100 );
        static constexpr auto toPrec    = std::ratio_divide<std::milli, period>::num;

        // Calibrate against wall clock
        auto t1_ref = std::chrono::high_resolution_clock::now();
        auto t1     = __rdtsc();
        std::this_thread::sleep_for( calibTime );
        auto t2     = __rdtsc();
        auto t2_ref = std::chrono::high_resolution_clock::now();

        // Calculate ticks per unit of Precision
        auto dt_ref         = std::chrono::duration_cast<std::chrono::milliseconds>( t2_ref - t1_ref ).count();
        rep  ticks_per_unit = ( t2 - t1 ) / ( dt_ref * toPrec );

        return ticks_per_unit;
      }
    };
  } // namespace Timers
} // namespace Gaudi

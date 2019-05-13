#pragma once

#include "Gaudi/Timers/GenericTimer.h"
#include "Gaudi/Timers/RdtscClock.h"

#include <chrono>

namespace Gaudi {
  /**
   * Default timer using a standard clock with microsecond precision
   *
   * \see Gaudi::Timers::GenericTimer
   */
  using Timer = Timers::GenericTimer<std::chrono::high_resolution_clock, std::chrono::microseconds>;

  /**
   * Fast RDTSC-based timer with microsecond precision
   *
   * \see Gaudi::Timers::GenericTimer
   * \see Gaudi::Timers::RdtscClock
   */
  using FastTimer = Timers::GenericTimer<Timers::RdtscClock<std::chrono::microseconds>, std::chrono::microseconds>;
} // namespace Gaudi

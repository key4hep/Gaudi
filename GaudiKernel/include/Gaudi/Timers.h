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

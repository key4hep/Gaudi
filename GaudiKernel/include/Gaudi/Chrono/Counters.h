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

/**
 * @brief Header file for std:chrono::duration-based Counters
 *
 * Include this header file (instead of GaudiKernel/Counters.h) if you want to
 * use std::chrono::duration-based Counters. It includes support for the required
 * arithmetic and stream operations.
 */
#include <chrono>
#include <cmath>

namespace Gaudi {
  namespace Accumulators {
    /*
     * std::chrono::duration only supports arithmetic operations that preserve the unit
     * of time, e.g. there are no operations that would result in units of ms^2. This can
     * be inconvenient for the use in general purpose libraries like Gaudi::Accumulators.
     * This header provides the missing operators.
     */

    /// sqrt for std::chrono::duration
    template <class Rep, class Period>
    auto sqrt( std::chrono::duration<Rep, Period> d ) {
      return std::chrono::duration<Rep, Period>( static_cast<Rep>( std::round( std::sqrt( d.count() ) ) ) );
    }

    /// Multiplication of two std::chrono::duration objects with same Period
    template <class Rep1, class Rep2, class Period>
    auto operator*( const std::chrono::duration<Rep1, Period>& lhs, const std::chrono::duration<Rep2, Period>& rhs ) {
      return std::chrono::duration<std::common_type_t<Rep1, Rep2>, Period>( lhs.count() * rhs.count() );
    }
  } // namespace Accumulators
} // namespace Gaudi

#include <Gaudi/Accumulators.h>
#include <Gaudi/Chrono/ChronoIO.h>

#pragma once
/**
 * @brief Provide additional arithmetic operations for std::chrono::duration
 *
 * std::chrono::duration only supports arithmetic operations that preserve the unit
 * of time, e.g. there are no operations that would result in units of ms^2. This can
 * be inconvenient for the use in general purpose libraries like Gaudi::Accumulators.
 * This header file provides a few of the missing operations.
 */
#include <chrono>
#include <cmath>

namespace std {
  namespace chrono {
    /// sqrt for std::chrono duration
    template <class Rep, class Period>
    auto sqrt( duration<Rep, Period> d ) {
      return duration<Rep, Period>( static_cast<Rep>( std::round( std::sqrt( d.count() ) ) ) );
    }

    /// Multiplication of two std::chrono duration objects with same Period
    template <class Rep1, class Rep2, class Period>
    auto operator*( const duration<Rep1, Period>& lhs, const duration<Rep2, Period>& rhs ) {
      return duration<std::common_type_t<Rep1, Rep2>, Period>( lhs.count() * rhs.count() );
    }
  } // namespace chrono
} // namespace std

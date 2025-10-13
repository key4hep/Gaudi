/***********************************************************************************\
* (c) Copyright 2023-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

/** Definitions to allow use of Gaudi::Property<T> with fmtlib.
 *
 * When this header is included, one can pass a Gaudi::Property<T> instance as argument to `fmt::format`.
 *
 * For example
 * ```cpp
 * #include <Gaudi/Property.h>
 * #include <Gaudi/PropertyFmt.h>
 * #include <fmt/format.h>
 *
 * int main() {
 *   Gaudi::Property<int> p{ "MyProp", 20 };
 *   fmt::print( "property {} has value {}\n", p.name(), p );
 * }
 * ```
 * will print
 * ```
 * property MyProp has value 20
 * ```
 *
 * A special formatting option can be used to print the property name as well as the value, so
 * ```cpp
 * #include <Gaudi/Property.h>
 * #include <Gaudi/PropertyFmt.h>
 * #include <fmt/format.h>
 * #include <iostream>
 *
 * int main() {
 *   Gaudi::Property<int> p{ "MyProp", 20 };
 *
 *   std::cout << "cout" << p << '\n';
 *   fmt::print( "fmt {:?}\n", p );
 * }
 * ```
 * will print
 * ```
 * cout 'MyProp':20
 * fmt  'MyProp':20
 * ```
 */

#include <Gaudi/Property.h>
#include <fmt/format.h>
#include <format>
#include <iomanip>
#include <sstream>

#define GAUDI_PROPERTY_FORMATTING_IMPL( ns, assert_for_ranges )                                                        \
  template <typename T, typename V, typename H>                                                                        \
  struct ns::formatter<Gaudi::Property<T, V, H>> : ns::formatter<T> {                                                  \
    assert_for_ranges;                                                                                                 \
    bool           debug = false;                                                                                      \
    constexpr auto parse( ns::format_parse_context& ctx ) {                                                            \
      auto it = ctx.begin(), end = ctx.end();                                                                          \
      if ( it != end && *it == '?' ) {                                                                                 \
        debug = true;                                                                                                  \
        ++it;                                                                                                          \
        if ( it != end && *it != '}' ) throw ns::format_error( "invalid format" );                                     \
        return it;                                                                                                     \
      }                                                                                                                \
      return ns::formatter<T>::parse( ctx );                                                                           \
    }                                                                                                                  \
    auto format( const Gaudi::Property<T, V, H>& p, ns::format_context& ctx ) const {                                  \
      if ( debug ) {                                                                                                   \
        if constexpr ( std::is_same_v<T, std::string> ) {                                                              \
          std::stringstream s;                                                                                         \
          s << std::quoted( p.value(), '\'' );                                                                         \
          return ns::format_to( ctx.out(), " '{}':{}", p.name(), s.str() );                                            \
        } else {                                                                                                       \
          return ns::format_to( ctx.out(), " '{}':{}", p.name(), p.value() );                                          \
        }                                                                                                              \
      } else {                                                                                                         \
        return ns::formatter<T>::format( static_cast<const T&>( p ), ctx );                                            \
      }                                                                                                                \
    }                                                                                                                  \
  };

GAUDI_PROPERTY_FORMATTING_IMPL( fmt, static_assert( true ) )

#ifndef __cpp_lib_format_ranges
#  define assert_for_ranges                                                                                            \
    static_assert( !std::ranges::range<T> || std::is_same_v<T, std::string>,                                           \
                   "Range-valued Property formatting is not supported with std::format in C++20" )
#else
#  define assert_for_ranges static_assert( true )
#endif

GAUDI_PROPERTY_FORMATTING_IMPL( std, assert_for_ranges )

#undef assert_for_ranges
#undef GAUDI_PROPERTY_FORMATTING_IMPL

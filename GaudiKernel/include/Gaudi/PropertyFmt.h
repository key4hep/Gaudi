/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
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
#include <iomanip>
#include <sstream>

template <typename T, typename V, typename H>
struct fmt::formatter<Gaudi::Property<T, V, H>> : formatter<T> {
  bool           debug = false;
  constexpr auto parse( format_parse_context& ctx ) -> format_parse_context::iterator {
    auto it = ctx.begin(), end = ctx.end();
    if ( it != end && *it == '?' ) {
      debug = true;
      ++it;
      if ( it != end && *it != '}' ) detail::error_handler().on_error( "invalid format" );
      return it;
    }
    return formatter<T>::parse( ctx );
  }
  auto format( const Gaudi::Property<T, V, H>& p, format_context& ctx ) const {
    if ( debug ) {
      if constexpr ( std::is_same_v<T, std::string> ) {
        std::stringstream s;
        s << std::quoted( p.value(), '\'' );
        return fmt::format_to( ctx.out(), " '{}':{}", p.name(), s.str() );
      } else {
        return fmt::format_to( ctx.out(), " '{}':{}", p.name(), p.value() );
      }
    } else {
      return formatter<T>::format( static_cast<const T&>( p ), ctx );
    }
  }
};

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
#ifndef GAUDIKERNEL_COMPOSE_H
#define GAUDIKERNEL_COMPOSE_H

#include <utility> // std::forward, std::move (objects)

namespace Gaudi {

  namespace details {

    // C++17 version: https://godbolt.org/g/YdcvGg
    template <typename... lambda_ts>
    struct overloaded_t : lambda_ts... {
      using lambda_ts::operator()...;
    };
    template <typename... lambda_ts>
    overloaded_t( lambda_ts... )->overloaded_t<lambda_ts...>;

  } // namespace details

  //
  // Create an object with an overloaded call operator by 'composing'/'joining'
  // a set of callables (such as lambdas)
  //
  // see eg. the example at http://en.cppreference.com/w/cpp/utility/variant/visit
  // for an example of why this is usefull
  //
  template <typename... lambda_ts>
  auto overload( lambda_ts&&... lambdas ) {
    return details::overloaded_t{std::forward<lambda_ts>( lambdas )...};
  }
} // namespace Gaudi

// for backwards compatibility
// [[deprecated("please use Gaudi::overload instead of compose")]]
template <typename... lambda_ts>
auto compose( lambda_ts&&... lambdas ) {
  return Gaudi::overload( std::forward<lambda_ts>( lambdas )... );
}

#endif

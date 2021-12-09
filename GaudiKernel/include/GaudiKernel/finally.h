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
#include <utility>
//
//  make it possible to execute an action at the exit of a scope
//
//  auto f = finally( [](){ std::cout << "end of scope!" << std::endl; } );
//
//  the above will execute the provided callable when f goes out of scope,
//  i.e. the 'current' scope ends.

template <typename F>
struct final_action {
  F act;
  final_action( final_action&& ) = default;
  ~final_action() { act(); }
};

template <typename F>
final_action<F> finally( F&& act ) {
  return { std::forward<F>( act ) };
}

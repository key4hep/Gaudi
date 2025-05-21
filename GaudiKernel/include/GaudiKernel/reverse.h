/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
//
// provide a generic 'reverse' function for use in range-based for loops.
//
// example:
//
//      #include "GaudiKernel/reverse.h"
//      auto l = { 1,2,3,4 };
//      for ( const auto& i : reverse( l ) ) std::cout << i << std::endl;
//
//  Note that it is perfectly fine to reverse a temporary. The temporary
//  will be moved into the reverse_wrapper, and thus kept alive for the
//  duration of the loop. In case reverse is called on an lvalue, the wrapper
//  will take a reference, so no copy will be performed. (if you wonder how
//  the code below (very implicitly) distinguishes between the two cases,
//  google for 'C++11 reference collapsing' -- short version: C++ does not
//  allow references to references, so in cases where this would happen,
//  one gets an lvalue reference)
//
//  Also note that reverse_wrapper does not have a constructor, but this
//  implies that it does meet the requirements for aggregate initializaton,
//  which allows for {} initialization of its member.
//

#include <iterator>
#include <utility>

namespace details {

  template <typename Iterable>
  struct reverse_wrapper {
    Iterable iterable;
  };

  template <typename T>
  auto begin( reverse_wrapper<T>& w ) {
    using std::rbegin;
    return rbegin( w.iterable );
  }
  template <typename T>
  auto end( reverse_wrapper<T>& w ) {
    using std::rend;
    return rend( w.iterable );
  }
} // namespace details

template <typename T>
::details::reverse_wrapper<T> reverse( T&& iterable ) {
  return { std::forward<T>( iterable ) };
}

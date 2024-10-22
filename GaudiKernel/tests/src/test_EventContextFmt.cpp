/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/EventContext.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "EventContext formatting" ) {
  CHECK( fmt::format( "{}", EventContext{ 1, 2 } ) == "s: 2  e: 1" );
  CHECK( fmt::format( "{}", EventContext{ 1, 2, 3 } ) == "s: 2  e: 1 sub: 3" );
  CHECK( fmt::format( ">{:>20}<", EventContext{ 1, 2, 3 } ) == ">   s: 2  e: 1 sub: 3<" );

  CHECK( fmt::format( "{}", EventContext{} ) == "INVALID" );
  CHECK( fmt::format( "{}", EventContext{ 1 } ) == "INVALID" );

  CHECK( fmt::format( "***{:^20}***", EventContext{} ) == "***      INVALID       ***" );

  EventContext ctx{ 1, 2, 3 };
  {
    std::ostringstream out;
    out << ctx;
    CHECK( out.str() == "s: 2  e: 1 sub: 3" );
  }
  {
    std::ostringstream out;
    out << &ctx;
    CHECK( out.str() == "s: 2  e: 1 sub: 3" );
  }
  {
    std::ostringstream out;
    out << static_cast<EventContext*>( nullptr );
    CHECK( out.str() == "INVALID" );
  }
}

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
#include <GaudiKernel/finally.h>
#include <string>

#if __has_include( <catch2/catch.hpp> )
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "finally" ) {
  std::string store = "initial";
  {
    CHECK( store == "initial" );
    auto f = finally( [&store]() { store = "final"; } );
    store  = "temporary";
    CHECK( store == "temporary" );
  }
  CHECK( store == "final" );
}

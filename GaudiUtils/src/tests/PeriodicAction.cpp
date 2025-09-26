/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Utils/PeriodicAction.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  if __has_include( <catch2/matchers/catch_matchers_floating_point.hpp>)
#    include <catch2/matchers/catch_matchers_floating_point.hpp>
#  else
#    include <catch2/catch_matchers_floating_point.hpp>
#  endif
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

using namespace std::chrono_literals;
using Gaudi::Utils::PeriodicAction;

TEST_CASE( "PeriodicAction" ) {
  SECTION( "default" ) {
    int counter{ 0 };
    {
      PeriodicAction periodic{ [&counter]() { ++counter; }, 100ms };
      std::this_thread::sleep_for( 210ms );
    }
    CHECK( counter == 2 );
  }

  SECTION( "manual start" ) {
    int counter{ 0 };
    {
      PeriodicAction periodic{ [&counter]() { ++counter; }, 100ms, false };
      std::this_thread::sleep_for( 100ms );
      periodic.start();
      std::this_thread::sleep_for( 210ms );
      periodic.stop();
      std::this_thread::sleep_for( 100ms );
    }
    CHECK( counter == 2 );
  }

  SECTION( "slow action" ) {
    // ensure that intervals do not depend on speed of action,
    // i.e. 300ms mean 3 calls every 100ms even if the action takes 70ms
    int counter{ 0 };
    {
      PeriodicAction periodic{ [&counter]() {
                                ++counter;
                                std::this_thread::sleep_for( 70ms );
                              },
                               100ms };
      std::this_thread::sleep_for( 310ms );
    }
    CHECK( counter == 3 );
  }

  SECTION( "skipping invocations" ) {
    // if the action happens to take more than the allocated time we
    // skip the next invocation(s) to make sure we call it only on
    // a multiple of the interval
    int counter{ 0 };
    {
      // here we have one invocation at t == 100ms, 300ms and 500ms (we skip that at 200ms and 400ms)
      PeriodicAction periodic{ [&counter]() {
                                ++counter;
                                std::this_thread::sleep_for( 150ms );
                              },
                               100ms };
      std::this_thread::sleep_for( 510ms );
    }
    CHECK( counter == 3 );
  }

  SECTION( "zero duration" ) {
    // should not start a timer if duration is zero
    int counter{ 0 };
    {
      PeriodicAction periodic{ [&counter]() { ++counter; }, 0ms, true };
      std::this_thread::sleep_for( 10ms );
    }
    CHECK( counter == 0 );
  }
}

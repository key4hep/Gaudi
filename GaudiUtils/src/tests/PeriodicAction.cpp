/***********************************************************************************\
* (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <string_view>
#include <vector>

using namespace std::chrono_literals;
using Gaudi::Utils::PeriodicAction;

namespace {
  /// mirrors GaudiTesting.platform_matches(): this test's real-time expectations don't hold up
  /// under sanitizer instrumentation slowdown, so skip it there rather than chase flaky margins
  bool running_under_sanitizer() {
    const char* tag = std::getenv( "BINARY_TAG" );
    if ( !tag ) return false;
    std::string_view sv{ tag };
    for ( std::string_view s : { "asan", "lsan", "tsan", "ubsan" } ) {
      if ( sv.find( s ) != std::string_view::npos ) return true;
    }
    return false;
  }

  /// Records when the action runs, so checks can wait for calls instead of racing a fixed sleep.
  class Recorder {
  public:
    void operator()() {
      {
        std::scoped_lock lock{ m_mutex };
        m_calls.push_back( PeriodicAction::clock::now() );
      }
      m_cv.notify_all();
    }
    /// Wait (generously) until at least n calls happened.
    bool wait_for( std::size_t n, std::chrono::seconds timeout = 10s ) {
      std::unique_lock lock{ m_mutex };
      return m_cv.wait_for( lock, timeout, [&] { return m_calls.size() >= n; } );
    }
    std::vector<PeriodicAction::time_point> calls() {
      std::scoped_lock lock{ m_mutex };
      return m_calls;
    }

  private:
    std::mutex                              m_mutex;
    std::condition_variable                 m_cv;
    std::vector<PeriodicAction::time_point> m_calls;
  };

  /// Calls may be late under load but never early: call k happens at least offsets[k] after t0.
  void check_not_early( const std::vector<PeriodicAction::time_point>& calls, PeriodicAction::time_point t0,
                        const std::vector<std::chrono::milliseconds>& offsets ) {
    REQUIRE( calls.size() >= offsets.size() );
    for ( std::size_t k = 0; k < offsets.size(); ++k ) {
      INFO( "call " << k );
      CHECK( calls[k] - t0 >= offsets[k] );
    }
  }
} // namespace

TEST_CASE( "PeriodicAction" ) {
  if ( running_under_sanitizer() ) {
    WARN( "Skipping timing-sensitive PeriodicAction checks: unreliable under sanitizer instrumentation" );
    return;
  }
  // The checks below only assume that calls are never early. How late they are depends on the
  // load of the machine, so the tests wait for the calls they need rather than counting the calls
  // made within a fixed sleep.

  SECTION( "default" ) {
    Recorder   rec;
    const auto t0 = PeriodicAction::clock::now();
    {
      PeriodicAction periodic{ std::ref( rec ), 100ms };
      REQUIRE( rec.wait_for( 2 ) );
    }
    check_not_early( rec.calls(), t0, { 100ms, 200ms } );
  }

  SECTION( "manual start" ) {
    Recorder rec;
    {
      PeriodicAction periodic{ std::ref( rec ), 100ms, false };
      std::this_thread::sleep_for( 100ms );
      CHECK( rec.calls().empty() );
      const auto t0 = PeriodicAction::clock::now();
      periodic.start();
      REQUIRE( rec.wait_for( 2 ) );
      periodic.stop();
      const auto n = rec.calls().size();
      std::this_thread::sleep_for( 100ms );
      CHECK( rec.calls().size() == n ); // no calls after stop
      check_not_early( rec.calls(), t0, { 100ms, 200ms } );
    }
  }

  SECTION( "slow action" ) {
    // ensure that intervals do not depend on speed of action,
    // i.e. one call every 100ms even if the action takes 70ms
    Recorder   rec;
    const auto t0 = PeriodicAction::clock::now();
    {
      PeriodicAction periodic{ [&rec]() {
                                rec();
                                std::this_thread::sleep_for( 70ms );
                              },
                               100ms };
      REQUIRE( rec.wait_for( 4 ) );
    }
    const auto calls = rec.calls();
    check_not_early( calls, t0, { 100ms, 200ms, 300ms, 400ms } );
    // if the action time added up the calls would be 170ms apart; allow plenty of jitter around 100ms
    CHECK( ( calls[3] - calls[0] ) / 3 < 150ms );
  }

  SECTION( "skipping invocations" ) {
    // if the action happens to take more than the allocated time we
    // skip the next invocation(s) to make sure we call it only on
    // a multiple of the interval
    Recorder   rec;
    const auto t0 = PeriodicAction::clock::now();
    {
      // here we have one invocation at t >= 100ms, 300ms and 500ms (we skip that at 200ms and 400ms)
      PeriodicAction periodic{ [&rec]() {
                                rec();
                                std::this_thread::sleep_for( 150ms );
                              },
                               100ms };
      REQUIRE( rec.wait_for( 3 ) );
    }
    check_not_early( rec.calls(), t0, { 100ms, 300ms, 500ms } );
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

  SECTION( "repetitions" ) {
    Recorder rec;
    {
      PeriodicAction periodic{ std::ref( rec ), 2ms, true, 3 };
      REQUIRE( rec.wait_for( 3 ) );
      std::this_thread::sleep_for( 20ms );
    }
    CHECK( rec.calls().size() == 3 );
  }
}

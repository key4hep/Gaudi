/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators.h>
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

using namespace Gaudi::Accumulators;

namespace {
  struct Algo;
}
template <>
class CommonMessaging<Algo> : public CommonMessagingBase {
  void create_msgStream() const override { m_msgStream.reset( new MsgStream( nullptr, "TestAlgo" ) ); }
};

namespace {
  // Little helper for using automatic nlohmann conversion mechanism
  template <typename T>
  nlohmann::json toJSON( T const& t ) {
    nlohmann::json j = t;
    return t;
  }

  // Mock code for the test
  struct MonitoringHub : Gaudi::Monitoring::Hub {};
  struct ServiceLocator {
    MonitoringHub& monitoringHub() { return m_monitHub; }
    MonitoringHub  m_monitHub{};
  };
  struct Algo : CommonMessaging<Algo> {
    ServiceLocator* serviceLocator() { return &m_serviceLocator; }
    std::string     name() const { return ""; }
    ServiceLocator  m_serviceLocator{};
  };

  template <typename C>
  struct helper;

  template <typename C>
  struct helper_base {
    using counter_t = C;
    counter_t c;

    void check_data() const {
      nlohmann::json j = toJSON( c );
      CHECK( j["type"].get<std::string>() == c.typeString );
      CHECK( j["empty"].get<bool>() == !c.toBePrinted() );
      static_cast<const helper<C>*>( this )->check_details( j, j["empty"].get<bool>() );
    }
    void check_round_trip() const {
      auto j  = toJSON( c );
      auto c2 = c.fromJSON( j );
      auto j2 = toJSON( c2 );
      CHECK( j == j2 );
    }
    void check() {
      check_data();
      check_round_trip();
      static_cast<helper<C>*>( this )->fill();
      check_data();
      check_round_trip();
    }
  };

  template <>
  struct helper<Counter<>> : helper_base<Counter<>> {
    void fill() { c += 10; }
    void check_details( const nlohmann::json& j, bool empty ) const {
      unsigned long expected = empty ? 0 : 10;
      CHECK( j["nEntries"].get<unsigned long>() == expected );
    }
  };
  template <>
  struct helper<AveragingCounter<>> : helper_base<AveragingCounter<>> {
    void fill() {
      c += 10;
      c += 20;
      c += 30;
    }
    void check_details( const nlohmann::json& j, bool empty ) const {
      if ( empty ) {
        CHECK( j["nEntries"].get<unsigned long>() == 0 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
      } else {
        CHECK( j["nEntries"].get<unsigned long>() == 3 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinRel( 60, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinRel( 20, 1.e-4 ) );
      }
    }
  };
  template <>
  struct helper<SigmaCounter<>> : helper_base<SigmaCounter<>> {
    void fill() {
      c += 10;
      c += 20;
      c += 30;
    }
    void check_details( const nlohmann::json& j, bool empty ) const {
      if ( empty ) {
        CHECK( j["nEntries"].get<unsigned long>() == 0 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["sum2"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["standard_deviation"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
      } else {
        CHECK( j["nEntries"].get<unsigned long>() == 3 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinRel( 60, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinRel( 20, 1.e-4 ) );
        REQUIRE_THAT( j["sum2"].get<double>(), Catch::Matchers::WithinRel( 1400, 1.e-4 ) );
        REQUIRE_THAT( j["standard_deviation"].get<double>(), Catch::Matchers::WithinRel( 8.164966, 1.e-4 ) );
      }
    }
  };
  template <>
  struct helper<StatCounter<>> : helper_base<StatCounter<>> {
    void fill() {
      c += 10;
      c += 20;
      c += 30;
    }
    void check_details( const nlohmann::json& j, bool empty ) const {
      if ( empty ) {
        CHECK( j["nEntries"].get<unsigned long>() == 0 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["sum2"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["standard_deviation"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        // REQUIRE_THAT( j["min"].get<double>(), Catch::Matchers::WithinAbs(0, 1.e-4 ) );
        // REQUIRE_THAT( j["max"].get<double>(), Catch::Matchers::WithinAbs(0, 1.e-4 ) );
      } else {
        CHECK( j["nEntries"].get<unsigned long>() == 3 );
        REQUIRE_THAT( j["sum"].get<double>(), Catch::Matchers::WithinRel( 60, 1.e-4 ) );
        REQUIRE_THAT( j["mean"].get<double>(), Catch::Matchers::WithinRel( 20, 1.e-4 ) );
        REQUIRE_THAT( j["sum2"].get<double>(), Catch::Matchers::WithinRel( 1400, 1.e-4 ) );
        REQUIRE_THAT( j["standard_deviation"].get<double>(), Catch::Matchers::WithinRel( 8.164966, 1.e-4 ) );
        REQUIRE_THAT( j["min"].get<double>(), Catch::Matchers::WithinRel( 10, 1.e-4 ) );
        REQUIRE_THAT( j["max"].get<double>(), Catch::Matchers::WithinRel( 30, 1.e-4 ) );
      }
    }
  };
  template <>
  struct helper<BinomialCounter<>> : helper_base<BinomialCounter<>> {
    void fill() {
      c += true;
      c += false;
      c += true;
      c += { 3, 7 };
    }
    void check_details( const nlohmann::json& j, bool empty ) const {
      if ( empty ) {
        CHECK( j["nEntries"].get<unsigned long>() == 0 );
        REQUIRE_THAT( j["nTrueEntries"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        REQUIRE_THAT( j["nFalseEntries"].get<double>(), Catch::Matchers::WithinAbs( 0, 1.e-4 ) );
        // REQUIRE_THAT( j["efficiency"].get<double>(), Catch::Matchers::WithinAbs(0, 1.e-4 ) );
        // REQUIRE_THAT( j["efficiencyErr"].get<double>(), Catch::Matchers::WithinAbs(0, 1.e-4 ) );
      } else {
        CHECK( j["nEntries"].get<unsigned long>() == 10 );
        REQUIRE_THAT( j["nTrueEntries"].get<double>(), Catch::Matchers::WithinRel( 5, 1.e-4 ) );
        REQUIRE_THAT( j["nFalseEntries"].get<double>(), Catch::Matchers::WithinRel( 5, 1.e-4 ) );
        REQUIRE_THAT( j["efficiency"].get<double>(), Catch::Matchers::WithinRel( 0.5, 1.e-4 ) );
        REQUIRE_THAT( j["efficiencyErr"].get<double>(), Catch::Matchers::WithinRel( 0.158114, 1.e-4 ) );
      }
    }
  };
  template <MSG::Level level>
  struct helper<MsgCounter<level>> {
    Algo alg;

    MsgCounter<level> c{ &alg, "my message" };

    void fill() {
      ++c;
      ++c;
      ++c;
    }

    void check_data() const {
      nlohmann::json j = toJSON( c );
      CHECK( j["type"].get<std::string>() == c.typeString );
      CHECK( j["empty"].get<bool>() == !c.toBePrinted() );
      CHECK( j["level"].get<MSG::Level>() == MSG::DEBUG );
      CHECK( j["msg"].get<std::string>() == "my message" );
      if ( j["empty"].get<bool>() ) {
        CHECK( j["nEntries"].get<unsigned long>() == 0 );
      } else {
        CHECK( j["nEntries"].get<unsigned long>() == 3 );
      }
    }

    void check_round_trip() const {
      auto j  = toJSON( c );
      auto c2 = c.fromJSON( j );
      auto j2 = toJSON( c2 );
      CHECK( j == j2 );
    }

    void check() {
      check_data();
      check_round_trip();
      fill();
      check_data();
      check_round_trip();
    }
  };
} // namespace

TEMPLATE_TEST_CASE( "counter serialization", "", Counter<>, AveragingCounter<>, SigmaCounter<>, StatCounter<>,
                    BinomialCounter<>, MsgCounter<MSG::DEBUG> ) {
  helper<TestType>{}.check();
}

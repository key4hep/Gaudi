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
#include <map>
#include <optional>
#include <string>
#include <utility>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

namespace {
  /// Mock helpers to be able to invoke counters auto registering constructor.
  Gaudi::Monitoring::Hub localHub;
  struct MiniSink : Gaudi::Monitoring::Hub::Sink {
    MiniSink() { localHub.addSink( this ); }
    ~MiniSink() { localHub.removeSink( this ); }
    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override { entity.emplace( std::move( ent ) ); }
    void removeEntity( Gaudi::Monitoring::Hub::Entity const& ) override {}
    std::optional<Gaudi::Monitoring::Hub::Entity> entity;
  };
  struct Owner {
    Owner*                  serviceLocator() { return this; }
    Gaudi::Monitoring::Hub& monitoringHub() { return localHub; }
    std::string             name() { return "owner"; }
  };
  struct DummyData {
    friend void to_json( nlohmann::json& j, DummyData const& ) {
      j = { { "type", "DummyData" }, { "data", "dummy data" } };
    }
  };

  // Little helper for using automatic nlohmann conversion mechanism
  template <typename T>
  nlohmann::json toJSON( T const& t ) {
    nlohmann::json j = t;
    return t;
  }
} // namespace

TEST_CASE( "Gaudi::Monitoring::Hub::Entity" ) {
  using namespace Gaudi::Accumulators;
  using Gaudi::Monitoring::Hub;

  Counter<>   c;
  Hub::Entity e( "algo", "counter", c.typeString, c );
  DummyData   d;
  Hub::Entity de( "owner", "dummy", "DummyData", d );

  SECTION( "basics" ) {
    CHECK( e.component == "algo" );
    CHECK( e.name == "counter" );
    CHECK( e.type == c.typeString );
    CHECK( e.typeIndex() == std::type_index{ typeid( c ) } );

    CHECK( de.component == "owner" );
    CHECK( de.name == "dummy" );
    CHECK( de.type == "DummyData" );
    CHECK( de.typeIndex() == std::type_index{ typeid( d ) } );
  }
  SECTION( "serialize to JSON" ) {
    CHECK( toJSON( c ) == toJSON( e ) );
    CHECK( toJSON( d ) == toJSON( de ) );
    c += 5;
    CHECK( toJSON( c ) == toJSON( e ) );
  }
  SECTION( "simple merge" ) {
    SECTION( "from Entity" ) {
      CHECK( c.nEntries() == 0 );
      c += 5;
      CHECK( c.nEntries() == 5 );
      Counter<> other;
      other += 10;
      CHECK( other.nEntries() == 10 );
      Hub::Entity other_entity( "", "", other.typeString, other );
      e.mergeAndReset( other_entity );
      CHECK( c.nEntries() == 15 );
    }
    SECTION( "from JSON" ) {
      REQUIRE( e.canMergeFromJSON() );
      CHECK( c.nEntries() == 0 );
      c += 5;
      CHECK( c.nEntries() == 5 );
      e.mergeAndReset( nlohmann::json{ { "nEntries", 10 } } );
      CHECK( c.nEntries() == 15 );
    }
    SECTION( "self registered" ) {
      MiniSink  sink;
      Owner     owner;
      Counter<> owned( &owner, "counter" );
      REQUIRE( sink.entity.has_value() );
      CHECK( sink.entity->canMergeFromJSON() );
      CHECK( owned.nEntries() == 0 );
      owned += 5;
      CHECK( owned.nEntries() == 5 );
      sink.entity->mergeAndReset( nlohmann::json{ { "nEntries", 10 } } );
      CHECK( owned.nEntries() == 15 );
    }
  }
  SECTION( "no merge" ) {
    SECTION( "from Entity" ) {
      DummyData   other;
      Hub::Entity other_entity( "", "", de.type, other );
      de.mergeAndReset( other_entity );
      // we just check there is no error
    }
    SECTION( "from JSON" ) {
      CHECK_FALSE( de.canMergeFromJSON() );
      de.mergeAndReset( nlohmann::json{} ); // can be invoked, but still no-op
    }
    SECTION( "invalid data" ) {
      CHECK_THROWS( e.mergeAndReset( de ) );
      CHECK_THROWS( e.mergeAndReset( nlohmann::json{} ) );
    }
  }
}

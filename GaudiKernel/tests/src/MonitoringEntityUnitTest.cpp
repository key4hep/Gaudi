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
#include <catch2/catch.hpp>

TEST_CASE( "Gaudi::Monitoring::Hub::Entity" ) {
  using namespace Gaudi::Accumulators;
  using Gaudi::Monitoring::Hub;

  Counter<>   c;
  Hub::Entity e( "algo", "counter", c.typeString, c );

  struct DummyData {
    void           reset() {}
    nlohmann::json toJSON() const { return { { "type", "DummyData" }, { "data", "dummy data" } }; }
  };
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
    CHECK( c.toJSON() == e.toJSON() );
    CHECK( d.toJSON() == de.toJSON() );
    c += 5;
    CHECK( c.toJSON() == e.toJSON() );
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

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
#include <GaudiKernel/StdArrayAsProperty.h>
#include <GaudiKernel/StringKey.h>
#include <GaudiKernel/VectorMap.h>
#include <GaudiKernel/VectorsAsProperty.h>

#define BOOST_TEST_MODULE Parsers_test
#include <boost/test/included/unit_test.hpp>

#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/ToStream.h>

using namespace Gaudi::Parsers;

BOOST_AUTO_TEST_CASE( test_StringGrammar ) {
  std::string result;
  BOOST_CHECK( parse( result, "'Hello \\ \\' world'" ) );
  BOOST_CHECK_EQUAL( result, "Hello \\ ' world" );
  BOOST_CHECK( parse( result, "string \"with\" quotes" ) );
  BOOST_CHECK_EQUAL( result, "string \"with\" quotes" );
}

BOOST_AUTO_TEST_CASE( test_IntGrammar ) {
  {
    int result;
    BOOST_CHECK( parse( result, "1000" ) );
    BOOST_CHECK_EQUAL( result, 1000 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "1000" ) );
    BOOST_CHECK_EQUAL( result, 1000 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "1L" ) );
    BOOST_CHECK_EQUAL( result, 1 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "100L" ) );
    BOOST_CHECK_EQUAL( result, 100 );
  }
}

BOOST_AUTO_TEST_CASE( test_CharGrammar ) {
  char result;
  BOOST_CHECK( parse( result, "'a'" ) );
  BOOST_CHECK_EQUAL( result, 'a' );
}

BOOST_AUTO_TEST_CASE( test_BoolGrammar ) {
  bool result;
  BOOST_CHECK( parse( result, "True" ) );
  BOOST_CHECK( result );
  BOOST_CHECK( parse( result, "False" ) );
  BOOST_CHECK( !result );
}

BOOST_AUTO_TEST_CASE( test_RealGrammar ) {
  {
    double result;
    BOOST_CHECK( parse( result, "1.5E2" ) );
    BOOST_CHECK_CLOSE( result, 150, 1.e-4 );
  }

  {
    float result;
    BOOST_CHECK( parse( result, "1.5E2" ) );
    BOOST_CHECK_CLOSE( result, 150, 1.e-4 );
  }

  {
    // test bug GAUDI-1121
    float result;
    BOOST_CHECK( parse( result, "-10000000000.0" ) );
    BOOST_CHECK_CLOSE( result, -1.E10, 1.e-4 );
  }
}

BOOST_AUTO_TEST_CASE( test_VectorGramar ) {
  {
    std::vector<int> result;
    BOOST_CHECK( parse( result, "[1, 2,  3] // Test Comments" ) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK_EQUAL( result[0], 1 );
    BOOST_CHECK_EQUAL( result[1], 2 );
    BOOST_CHECK_EQUAL( result[2], 3 );
  }

  {
    // trailing comma
    std::vector<int> result;
    BOOST_CHECK( parse( result, "[1, 2, 3,]" ) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK_EQUAL( result[0], 1 );
    BOOST_CHECK_EQUAL( result[1], 2 );
    BOOST_CHECK_EQUAL( result[2], 3 );
  }

  {
    std::vector<std::vector<double>> result;
    BOOST_CHECK( parse( result, "[[1, 2]/* Test comments */,[3]]" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK_CLOSE( result[0][0], 1, 1.e-4 );
    BOOST_CHECK_CLOSE( result[0][1], 2, 1.e-4 );
    BOOST_CHECK_CLOSE( result[1][0], 3, 1.e-4 );
  }

  {
    std::vector<std::vector<int>> result;
    BOOST_CHECK( parse( result, "[[1, 2]/* Test comments */,[3]]" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK_EQUAL( result[0][0], 1 );
    BOOST_CHECK_EQUAL( result[0][1], 2 );
    BOOST_CHECK_EQUAL( result[1][0], 3 );
  }

  {
    std::vector<double> result;
    BOOST_CHECK( parse( result, "[1.1, 2.2 ]" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK_CLOSE( result[0], 1.1, 1.e-4 );
    BOOST_CHECK_CLOSE( result[1], 2.2, 1.e-4 );
  }
  {
    std::set<double> result;
    BOOST_CHECK( parse( result, "[1.1, 2.2 ]" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    auto it = result.begin();
    BOOST_CHECK_CLOSE( *it, 1.1, 1.e-4 );
    BOOST_CHECK_CLOSE( *( ++it ), 2.2, 1.e-4 );
  }

  {
    std::list<double> result;
    BOOST_CHECK( parse( result, "[1.1, 2.2 ]" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    auto it = result.begin();
    BOOST_CHECK_CLOSE( *it, 1.1, 1.e-4 );
    BOOST_CHECK_CLOSE( *( ++it ), 2.2, 1.e-4 );
  }
}

BOOST_AUTO_TEST_CASE( test_PairGramar ) {
  std::pair<double, double> result;
  BOOST_CHECK( parse( result, "(10.1, 10)" ) );
  BOOST_CHECK_CLOSE( result.first, 10.1, 1.e-4 );
  BOOST_CHECK_CLOSE( result.second, 10, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_MapGramar ) {
  {
    std::map<std::string, int> result;
    BOOST_CHECK( parse( result, "{'key':10, 'key1'=20}" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK_EQUAL( result["key"], 10 );
    BOOST_CHECK_EQUAL( result["key1"], 20 );
  }

  {
    // trailing comma and new-lines
    std::map<std::string, int> result;
    BOOST_CHECK( parse( result, R"(
      {
        'key':10,
        'key1':20,
      }
    )" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK_EQUAL( result["key"], 10 );
    BOOST_CHECK_EQUAL( result["key1"], 20 );
  }

  {
    std::map<std::string, std::vector<double>> result;
    BOOST_CHECK( parse( result, "{'key':[10.0,20.0]}" ) );
    BOOST_CHECK_EQUAL( result.size(), 1 );
    BOOST_CHECK_EQUAL( result["key"].size(), 2 );
    BOOST_CHECK_CLOSE( result["key"][0], 10, 1.e-4 );
  }

  {
    std::map<std::string, std::pair<double, double>> result;
    BOOST_CHECK( parse( result, "{'key':(10.0,20.0)}" ) );
    BOOST_CHECK_EQUAL( result.size(), 1 );
    BOOST_CHECK_CLOSE( result["key"].first, 10, 1.e-4 );
    BOOST_CHECK_CLOSE( result["key"].second, 20, 1.e-4 );
  }

  {
    std::map<std::string, std::pair<int, int>> result;
    BOOST_CHECK( parse( result, "{'key':(10,20)}" ) );
    BOOST_CHECK_EQUAL( result.size(), 1 );
    BOOST_CHECK_EQUAL( result["key"].first, 10 );
    BOOST_CHECK_EQUAL( result["key"].second, 20 );
  }

  {
    GaudiUtils::VectorMap<Gaudi::StringKey, double> result;
    BOOST_CHECK( parse( result, "{'key':10, 'key1'=20}" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    Gaudi::StringKey key = std::string( "key" );
    BOOST_CHECK_CLOSE( result.at( key ), 10, 1.e-4 );
    key = std::string( "key1" );
    BOOST_CHECK_CLOSE( result.at( key ), 20, 1.e-4 );
  }
}

BOOST_AUTO_TEST_CASE( test_Pnt3DTypes ) {
  Gaudi::XYZPoint result;
  BOOST_CHECK( parse( result, "(px:10.0, py:11.0, pZ:12.0)" ) );
  BOOST_CHECK_CLOSE( result.X(), 10.0, 1.e-4 );
  BOOST_CHECK_CLOSE( result.Y(), 11.0, 1.e-4 );
  BOOST_CHECK_CLOSE( result.Z(), 12.0, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_Pnt4DTypes ) {
  Gaudi::LorentzVector result;
  BOOST_CHECK( parse( result, "(px:10.0, py:11.0, pZ:12.0;100.0)" ) );
  BOOST_CHECK_CLOSE( result.X(), 10.0, 1.e-4 );
  BOOST_CHECK_CLOSE( result.Y(), 11.0, 1.e-4 );
  BOOST_CHECK_CLOSE( result.Z(), 12.0, 1.e-4 );
  BOOST_CHECK_CLOSE( result.T(), 100.0, 1.e-4 );
}

BOOST_AUTO_TEST_CASE( test_HistoTypes ) {
  {
    Gaudi::Histo1DDef result;
    BOOST_CHECK( parse( result, "('test', 1.0,2.0, 100)" ) );
    BOOST_CHECK_EQUAL( result.title(), "test" );
    BOOST_CHECK_CLOSE( result.lowEdge(), 1.0, 1.e-4 );
    BOOST_CHECK_CLOSE( result.highEdge(), 2.0, 1.e-4 );
    BOOST_CHECK_EQUAL( result.bins(), 100 );
  }

  {
    Gaudi::Histo1DDef result;
    BOOST_CHECK( parse( result, "(1.0,2.0, 'test', 100)" ) );
    BOOST_CHECK_EQUAL( result.title(), "test" );
    BOOST_CHECK_CLOSE( result.lowEdge(), 1.0, 1.e-4 );
    BOOST_CHECK_CLOSE( result.highEdge(), 2.0, 1.e-4 );
    BOOST_CHECK_EQUAL( result.bins(), 100 );
  }
}

BOOST_AUTO_TEST_CASE( test_Tuples ) {
  {
    std::tuple<int> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(2)" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
  }
  {
    // see https://gitlab.cern.ch/gaudi/Gaudi/-/issues/21
    std::tuple<int> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(2,)" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
  }
  {
    std::tuple<int, int> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(2 , 3)" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), 3 );
  }
  {
    std::tuple<int, bool> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(2,True)" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), true );

    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "( 2 , True , )" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), true );
  }
  {
    std::tuple<int, std::string> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(2, 'hello')" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), "hello" );

    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(  2 , \"hello\" )" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), "hello" );

    BOOST_REQUIRE( Gaudi::Parsers::parse_( result, "(  2 , \"hello\",  )" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), "hello" );
  }

  {
    std::tuple<int, std::string, bool> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse( result, "(2, 'hello', True)" ) );
    BOOST_CHECK_EQUAL( std::get<0>( result ), 2 );
    BOOST_CHECK_EQUAL( std::get<1>( result ), "hello" );
    BOOST_CHECK_EQUAL( std::get<2>( result ), true );
  }

  {
    std::vector<std::tuple<int, std::string, bool>> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse( result, "[(2, 'hello', True), (3, 'world', False)]" ) );
    BOOST_REQUIRE_EQUAL( result.size(), 2 );
    BOOST_CHECK_EQUAL( std::get<0>( result[0] ), 2 );
    BOOST_CHECK_EQUAL( std::get<0>( result[1] ), 3 );
    BOOST_CHECK_EQUAL( std::get<1>( result[0] ), "hello" );
    BOOST_CHECK_EQUAL( std::get<1>( result[1] ), "world" );
    BOOST_CHECK( std::get<2>( result[0] ) );
    BOOST_CHECK( !std::get<2>( result[1] ) );
  }

  {
    std::tuple<int, std::string> result;
    BOOST_CHECK( !Gaudi::Parsers::parse( result, "(2, 'hello', 1.0)" ) );
  }
}

BOOST_AUTO_TEST_CASE( test_Array ) {
  std::array<int, 2> result;
  BOOST_REQUIRE( Gaudi::Parsers::parse( result, "(1, 2)" ) );
  BOOST_CHECK_EQUAL( result[0], 1 );
  BOOST_CHECK_EQUAL( result[1], 2 );
}

BOOST_AUTO_TEST_CASE( test_Set ) {
  {
    std::unordered_set<int> result;
    BOOST_CHECK( parse( result, "{}" ) ); // accepted but not valid Python syntax
    BOOST_CHECK( result.empty() );
  }

  {
    std::unordered_set<int> result;
    BOOST_CHECK( parse( result, "set()" ) );
    BOOST_CHECK( result.empty() );
  }

  {
    std::unordered_set<int> result;
    BOOST_CHECK( parse( result, "{1, 2, 3} // Test comment" ) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK( result == std::unordered_set<int>( { 1, 2, 3 } ) );
  }

  {
    std::unordered_set<int> result;
    BOOST_CHECK( parse( result, "{1, 2, 3,}" ) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK( result == std::unordered_set<int>( { 1, 2, 3 } ) );
  }

  {
    std::unordered_set<double> result;
    BOOST_CHECK( parse( result, "{1.1, 2.2, 3.3}" ) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    const double sum = std::accumulate( result.begin(), result.end(), 0.0 );
    BOOST_CHECK_CLOSE( sum, 6.6, 1.e-4 );
  }

  {
    std::unordered_set<std::string> result;
    BOOST_CHECK( parse( result, "{'a', 'b'}" ) );
    BOOST_CHECK_EQUAL( result.size(), 2 );
    BOOST_CHECK( result == std::unordered_set<std::string>( { "a", "b" } ) );
  }
}

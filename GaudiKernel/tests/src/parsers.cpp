#include "GaudiKernel/StdArrayAsProperty.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/VectorsAsProperty.h"
#include <boost/test/minimal.hpp>

#include "GaudiKernel/ToStream.h"
#include <Gaudi/Parsers/Factory.h>

using namespace Gaudi::Parsers;

int test_main( int /*argc*/, char** /*argv*/ ) // note the name!
{
  //==============================================================================
  // StringGrammar
  //==============================================================================
  {
    std::string result;
    BOOST_CHECK( parse( result, "'Hello \\ \\' world'" ) );
    BOOST_CHECK( result == "Hello \\ ' world" );
  }
  //==============================================================================
  // IntGrammar
  //==============================================================================
  {
    int result;
    BOOST_CHECK( parse( result, "1000" ) );
    BOOST_CHECK( result == 1000 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "1000" ) );
    BOOST_CHECK( result == 1000 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "1L" ) );
    BOOST_CHECK( result == 1 );
  }

  {
    long result;
    BOOST_CHECK( parse( result, "100L" ) );
    BOOST_CHECK( result == 100 );
  }
  //==============================================================================
  // CharGrammar
  //==============================================================================
  {
    char result;
    BOOST_CHECK( parse( result, "'a'" ) );
    BOOST_CHECK( result == 'a' );
  }
  //==============================================================================
  // BoolGrammar
  //==============================================================================
  {
    bool result;
    BOOST_CHECK( parse( result, "True" ) );
    BOOST_CHECK( result );
    BOOST_CHECK( parse( result, "False" ) );
    BOOST_CHECK( !result );
  }
  //==============================================================================
  // RealGrammar
  //==============================================================================
  {
    double result;
    BOOST_CHECK( parse( result, "1.5E2" ) );
    BOOST_CHECK( result == 150 );
  }

  {
    float result;
    BOOST_CHECK( parse( result, "1.5E2" ) );
    BOOST_CHECK( result == 150 );
  }

  {
    // test bug GAUDI-1121
    float result;
    BOOST_CHECK( parse( result, "-10000000000.0" ) );
    BOOST_CHECK( result == -1.E10 );
  }
  //==============================================================================
  // VectorGramar
  //==============================================================================
  {
    std::vector<int> result;
    BOOST_CHECK( parse( result, "[1, 2,  3] // Test Comments" ) );
    BOOST_CHECK( result.size() == 3 );
    BOOST_CHECK( result[0] == 1 );
    BOOST_CHECK( result[1] == 2 );
    BOOST_CHECK( result[2] == 3 );
  }

  {
    std::vector<std::vector<double>> result;
    BOOST_CHECK( parse( result, "[[1, 2]/* Test comments */,[3]]" ) );
    BOOST_CHECK( result.size() == 2 );
    BOOST_CHECK( result[0][0] == 1 );
    BOOST_CHECK( result[0][1] == 2 );
    BOOST_CHECK( result[1][0] == 3 );
  }

  {
    std::vector<double> result;
    BOOST_CHECK( parse( result, "[1.1, 2.2 ]" ) );
    BOOST_CHECK( result.size() == 2 );
    BOOST_CHECK( result[0] == 1.1 );
    BOOST_CHECK( result[1] == 2.2 );
  }

  /*{
      std::set<double> result;
      BOOST_CHECK(parse(result, "[1.1, 2.2 ]"));
      BOOST_CHECK(result.size()==2);
      BOOST_CHECK(result.count(1.1)==1);
      BOOST_CHECK(result.count(2.2)==1);
  }

  {
    std::list<double> result;
    BOOST_CHECK(parse(result, "[1.1, 2.2 ]"));
    BOOST_CHECK(result.size()==2);
    BOOST_CHECK(result.front()==1.1);
    BOOST_CHECK(result.back()==2.2);
  }*/
  //==============================================================================
  // PairGramar
  //==============================================================================
  {

    std::pair<double, double> result;
    BOOST_CHECK( parse( result, "(10.1, 10)" ) );
    BOOST_CHECK( result.first == 10.1 );
    BOOST_CHECK( result.second == 10 );
  }

  //==============================================================================
  // MapGramar
  //==============================================================================

  {
    std::map<std::string, int> result;
    BOOST_CHECK( parse( result, "{'key':10, 'key1'=20}" ) );
    BOOST_CHECK( result.size() == 2 );
    BOOST_CHECK( result["key"] == 10 );
    BOOST_CHECK( result["key1"] == 20 );
  }

  {
    std::map<std::string, std::vector<double>> result;
    BOOST_CHECK( parse( result, "{'key':[10.0,20.0]}" ) );
    BOOST_CHECK( result.size() == 1 );
    BOOST_CHECK( result["key"].size() == 2 );
    BOOST_CHECK( result["key"][0] == 10 );
  }

  {
    GaudiUtils::VectorMap<Gaudi::StringKey, double> result;
    BOOST_CHECK( parse( result, "{'key':10, 'key1'=20}" ) );
    BOOST_CHECK( result.size() == 2 );
    Gaudi::StringKey key = std::string( "key" );
    BOOST_CHECK( result.at( key ) == 10 );
    key = std::string( "key1" );
    BOOST_CHECK( result.at( key ) == 20 );
  }
  //==============================================================================
  // Pnt3DTypes
  //==============================================================================
  {
    Gaudi::XYZPoint result;
    BOOST_CHECK( parse( result, "(px:10.0, py:11.0, pZ:12.0)" ) );
    BOOST_CHECK( result.X() == 10.0 );
    BOOST_CHECK( result.Y() == 11.0 );
    BOOST_CHECK( result.Z() == 12.0 );
  }
  //==============================================================================
  // Pnt4DTypes
  //==============================================================================
  {
    Gaudi::LorentzVector result;
    BOOST_CHECK( parse( result, "(px:10.0, py:11.0, pZ:12.0;100.0)" ) );
    BOOST_CHECK( result.X() == 10.0 );
    BOOST_CHECK( result.Y() == 11.0 );
    BOOST_CHECK( result.Z() == 12.0 );
    BOOST_CHECK( result.T() == 100.0 );
  }

  //==============================================================================
  // Pnt4DTypes
  //==============================================================================
  {
    Gaudi::Histo1DDef result;
    BOOST_CHECK( parse( result, "('test', 1.0,2.0, 100)" ) );
    BOOST_CHECK( result.title() == "test" );
    BOOST_CHECK( result.lowEdge() == 1.0 );
    BOOST_CHECK( result.highEdge() == 2.0 );
    BOOST_CHECK( result.bins() == 100 );
  }

  {
    Gaudi::Histo1DDef result;
    BOOST_CHECK( parse( result, "(1.0,2.0, 'test', 100)" ) );
    BOOST_CHECK( result.title() == "test" );
    BOOST_CHECK( result.lowEdge() == 1.0 );
    BOOST_CHECK( result.highEdge() == 2.0 );
    BOOST_CHECK( result.bins() == 100 );
  }
  //==============================================================================
  // Tuples
  //==============================================================================
  {
    std::tuple<int> result;
    BOOST_REQUIRE( Gaudi::Parsers::sparse<std::tuple<int>>::parse_( result, "(2)" ) );
    BOOST_CHECK( std::get<0>( result ) == 2 );
  }
  #define COMMA ,
  {
    std::tuple<int, std::string> result;
    BOOST_REQUIRE( Gaudi::Parsers::sparse<std::tuple<int COMMA std::string>>::parse_( result, "(2, 'hello')" ) );
    BOOST_CHECK( std::get<0>( result ) == 2 );
    BOOST_CHECK( std::get<1>( result ) == "hello" );
  }
  {
    std::tuple<int, std::string, bool> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse( result, "(2, 'hello', True)" ) );
    BOOST_CHECK( std::get<0>( result ) == 2 );
    BOOST_CHECK( std::get<1>( result ) == "hello" );
    BOOST_CHECK( std::get<2>( result ) == true );
  }

  {
    std::vector<std::tuple<int, std::string, bool>> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse( result, "[(2, 'hello', True), (3, 'world', False)]" ) );
    BOOST_REQUIRE( result.size() == 2 );
    BOOST_CHECK( std::get<0>( result[0] ) == 2 );
    BOOST_CHECK( std::get<0>( result[1] ) == 3 );
    BOOST_CHECK( std::get<1>( result[0] ) == "hello" );
    BOOST_CHECK( std::get<1>( result[1] ) == "world" );
    BOOST_CHECK( std::get<2>( result[0] ) );
    BOOST_CHECK( !std::get<2>( result[1] ) );
  }

  {
    std::tuple<int, std::string> result;
    BOOST_CHECK( !Gaudi::Parsers::parse( result, "(2, 'hello', 1.0)" ) );
  }

  //==============================================================================
  // Array
  //==============================================================================
  {
    std::array<int, 2> result;
    BOOST_REQUIRE( Gaudi::Parsers::parse( result, "(1, 2)" ) );
    BOOST_CHECK( result[0] == 1 );
    BOOST_CHECK( result[1] == 2 );
  }
  //==============================================================================
  return 0;
}

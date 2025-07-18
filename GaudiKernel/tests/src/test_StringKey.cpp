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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_StringKey
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/StringKey.h>

#include <map>
#include <unordered_map>

void func( const Gaudi::StringKey& ) {}

BOOST_AUTO_TEST_CASE( construct ) {
  {
    const std::string s( "foo" );
    Gaudi::StringKey  sk( s );
    BOOST_CHECK( sk.str() == "foo" );
  }
  {
    Gaudi::StringKey sk( "foo" );
    BOOST_CHECK( sk.str() == "foo" );
  }
  // https://its.cern.ch/jira/browse/GAUDI-905
  func( "test" );
}

BOOST_AUTO_TEST_CASE( empty ) {
  Gaudi::StringKey sk;
  BOOST_CHECK( sk.empty() );
  BOOST_CHECK( !sk );
  BOOST_CHECK( sk.str().empty() );
}

BOOST_AUTO_TEST_CASE( comparison ) {
  Gaudi::StringKey sk = "foo";
  BOOST_CHECK( sk == "foo" );
  BOOST_CHECK( sk == std::string( "foo" ) );
  BOOST_CHECK( sk == std::string_view( "foo" ) );
  BOOST_CHECK( sk == Gaudi::StringKey( "foo" ) );

  BOOST_CHECK( "foo" == sk );
  BOOST_CHECK( std::string( "foo" ) == sk );
  BOOST_CHECK( std::string_view( "foo" ) == sk );
  BOOST_CHECK( Gaudi::StringKey( "foo" ) == sk );

  BOOST_CHECK( sk != Gaudi::StringKey( "bar" ) );
  BOOST_CHECK( std::string( "bar" ) != sk );
}

BOOST_AUTO_TEST_CASE( hashing ) {
  Gaudi::StringKey sk1 = "foo";
  Gaudi::StringKey sk2 = "foo";
  Gaudi::StringKey sk3 = "bar";
  BOOST_CHECK( sk1.__hash__() == sk2.__hash__() );
  BOOST_CHECK( sk2.__hash__() != sk3.__hash__() );
}

BOOST_AUTO_TEST_CASE( concat ) {
  Gaudi::StringKey sk = Gaudi::StringKey( "foo" ) + "bar";
  BOOST_CHECK( sk.str() == "foobar" );

  sk = Gaudi::StringKey( "foo" ) + std::string( "bar" );
  BOOST_CHECK( sk.str() == "foobar" );
}

BOOST_AUTO_TEST_CASE( cast ) {
  std::string s = Gaudi::StringKey( "foo" );
  BOOST_CHECK( s == "foo" );
}

BOOST_AUTO_TEST_CASE( ordered_map ) {
  std::map<Gaudi::StringKey, int> m = { { "one", 1 }, { "two", 2 } };
  BOOST_CHECK( m["one"] == 1 );
  BOOST_CHECK( m["two"] == 2 );
}

BOOST_AUTO_TEST_CASE( unordered_map ) {
  std::unordered_map<Gaudi::StringKey, int> m = { { "one", 1 }, { "two", 2 } };
  BOOST_CHECK( m["one"] == 1 );
  BOOST_CHECK( m["two"] == 2 );
}

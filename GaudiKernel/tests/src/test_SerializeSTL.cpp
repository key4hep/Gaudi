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
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/Map.h>
#include <GaudiKernel/SerializeSTL.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

#define CHECK_SERIALIZATION( exp, obj )                                                                                \
  {                                                                                                                    \
    std::ostringstream __o;                                                                                            \
    __o << obj;                                                                                                        \
    CHECK( __o.str() == std::string( exp ) );                                                                          \
  }

TEST_CASE( "SerializeSTL vector test", "[SerializeSTL]" ) {
  using namespace GaudiUtils;

  std::vector<int> v_i;
  CHECK_SERIALIZATION( "[]", v_i )
  v_i.push_back( 123 );
  CHECK_SERIALIZATION( "[123]", v_i )
  v_i.push_back( 456 );
  v_i.push_back( 789 );
  v_i.push_back( 10000 );
  CHECK_SERIALIZATION( "[123, 456, 789, 10000]", v_i )

  std::vector<std::string> v_s;
  v_s.push_back( "a" );
  CHECK_SERIALIZATION( "[a]", v_s )
  v_s.push_back( "bc" );
  v_s.push_back( "def" );
  v_s.push_back( "Hello World!" );
  CHECK_SERIALIZATION( "[a, bc, def, Hello World!]", v_s )
}

TEST_CASE( "SerializeSTL list test", "[SerializeSTL]" ) {
  using namespace GaudiUtils;

  std::list<int> l_i;
  CHECK_SERIALIZATION( "[]", l_i )
  l_i.push_back( 123 );
  CHECK_SERIALIZATION( "[123]", l_i )
  l_i.push_back( 456 );
  l_i.push_back( 789 );
  l_i.push_back( 10000 );
  CHECK_SERIALIZATION( "[123, 456, 789, 10000]", l_i )

  std::list<std::string> l_s;
  l_s.push_back( "a" );
  CHECK_SERIALIZATION( "[a]", l_s )
  l_s.push_back( "bc" );
  l_s.push_back( "def" );
  l_s.push_back( "Hello World!" );
  CHECK_SERIALIZATION( "[a, bc, def, Hello World!]", l_s )
}

TEST_CASE( "SerializeSTL pair test", "[SerializeSTL]" ) {
  using namespace GaudiUtils;

  std::pair<int, int> p1( 20, 11 );
  CHECK_SERIALIZATION( "(20, 11)", p1 )

  std::pair<std::string, std::string> p2( "Hello", "World" );
  CHECK_SERIALIZATION( "(Hello, World)", p2 )

  std::pair<std::string, int> p3( "November", 20 );
  CHECK_SERIALIZATION( "(November, 20)", p3 )

  std::list<int> l;
  l.push_back( 1 );
  l.push_back( 2 );
  l.push_back( 3 );
  // <char,list>
  std::pair<char, std::list<int>> p4( 'x', l );
  CHECK_SERIALIZATION( "(x, [1, 2, 3])", p4 )
}

TEST_CASE( "SerializeSTL map test", "[SerializeSTL]" ) {
  using namespace GaudiUtils;

  std::map<int, int> m1;
  CHECK_SERIALIZATION( "{}", m1 )
  m1[1] = 1;
  CHECK_SERIALIZATION( "{1: 1}", m1 )
  m1[2] = 4;
  m1[3] = 9;
  CHECK_SERIALIZATION( "{1: 1, 2: 4, 3: 9}", m1 )

  std::map<std::string, int> m2;
  m2["a"] = 1;
  CHECK_SERIALIZATION( "{a: 1}", m2 )
  m2["b"] = 4;
  m2["c"] = 9;
  CHECK_SERIALIZATION( "{a: 1, b: 4, c: 9}", m2 )
}

TEST_CASE( "SerializeSTL GaudiUtils Map test", "[SerializeSTL]" ) {
  GaudiUtils::Map<std::string, int> m1;
  CHECK_SERIALIZATION( "{}", m1 )
  m1["a"] = 1;
  CHECK_SERIALIZATION( "{a: 1}", m1 )
  m1["b"] = 4;
  m1["c"] = 9;
  CHECK_SERIALIZATION( "{a: 1, b: 4, c: 9}", m1 )
}

TEST_CASE( "SerializeSTL GaudiUtils HashMap test", "[SerializeSTL]" ) {
  GaudiUtils::HashMap<std::string, int> m1;
  CHECK_SERIALIZATION( "{}", m1 )
  m1["a"] = 1;
  CHECK_SERIALIZATION( "{a: 1}", m1 )
  m1["b"] = 4;
  m1["c"] = 9;
  CHECK_SERIALIZATION( "{a: 1, b: 4, c: 9}", m1 )
}

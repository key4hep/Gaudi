/***********************************************************************************\
* (c) Copyright 2025-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_VectorMap
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/SmartRef.h>
#include <GaudiKernel/VectorMap.h>
#include <string>
#include <string_view>

namespace {
  template <typename Compare>
  void check_smartref_pointer_lookup() {
    using Key = SmartRef<DataObject>;

    DataObject                               object;
    const DataObject*                        key = &object;
    GaudiUtils::VectorMap<Key, int, Compare> map;
    map.insert( Key{ key }, 1 );
    const auto& cmap = map;

    BOOST_CHECK( cmap.find( key ) != cmap.end() );
    BOOST_CHECK_EQUAL( cmap.count( key ), 1 );
    BOOST_CHECK( cmap.lower_bound( key ) != cmap.end() );
    BOOST_CHECK( cmap.upper_bound( key ) == cmap.end() );
    BOOST_CHECK( cmap.equal_range( key ).first != cmap.end() );
    BOOST_CHECK_EQUAL( cmap( key ), 1 );
    BOOST_CHECK_EQUAL( cmap[key], 1 );
    BOOST_CHECK_EQUAL( cmap.at( key ), 1 );
    BOOST_CHECK( map.update( key, 2 ) );
    BOOST_CHECK_EQUAL( map.erase( key ), 1 );
  }
} // namespace

BOOST_AUTO_TEST_CASE( heterogeneous ) {
  GaudiUtils::VectorMap<std::string, int, std::less<>> vm1;
  vm1.erase( std::string_view{ "abc" } ); // OK (transparent)
  vm1.find( "abc" );                      // OK

  GaudiUtils::VectorMap<std::string, int, std::less<std::string>> vm2;
  vm2.insert( "abc", 1 );
  const auto& cvm2 = vm2;
  BOOST_CHECK( vm2.find( "abc" ) != vm2.end() );
  BOOST_CHECK_EQUAL( vm2.count( "abc" ), 1 );
  BOOST_CHECK( cvm2.lower_bound( "abc" ) != vm2.end() );
  BOOST_CHECK( vm2.upper_bound( "abc" ) == vm2.end() );
  BOOST_CHECK( vm2.equal_range( "abc" ).first != vm2.end() );
  BOOST_CHECK_EQUAL( vm2( "abc" ), 1 );
  BOOST_CHECK_EQUAL( vm2["abc"], 1 );
  BOOST_CHECK_EQUAL( vm2.at( "abc" ), 1 );
  BOOST_CHECK( vm2.update( "abc", 2 ) );
  BOOST_CHECK_EQUAL( vm2.erase( "abc" ), 1 );
  /* vm2.erase(std::string_view{"abc"}); */ // must NOT compile (hetero disabled)
}

BOOST_AUTO_TEST_CASE( smartref_pointer_lookup ) {
  check_smartref_pointer_lookup<std::less<>>();
  check_smartref_pointer_lookup<std::less<const SmartRef<DataObject>>>();
}

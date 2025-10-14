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
/*
 * Time_test.cpp
 *
 *  Created on: Jan 13, 2011
 *      Author: Marco Clemencic
 */

#include <GaudiKernel/Time.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "Gaudi::Time nanoformat", "[GaudiTime]" ) {
  Gaudi::Time t( 2011, 0, 13, 14, 22, 45, 123000, true );

  CHECK( t.nanoformat() == "000123" );
  CHECK( t.nanoformat( 9 ) == "000123000" );
  CHECK( t.nanoformat( 2, 7 ) == "000123" );
  CHECK( t.nanoformat( 1, 3 ) == "0" );
  CHECK( t.nanoformat( 1, 5 ) == "00012" );
  CHECK( t.nanoformat( 3, 3 ) == "000" );
}

TEST_CASE( "Gaudi::Time format", "[GaudiTime]" ) {
  Gaudi::Time t( 2011, 0, 13, 14, 22, 45, 1230000, true );

  CHECK( t.format( true, "%Y-%m-%d %H:%M:%S" ) == "2011-01-13 14:22:45" );
  CHECK( t.format( true, "%Y-%m-%d %H:%M:%S.%f" ) == "2011-01-13 14:22:45.001" );
  CHECK( t.format( true, "%H:%M:%S %Y/%m/%d " ) == "14:22:45 2011/01/13 " );
}

TEST_CASE( "Gaudi::Time ordering", "[GaudiTime]" ) {
  Gaudi::Time t1( 2011, 0, 13, 14, 22, 45, 1230000, true );
  Gaudi::Time t2( 2011, 0, 13, 14, 22, 45, 1230000, true );
  Gaudi::Time t3( 2011, 0, 13, 14, 22, 46, 1230000, true );

  CHECK( t1 == t2 );
  CHECK( t1 <= t2 );
  CHECK( t1 >= t2 );
  CHECK_FALSE( t1 != t2 );

  CHECK( t2 < t3 );
  CHECK( t2 <= t3 );
  CHECK( t2 != t3 );
  CHECK_FALSE( t2 == t3 );
  CHECK( t3 > t2 );
  CHECK( t3 >= t2 );
}

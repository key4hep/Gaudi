/***********************************************************************************\
* (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/SmartRef.h>
#include <utility>

#if __has_include( <catch2/catch.hpp> )
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "SmartRef comparison" ) {
  SmartRef<DataObject> ref;
  DataObject*          other = nullptr;
  DataObject           data;

  CHECK( ref == other );
  CHECK( other == ref );

  CHECK( ref == const_cast<const DataObject*>( other ) );

  CHECK( ref != &data );
  CHECK( &data != ref );

  SmartRef<DataObject> ref2;
  CHECK( ref == ref2 );
  CHECK( ref2 == ref );
  CHECK( std::as_const( ref ) == ref2 );
  CHECK( std::as_const( ref ) == std::as_const( ref2 ) );
  CHECK( !( ref != ref2 ) );
}

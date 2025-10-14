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
#include <GaudiKernel/AnyDataWrapper.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "AnyDataWrapper simple wrapper test", "[AnyDataObject]" ) {
  auto adw = AnyDataWrapper<int>( 42 );
  CHECK( adw.getData() == 42 );

  int myval = 43;
  {
    auto adw2 = AnyDataWrapper<int>( std::move( myval ) );
    CHECK( adw2.getData() == 43 );
  }
}

TEST_CASE( "AnyDataWrapper vector wrapper test", "[AnyDataObject]" ) {
  using V     = std::vector<int>;
  using VSize = V::size_type;

  SECTION( "Test move of a vector" ) {
    auto  adw = AnyDataWrapper<V>( { 1, 2, 3, 4 } );
    VSize s4( 4 );
    CHECK( ( adw.getData() ).size() == s4 );
  }

  SECTION( "Making sure that the vector is copied" ) {
    V myvector{ 1, 2, 3, 4 };
    CHECK( myvector.size() == static_cast<VSize>( 4 ) );

    auto adw = AnyDataWrapper<V>( std::move( myvector ) );

    CHECK( ( adw.getData() ).size() == static_cast<VSize>( 4 ) );
    CHECK( myvector.size() == static_cast<VSize>( 0 ) );
  }

  SECTION( "Trying with std::unique_ptr" ) {
    auto myvector = std::make_unique<V>( V{ 1, 2, 3, 4 } );
    auto adw      = AnyDataWrapper<decltype( myvector )>( std::move( myvector ) );
    CHECK( ( adw.getData() )->size() == static_cast<VSize>( 4 ) );
  }
}

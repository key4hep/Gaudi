/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_compose
#include <boost/test/unit_test.hpp>

#include <string>

#include "GaudiKernel/compose.h"

const auto f = compose( []( int ) { return 0; }, []( double ) { return 1; }, []( float ) { return 2; },
                        []( const char* ) { return 3; }, []( void* ) { return 4; } );

BOOST_AUTO_TEST_CASE( test_compose ) {
  {
    BOOST_CHECK( f( 0 ) == 0 );
    BOOST_CHECK( f( 1.0 ) == 1 );
    BOOST_CHECK( f( "Hello World" ) == 3 );
  }
}

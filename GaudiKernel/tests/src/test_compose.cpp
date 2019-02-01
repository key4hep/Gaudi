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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_reverse
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>

#include "GaudiKernel/reverse.h"

template <typename R1, typename R2>
void copy( R1&& r1, R2& r2 )
{
  std::copy( begin( r1 ), end( r1 ), begin( r2 ) );
}

BOOST_AUTO_TEST_CASE( test_reverse )
{
  {

    std::array<int, 3> a{0, 1, 2};
    std::array<int, 3> b;
    copy( reverse( a ), b );

    BOOST_CHECK( b[0] == 2 );
    BOOST_CHECK( b[1] == 1 );
    BOOST_CHECK( b[2] == 0 );

    copy( reverse( std::array<int, 3>{10, 11, 12} ), b );

    BOOST_CHECK( b[0] == 12 );
    BOOST_CHECK( b[1] == 11 );
    BOOST_CHECK( b[2] == 10 );

    const std::array<int, 3> ca{20, 21, 22};
    copy( reverse( ca ), b );

    BOOST_CHECK( b[0] == 22 );
    BOOST_CHECK( b[1] == 21 );
    BOOST_CHECK( b[2] == 20 );
  }
}

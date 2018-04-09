#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_apply
#include <boost/test/unit_test.hpp>

#include <array>
#include <tuple>

#include "GaudiKernel/apply.h"

BOOST_AUTO_TEST_CASE( test_apply )
{
  {

    std::array<int, 3> a{1, 2, 3};

    BOOST_CHECK( Gaudi::apply( []( int i, int j, int k ) { return i + j + k; }, a ) == 6 );

    std::tuple<int, double> b{2, 2.1};

    BOOST_CHECK( Gaudi::apply( []( int i, int j ) { return i + j; }, b ) == 4 );
  }
}

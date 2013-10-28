#include "GaudiKernel/ContextSpecificPtr.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace Gaudi::Hive;

BOOST_AUTO_TEST_CASE( basic_usage )
{
  ContextSpecificPtr<int> p;

  setCurrentContextId(0);

  BOOST_CHECK( ! p.isValid() );

  p = new int(42);

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 42 );
  int *p0 = p.get();

  setCurrentContextId(1);

  BOOST_CHECK( ! p.isValid() );

  p = new int(23);

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 23 );
  int *p1 = p.get();

  BOOST_CHECK( p0 != p1 );

  setCurrentContextId(0);

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 42 );


  int sum = p.accumulate([](const int*x) -> int {return *x;}, 0);
  BOOST_CHECK( sum == (42 + 23) );

  int prod = p.accumulate([](const int*x) -> int {return *x;}, 1,
      [](int a, int b) -> int {return a * b;});
  BOOST_CHECK( prod == (42 * 23) );


  p.deleteAll();
}

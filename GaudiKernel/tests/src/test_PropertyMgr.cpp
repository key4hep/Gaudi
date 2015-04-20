#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyMgr
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/GaudiException.h"

BOOST_AUTO_TEST_CASE( setters_and_getters )
{
  StringProperty p1{"v1"};
  StringProperty p2{"v2"};
  StringProperty p3{"v3"};
  {
    PropertyMgr mgr{nullptr};
    mgr.declareProperty("p1", p1);
    mgr.declareProperty("p2", p2);

    BOOST_CHECK(mgr.hasProperty("p1"));
    BOOST_CHECK(mgr.hasProperty("p2"));
    BOOST_CHECK(!mgr.hasProperty("p0"));

    // case insensitive check
    BOOST_CHECK(mgr.hasProperty("P1"));

    auto redeclare_property = [&mgr, &p3] () {
      mgr.declareProperty("p1", p3);
    };
    // FIXME: to be enabled if we decide to throw an exception, otherwise
    //        we need to improve it to check that the warning is printed
    //BOOST_CHECK_THROW(redeclare_property(), GaudiException);
  }
}

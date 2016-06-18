#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyMgr
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/GaudiException.h"

namespace {
  const std::string emptyName{};
  /// Helper to allow instantiation of PropertyMgr.
  struct AnonymousPropertyMgr: public implements<IProperty, INamedInterface>,
                               public PropertyMgr {
    const std::string& name() const override { return emptyName; }
  };
}

BOOST_AUTO_TEST_CASE( declareProperty )
{
  StringProperty p1{"v1"};
  StringProperty p2{"v2"};
  StringProperty p3{"v3"};
  {
    AnonymousPropertyMgr mgr;
    mgr.declareProperty("p1", p1);
    mgr.declareProperty("p2", p2);

    BOOST_CHECK(mgr.hasProperty("p1"));
    BOOST_CHECK(mgr.hasProperty("p2"));
    BOOST_CHECK(!mgr.hasProperty("p0"));

    // case insensitive check
    BOOST_CHECK(mgr.hasProperty("P1"));

    // FIXME: to be enabled if we decide to throw an exception, otherwise
    //        we need to improve it to check that the warning is printed
    //auto redeclare_property = [&mgr, &p3] () {
    //  mgr.declareProperty("p1", p3);
    //};
    //BOOST_CHECK_THROW(redeclare_property(), GaudiException);
  }
}

/*****************************************************************************\
* (c) Copyright 2013 CERN                                                     *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "LICENCE".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
/**
 * Compile-time test for all known PluginService use-cases
 *
 * @author Marco Clemencic <marco.clemencic@cern.ch>
 */

#include <Gaudi/PluginService.h>

// standard use, 0 arguments
class Base {
public:
  typedef Gaudi::PluginService::Factory0<Base*> Factory;
  virtual ~Base() {}
};
class Component0: public Base { };
DECLARE_COMPONENT(Component0)


// standard use, 2 arguments
class Base2 {
public:
  typedef Gaudi::PluginService::Factory2<Base2*, const std::string&, int> Factory;
  virtual ~Base2() {}
};
class Component2: public Base2 {
public:
  Component2(const std::string& _s, int _i): i(_i), s(_s) {}
  int i;
  std::string s;
};
DECLARE_COMPONENT(Component2)

// namespaces
namespace Test {
  class ComponentA: public Base {};
  class ComponentB: public Base {};
  class ComponentC: public Base {};
}

namespace {
  using Test::ComponentA;
  DECLARE_COMPONENT(ComponentA)
}

DECLARE_COMPONENT(Test::ComponentB)

namespace Test {
  DECLARE_COMPONENT(ComponentC)
}

// using ids
DECLARE_COMPONENT_WITH_ID(Component2, "Id2")
DECLARE_COMPONENT_WITH_ID(Test::ComponentB, "B")

// explicit factory
DECLARE_FACTORY_WITH_ID(Test::ComponentA, "A", Base::Factory)


// Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( basic )
{
  BOOST_CHECK(Base::Factory::create("Component0") != 0);
}

BOOST_AUTO_TEST_CASE( basic_with_args )
{
  Base2* instance = Base2::Factory::create("Component2", "hello", 2);
  BOOST_CHECK(instance != 0);

  Component2* c2 = dynamic_cast<Component2*>(instance);
  BOOST_REQUIRE(c2 != 0);
  BOOST_CHECK(c2->i == 2);
  BOOST_CHECK(c2->s == "hello");
}

BOOST_AUTO_TEST_CASE( namespaces )
{
  BOOST_CHECK(Base::Factory::create("Test::ComponentA") != 0);
  BOOST_CHECK(Base::Factory::create("Test::ComponentB") != 0);
  BOOST_CHECK(Base::Factory::create("Test::ComponentC") != 0);
}

BOOST_AUTO_TEST_CASE( ids )
{
  BOOST_CHECK(Base2::Factory::create("Id2", "id", -2) != 0);
  BOOST_CHECK(Base::Factory::create("A") != 0);
  BOOST_CHECK(Base::Factory::create("B") != 0);
}

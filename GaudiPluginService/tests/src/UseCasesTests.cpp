/***********************************************************************************\
* (c) Copyright 2013-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/**
 * Compile-time test for all known PluginService use-cases
 *
 * @author Marco Clemencic <marco.clemencic@cern.ch>
 */

#include "UseCasesLib.cpp"

// Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( basic ) { BOOST_CHECK( Base::Factory::create( "Component0" ) != nullptr ); }

BOOST_AUTO_TEST_CASE( basic_with_args ) {
  auto instance = Base2::Factory::create( "Component2", "hello", 2 );
  BOOST_CHECK( instance != nullptr );

  auto c2 = dynamic_cast<Component2*>( instance.get() );
  BOOST_REQUIRE( c2 != nullptr );
  BOOST_CHECK( c2->i == 2 );
  BOOST_CHECK( c2->s == "hello" );
}

BOOST_AUTO_TEST_CASE( namespaces ) {
  BOOST_CHECK( Base::Factory::create( "Test::ComponentA" ) != nullptr );
  BOOST_CHECK( Base::Factory::create( "Test::ComponentB" ) != nullptr );
  BOOST_CHECK( Base::Factory::create( "Test::ComponentC" ) != nullptr );
  BOOST_CHECK( Base::Factory::create( "Test::ComponentD" ) != nullptr );
}

BOOST_AUTO_TEST_CASE( ids ) {
  BOOST_CHECK( Base2::Factory::create( "Id2", "id", -2 ) != nullptr );
  BOOST_CHECK( Base::Factory::create( "A" ) != nullptr );
  BOOST_CHECK( Base::Factory::create( "B" ) != nullptr );
}

BOOST_AUTO_TEST_CASE( properties ) {
  using Gaudi::PluginService::Details::Registry;
  Registry&            reg   = Registry::instance();
  Registry::Properties props = reg.getInfo( "Component1" ).properties;

  BOOST_CHECK( props["name"] == "Component1" );

  BOOST_CHECK( reg.getInfo( "special-id" ).getprop( "MyProperty" ) == "special" );
}

#if __cplusplus > 201703L
BOOST_AUTO_TEST_CASE( source_location ) {
  using Gaudi::PluginService::Details::Registry;
  Registry&            reg   = Registry::instance();
  Registry::Properties props = reg.getInfo( "Component1" ).properties;

  BOOST_CHECK( props["declaration_location"] == "UseCasesLib.cpp:37" );
}
#endif

BOOST_AUTO_TEST_CASE( custom_factory ) {
  {
    auto c = MyBase::Factory::create( "MyComponent", "TheName" );
    BOOST_CHECK( c != nullptr );
    BOOST_CHECK( c->name() == "TheName" );
  }
  {
    auto c = Base::Factory::create( "special-id" );
    BOOST_CHECK( c != nullptr );
    auto c2 = dynamic_cast<SpecialId::MyComponent*>( c.get() );
    BOOST_CHECK( c2 != nullptr );
    BOOST_CHECK( c2->name == "special-id" );
  }
}

BOOST_AUTO_TEST_CASE( custom_factory_wrapper ) {
  {
    auto c = CustomFactoryWrapper::Base::Factory::create( "CustomFactoryWrapper::ComponentNew", "TheName" );
    BOOST_CHECK( c != nullptr );
    BOOST_CHECK( c->name() == "TheName" );
  }
  {
    auto c = CustomFactoryWrapper::Base::Factory::create( "CustomFactoryWrapper::ComponentOld", "TheName" );
    BOOST_CHECK( c != nullptr );
    BOOST_CHECK( c->name() == "TheName" );
  }
}

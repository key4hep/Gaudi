/***********************************************************************************\
* (c) Copyright 2013-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include "LegacyUseCasesLib.cpp"

// Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_CASE( basic ) {
  std::unique_ptr<Base> c( Base::Factory::create( "Component0" ) );
  BOOST_CHECK( c != nullptr );
}

BOOST_AUTO_TEST_CASE( basic_with_args ) {
  std::unique_ptr<Base2> instance( Base2::Factory::create( "Component2", "hello", 2 ) );
  BOOST_CHECK( instance != nullptr );

  Component2* c2 = dynamic_cast<Component2*>( instance.get() );
  BOOST_REQUIRE( c2 != nullptr );
  BOOST_CHECK( c2->i == 2 );
  BOOST_CHECK( c2->s == "hello" );
}

BOOST_AUTO_TEST_CASE( namespaces ) {
  std::unique_ptr<Base> a( Base::Factory::create( "Test::ComponentA" ) );
  std::unique_ptr<Base> b( Base::Factory::create( "Test::ComponentB" ) );
  std::unique_ptr<Base> c( Base::Factory::create( "Test::ComponentC" ) );
  BOOST_CHECK( a != nullptr );
  BOOST_CHECK( b != nullptr );
  BOOST_CHECK( c != nullptr );
}

BOOST_AUTO_TEST_CASE( ids ) {
  std::unique_ptr<Base2> id2( Base2::Factory::create( "Id2", "id", -2 ) );
  std::unique_ptr<Base>  a( Base::Factory::create( "A" ) );
  std::unique_ptr<Base>  b( Base::Factory::create( "B" ) );
  BOOST_CHECK( id2 != nullptr );
  BOOST_CHECK( a != nullptr );
  BOOST_CHECK( b != nullptr );
}

BOOST_AUTO_TEST_CASE( properties ) {
  using Gaudi::PluginService::Details::Registry;
  Registry&            reg   = Registry::instance();
  Registry::Properties props = reg.getInfo( "Component1" ).properties;

  BOOST_CHECK( props["name"] == "Component1" );
}

BOOST_AUTO_TEST_CASE( custom_factory ) {
  {
    _custom_factory_called = false;

    std::unique_ptr<Base> c( Base::Factory::create( "CompWithCustomFactory" ) );
    BOOST_CHECK( c != nullptr );
    BOOST_CHECK( _custom_factory_called == true );
  }
}

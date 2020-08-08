/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_JOS
#include <boost/test/unit_test.hpp>

#include "fixture.h"

#include <Gaudi/Property.h>
#include <GaudiKernel/IJobOptionsSvc.h>
#include <GaudiKernel/ISvcLocator.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
BOOST_AUTO_TEST_CASE( BackwardCompatibility ) {
  Fixture f;

  auto& jos_new = Gaudi::svcLocator()->getOptsSvc();
  auto  jos     = Gaudi::svcLocator()->service( "JobOptionsSvc" ).as<IJobOptionsSvc>();
  BOOST_REQUIRE( jos );

  {
    Gaudi::Property<bool>        bp{true};
    std::string                  bpv = bp.toString();
    Gaudi::Property<std::string> sp( "MeasureTime", bpv );
    BOOST_CHECK( jos->addPropertyToCatalogue( "SomeClient", sp ) );
    BOOST_CHECK_EQUAL( jos_new.get( "SomeClient.MeasureTime" ), "True" );
  }

  {
    jos_new.set( "Parent.SomeNumber", "42" );
    auto p = jos->getClientProperty( "Parent", "SomeNumber" );
    BOOST_REQUIRE( p );
    BOOST_CHECK( jos->addPropertyToCatalogue( "Parent.Child", *p ) );
    BOOST_CHECK_EQUAL( jos_new.get( "Parent.Child.SomeNumber" ), "42" );

    auto sp = dynamic_cast<const Gaudi::Property<std::string>*>( p );
    BOOST_REQUIRE( sp );
    BOOST_CHECK_EQUAL( sp->value(), "42" );
  }

  {
    jos_new.set( "Parent.SomeText", "\"lorem ipsum\"" );
    auto p = jos->getClientProperty( "Parent", "SomeText" );
    BOOST_REQUIRE( p );
    BOOST_CHECK( jos->addPropertyToCatalogue( "Parent.Child", *p ) );
    BOOST_CHECK_EQUAL( jos_new.get( "Parent.Child.SomeText" ), "lorem ipsum" );

    auto sp = dynamic_cast<const Gaudi::Property<std::string>*>( p );
    BOOST_REQUIRE( sp );
    BOOST_CHECK_EQUAL( sp->value(), "lorem ipsum" );
  }
}

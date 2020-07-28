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

#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Property.h>

BOOST_AUTO_TEST_CASE( PropertyBinding ) {
  Fixture f;

  auto& jos = Gaudi::svcLocator()->getOptsSvc();

  TestPropertyHolder           ph;
  Gaudi::Property<std::string> p1{&ph, "p1", "v1"};

  ph.bindPropertiesTo( jos );

  BOOST_CHECK( jos.has( "test.p1" ) );
  BOOST_CHECK_EQUAL( jos.get( "test.p1" ), "'v1'" );
}

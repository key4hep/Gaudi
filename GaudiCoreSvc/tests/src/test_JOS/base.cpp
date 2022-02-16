/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
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

BOOST_AUTO_TEST_CASE( JobOptionsSvc ) {
  Fixture f;

  auto& jos = Gaudi::svcLocator()->getOptsSvc();

  BOOST_CHECK( jos.has( "ApplicationMgr.AppName" ) );
  BOOST_CHECK( !jos.isSet( "ApplicationMgr.AppName" ) );
  BOOST_CHECK_EQUAL( jos.get( "ApplicationMgr.JobOptionsType" ), "'NONE'" );
  BOOST_CHECK_EQUAL( jos.get( "ApplicationMgr.AppName" ), "''" );
  BOOST_CHECK_EQUAL( jos.get( "ApplicationMgr.OutputLevel" ), "6" );
  jos.set( "ApplicationMgr.AppName", "'test_JOS.exe'" );
  BOOST_CHECK( jos.has( "ApplicationMgr.AppName" ) );
  BOOST_CHECK( jos.isSet( "ApplicationMgr.AppName" ) );
  BOOST_CHECK_EQUAL( jos.get( "ApplicationMgr.AppName" ), "'test_JOS.exe'" );

  BOOST_CHECK_EQUAL( jos.items().size(), 97 );

  BOOST_CHECK( !jos.has( "MyAlg.SomeOpt" ) );
  BOOST_CHECK( !jos.isSet( "MyAlg.SomeOpt" ) );
  jos.set( "MyAlg.SomeOpt", "42" );
  BOOST_CHECK( jos.has( "MyAlg.SomeOpt" ) );
  BOOST_CHECK( jos.isSet( "MyAlg.SomeOpt" ) );
  BOOST_CHECK_EQUAL( jos.get( "MyAlg.SomeOpt" ), "42" );

  BOOST_CHECK_EQUAL( jos.items().size(), 98 );
  BOOST_CHECK_EQUAL( jos.items( [&jos]( const auto& p ) { return jos.isSet( std::get<0>( p ) ); } ).size(), 2 );
  BOOST_CHECK_EQUAL( jos.items( std::regex{ ".*Level" } ).size(), 5 );
}

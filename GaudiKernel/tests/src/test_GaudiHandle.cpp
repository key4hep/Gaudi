/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_StringKey
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ServiceHandle.h>
#include <GaudiKernel/ToolHandle.h>

#include <string>

BOOST_AUTO_TEST_CASE( TestToolHandle ) {
  {
    ToolHandle<IAlgTool> h( "Type/Name" );
    BOOST_TEST( h.parentName() == "ToolSvc" );
    BOOST_TEST( h.typeAndName() == "Type/Name" );

    h = "OtherType/OtherName";
    BOOST_TEST( h.type() == "OtherType" );
    BOOST_TEST( h.name() == "OtherName" );

    std::string nameType;
    h = nameType;
    BOOST_TEST( h.empty() );
  }
}

BOOST_AUTO_TEST_CASE( TestServiceHandle ) {
  {
    ServiceHandle<IService> h( "Type/Name", "Parent" );
    BOOST_TEST( h.parentName() == "Parent" );
    BOOST_TEST( h.typeAndName() == "Type/Name" );

    h = "OtherType/OtherName";
    BOOST_TEST( h.type() == "OtherType" );
    BOOST_TEST( h.name() == "OtherName" );

    std::string nameType;
    h = nameType;
    BOOST_TEST( h.empty() );
  }
}

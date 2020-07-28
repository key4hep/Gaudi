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
#pragma once

#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/SmartIF.h>
#include <string>

#ifndef NO_BOOST
#  include <boost/test/unit_test.hpp>
#else
#  define BOOST_REQUIRE( x )
#endif

struct Fixture {
  Fixture( const std::string& jos_name = "JobOptionsSvc", int outLevel = 6 ) {
    // Load required libraries (bypass PluginService)
    {
      System::ImageHandle handle;
      System::loadDynamicLib( "libGaudiCoreSvc.so", &handle );
    }

    // == Begin Bootstrap
    // Instantiate the application
    auto app = Gaudi::createApplicationMgr();

    SmartIF<IProperty> appProp{app};
    BOOST_REQUIRE( appProp );

    appProp->setProperty( "JobOptionsSvcType", jos_name ).ignore();
    // prevent reading of options file
    appProp->setProperty( "JobOptionsType", "NONE" ).ignore();
    // prevent printout of ApplicationMgr banner
    appProp->setPropertyRepr( "AppName", "''" ).ignore();
    appProp->setProperty( "OutputLevel", outLevel ).ignore();

    BOOST_REQUIRE( app->configure() );
    // == End Bootstrap
  }

  ~Fixture() = default;
};

/// Helper to allow instantiation of PropertyHolder.
struct TestPropertyHolder : public PropertyHolder<implements<IProperty, INamedInterface>> {
  const std::string& name() const override { return m_name; }
  std::string        m_name{"test"};
};

#ifdef NO_BOOST
#  undef BOOST_REQUIRE
#endif

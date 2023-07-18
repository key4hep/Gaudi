/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Gaudi/PluginService.h"
#include "GaudiKernel/Algorithm.h"

#include <iostream>

namespace PluginServiceTest {
  class CustomFactoryAlgorithm : public Algorithm {
  public:
    using Algorithm::Algorithm;

    StatusCode execute() override {
      info() << "running" << endmsg;
      return StatusCode::SUCCESS;
    }
  };
} // namespace PluginServiceTest

namespace {
  using PluginServiceTest::CustomFactoryAlgorithm;

  std::unique_ptr<IAlgorithm> creator( const std::string& name, ISvcLocator* svcLoc ) {
    auto p = std::make_unique<CustomFactoryAlgorithm>( name, svcLoc );

    // do not print messages if we are created in genconf
    const std::string cmd = System::cmdLineArgs()[0];
    if ( cmd.find( "genconf" ) == std::string::npos ) {
      std::cout << "created CustomFactoryAlgorithm at " << p.get() << std::endl;
    }
    return p;
  }

  Gaudi::PluginService::DeclareFactory<CustomFactoryAlgorithm> _{ creator };
} // namespace

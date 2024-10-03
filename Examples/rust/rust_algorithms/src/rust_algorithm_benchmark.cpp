/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Application.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>

#define PICOBENCH_IMPLEMENT_WITH_MAIN
#include "picobench.hpp"

namespace {
  void benchmark( std::string name, picobench::state& s ) {
    using namespace Gaudi;
    auto app = Application( { { "ApplicationMgr.EvtSel", "NONE" },
                              { "ApplicationMgr.JobOptionsType", "NONE" },
                              { "ApplicationMgr.OutputLevel", "5" },
                              { "ApplicationMgr.AppName", "" } } );
    app.run( [&name, &s]( auto app ) {
      auto         prop = app.template as<IProperty>();
      EventContext ctx{ 0, 0 };
      if ( app->initialize() ) {
        if ( app->start() ) {
          auto alg = app.template as<IAlgManager>()->algorithm( name + "/alg" );
          for ( [[maybe_unused]] auto _ : s ) { alg->execute( ctx ).ignore(); }
          app->stop().ignore();
        } else
          setAppReturnCode( prop, ReturnCode::GenericFailure ).ignore();
        app->finalize().ignore();
      } else
        setAppReturnCode( prop, ReturnCode::GenericFailure ).ignore();
      return getAppReturnCode( prop );
    } );
  }
} // namespace

static void cpp( picobench::state& s ) { benchmark( "Gaudi::Examples::MyCppCountingAlg", s ); }
PICOBENCH( cpp );
static void rust( picobench::state& s ) { benchmark( "Gaudi::Examples::MyRustCountingAlg", s ); }
PICOBENCH( rust );

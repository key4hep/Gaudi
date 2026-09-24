/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/IAuditor.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/Guards.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/StatusCode.h>
#include <string>

#if __has_include( <catch2/catch.hpp> )
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

namespace {
  // Minimal auditor that reads the EventContext handed to before()/after(), the same
  // thing GaudiTestSuite's LoggingAuditor does. That read is what turned the dangling
  // reference in gaudi/Gaudi#421 into an observable ASan stack-use-after-scope.
  struct RecordingAuditor : implements<Gaudi::IAuditor> {
    const std::string& name() const override {
      static const std::string name = "RecordingAuditor";
      return name;
    }
    void before( std::string const&, std::string const&, EventContext const& ctx ) override {
      ++beforeCount;
      beforeWasValid = ctx.valid();
    }
    void after( std::string const&, std::string const&, EventContext const& ctx,
                StatusCode const& = StatusCode::SUCCESS ) override {
      ++afterCount;
      afterWasValid = ctx.valid();
    }
    bool       isEnabled() const override { return true; }
    StatusCode sysInitialize() override { return StatusCode::SUCCESS; }
    StatusCode sysFinalize() override { return StatusCode::SUCCESS; }

    int  beforeCount{ 0 };
    int  afterCount{ 0 };
    bool beforeWasValid{ true };
    bool afterWasValid{ true };
  };
} // namespace

TEST_CASE( "AuditorGuard without an EventContext does not dangle" ) {
  // Regression test for gaudi/Gaudi#421: AuditorGuard used to bind its m_context
  // reference member to the `EventContext const& context = {}` default-argument
  // temporary, which is destroyed at the end of the constructor call, leaving the
  // member dangling for the rest of the guard's life. Reading ctx.valid() below, from
  // after() called out of ~AuditorGuard(), is exactly what turned that into an ASan
  // stack-use-after-scope report. A held extra reference keeps the auditor alive past
  // the guard so its recorded state can be checked afterwards.
  auto*                    auditor = new RecordingAuditor();
  SmartIF<Gaudi::IAuditor> keepAlive{ auditor };
  {
    Gaudi::Guards::AuditorGuard guard( "SomeAlgorithm", auditor, Gaudi::IAuditor::Execute );
    CHECK( auditor->beforeCount == 1 );
    CHECK_FALSE( auditor->beforeWasValid );
  }
  CHECK( auditor->afterCount == 1 );
  CHECK_FALSE( auditor->afterWasValid );
}

TEST_CASE( "AuditorGuard forwards a caller-provided EventContext" ) {
  auto*                    auditor = new RecordingAuditor();
  SmartIF<Gaudi::IAuditor> keepAlive{ auditor };
  EventContext             ctx{ 42, 0 };
  REQUIRE( ctx.valid() );
  {
    Gaudi::Guards::AuditorGuard guard( "SomeAlgorithm", auditor, Gaudi::IAuditor::Execute, ctx );
    CHECK( auditor->beforeWasValid );
  }
  CHECK( auditor->afterWasValid );
}

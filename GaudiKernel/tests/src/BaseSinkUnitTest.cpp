/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_BaseSink

#include <Gaudi/Application.h>
#include <Gaudi/BaseSink.h>
#include <Gaudi/MonitoringHub.h>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <sstream>
#include <string>

// Mock code for the test
struct MonitoringHub : Gaudi::Monitoring::Hub {};
struct ServiceLocator {
  MonitoringHub& monitoringHub() { return m_monitHub; }
  MonitoringHub  m_monitHub{};
};
struct EmptyEntity {
  friend void to_json( nlohmann::json& j, EmptyEntity const& ) { j = {}; }
};

struct TestSelectSink : public Gaudi::Monitoring::BaseSink {
  TestSelectSink( std::ostream& output, ISvcLocator* sl ) : BaseSink( "TestSelectSink", sl ), m_output( output ) {}
  void flush( bool ) override {
    applyToAllSortedEntities(
        [this]( std::string const&, std::string const& name, nlohmann::json const& ) { m_output << name << "\n"; } );
  }
  std::ostream& m_output;
};

BOOST_AUTO_TEST_CASE( test_sink_select ) {
  Gaudi::Application::Options opts{ { "ApplicationMgr.JobOptionsType", "\"NONE\"" } };
  auto                        app = Gaudi::Application( std::move( opts ) );
  app.run( []( SmartIF<IStateful>& app ) -> int {
    // create hub and sink
    std::stringstream output;
    ISvcLocator*      sl = app.as<ISvcLocator>();
    TestSelectSink    sink{ output, sl };
    auto&             mh = sl->monitoringHub();
    mh.addSink( &sink );
    // setup sink to ignore some entities and add entitites
    sink.setProperty( "NamesToSave", std::vector<std::string>{ "GivenName", "OKName.*" } ).ignore();
    sink.setProperty( "ComponentsToSave", std::vector<std::string>{ "OKComp", "OKReg[123]+Comp" } ).ignore();
    EmptyEntity e1, e2, e3, e4, e5;
    mh.registerEntity( "OKComp", "GivenName", "Any type should work", e1 );
    mh.registerEntity( "NotOkComp", "OKName", "Any type should work", e2 );
    mh.registerEntity( "OKComp", "NotOKName", "Any type should work", e3 );
    mh.registerEntity( "OKReg122333Comp", "OKNameExtended", "Any type should work", e4 );
    mh.registerEntity( "OKRegNoNumberComp", "OKNameExtended2", "Any type should work", e5 );
    // Run the sink and check
    sink.start().ignore();
    sink.stop().ignore();
    BOOST_TEST( output.str() == "GivenName\nOKNameExtended\n" );
    return 0;
  } );
}

const std::string RegularLine{ "Flush called regularly\n" };
const std::string FinalLine{ "Flush called in stop\n" };

struct TestFlushSink : public Gaudi::Monitoring::BaseSink {
  TestFlushSink( std::ostream& output, ISvcLocator* sl ) : BaseSink( "TestFlushSink", sl ), m_output( output ) {}
  void          flush( bool isStop ) override { m_output << ( isStop ? FinalLine : RegularLine ); }
  std::ostream& m_output;
};

BOOST_AUTO_TEST_CASE( test_sink_autoflush ) {
  using namespace std::chrono_literals;
  Gaudi::Application::Options opts{ { "ApplicationMgr.JobOptionsType", "\"NONE\"" } };
  auto                        app = Gaudi::Application( std::move( opts ) );
  app.run( []( SmartIF<IStateful>& app ) -> int {
    std::stringstream output;
    TestFlushSink     sink{ output, app.as<ISvcLocator>() };
    sink.setProperty( "AutoFlushPeriod", .5 ).ignore();
    sink.start().ignore();
    std::this_thread::sleep_for( 600ms );
    BOOST_TEST( output.str() == RegularLine );
    std::this_thread::sleep_for( 600ms );
    BOOST_TEST( output.str() == RegularLine + RegularLine );
    sink.stop().ignore();
    BOOST_TEST( output.str() == RegularLine + RegularLine + FinalLine );
    return 0;
  } );
}

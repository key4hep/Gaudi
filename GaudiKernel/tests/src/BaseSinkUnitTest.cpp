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
#define BOOST_TEST_MODULE test_BaseSink

#include <Gaudi/Application.h>
#include <Gaudi/BaseSink.h>
#include <Gaudi/MonitoringHub.h>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
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
  TestFlushSink( ISvcLocator* sl ) : BaseSink( "TestFlushSink", sl ) {}
  void flush( bool isStop ) override {
    {
      std::lock_guard lock{ m_mutex };
      m_output << ( isStop ? FinalLine : RegularLine );
      ++m_flushCount;
    }
    m_cv.notify_all();
  }
  /// Block until at least n flushes have occurred (5 s timeout).
  void waitForFlushCount( int n ) {
    std::unique_lock lock{ m_mutex };
    m_cv.wait_for( lock, std::chrono::seconds( 5 ), [&] { return m_flushCount >= n; } );
  }
  /// Thread-safe snapshot of the accumulated output.
  std::string str() {
    std::lock_guard lock{ m_mutex };
    return m_output.str();
  }
  std::stringstream       m_output;
  std::mutex              m_mutex;
  std::condition_variable m_cv;
  int                     m_flushCount{ 0 };
};

BOOST_AUTO_TEST_CASE( test_sink_autoflush ) {
  Gaudi::Application::Options opts{ { "ApplicationMgr.JobOptionsType", "\"NONE\"" } };
  auto                        app = Gaudi::Application( std::move( opts ) );
  app.run( []( SmartIF<IStateful>& app ) -> int {
    TestFlushSink sink{ app.as<ISvcLocator>() };
    sink.setProperty( "AutoFlushPeriod", .5 ).ignore();
    auto t0 = std::chrono::steady_clock::now();
    sink.start().ignore();
    sink.waitForFlushCount( 1 );
    BOOST_CHECK( std::chrono::steady_clock::now() - t0 >= std::chrono::milliseconds( 500 ) );
    BOOST_TEST( sink.str() == RegularLine );
    sink.waitForFlushCount( 2 );
    BOOST_CHECK( std::chrono::steady_clock::now() - t0 >= std::chrono::milliseconds( 1000 ) );
    BOOST_TEST( sink.str() == RegularLine + RegularLine );
    sink.stop().ignore();
    BOOST_TEST( sink.str() == RegularLine + RegularLine + FinalLine );
    return 0;
  } );
}

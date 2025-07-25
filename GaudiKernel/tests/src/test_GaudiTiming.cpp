/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * Time_test.cpp
 *
 *  Created on: Jan 13, 2011
 *      Author: Marco Clemencic
 */

#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/Memory.h>
#include <GaudiKernel/Sleep.h>
#include <GaudiKernel/Timing.h>
#include <cmath>

// from SPI version of the testdriver
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

// to produce one line per test
#include <cppunit/TestResult.h>
// #include <cppunit/BriefTestProgressListener.h>
// #include <cppunit/TestResultCollector.h>

// needed by the implementation of cool::ProgressListener
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>

// provides macros for the tests
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace GaudiKernelTest {
  class TimingTest : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( TimingTest );

    CPPUNIT_TEST( test_adjustTime );
    CPPUNIT_TEST( test_ProcessTime );
    CPPUNIT_TEST( test_ProcessTimePerf );
    CPPUNIT_TEST( test_ChronoEntity );

    CPPUNIT_TEST_SUITE_END();

  public:
    TimingTest() {}
    virtual ~TimingTest() {}

    void test_adjustTime() {
      long long t = 12345678901234560LL; // units of 100 nanoseconds
      // 1234567890123456000 nanoseconds
      //    1234567890123456 microseconds
      //       1234567890123 milliseconds
      //          1234567890 seconds
      //            20576131 minutes
      //              342935 hours
      //               14288 days
      //                 476 months (of 30 days)
      //                  39 years (of 365 days)
      //   12345678901234560 native
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::nanoSec>( t ), 1234567890123456000LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::microSec>( t ), 1234567890123456LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::milliSec>( t ), 1234567890123LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Sec>( t ), 1234567890LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Min>( t ), 20576131LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Hour>( t ), 342935LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Day>( t ), 14288LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Month>( t ), 476LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Year>( t ), 39LL );
      CPPUNIT_ASSERT_EQUAL( System::adjustTime<System::Native>( t ), 12345678901234560LL );
    }

    void test_currentTime() {
      long long now = System::currentTime( System::microSec );
      CPPUNIT_ASSERT( now ); // non-zero

      Gaudi::Sleep( 1 ); // wait a wee bit

      long long later = System::currentTime( System::microSec );
      CPPUNIT_ASSERT( later ); // non-zero

      CPPUNIT_ASSERT( later > now );
      float seconds = float( later - now ) / 1000000;
      CPPUNIT_ASSERT( fabs( seconds - 1.f ) < 0.01 );
    }

    void test_ProcessTime() {
      System::ProcessTime t0 = System::getProcessTime();
      Gaudi::Sleep( 1 );
      System::ProcessTime t1 = System::getProcessTime();

      // light check on the conversion
      CPPUNIT_ASSERT_EQUAL( t1.kernelTime<System::Sec>(), t1.kernelTime<System::milliSec>() / 1000 );
      // measure the elapsed time
      CPPUNIT_ASSERT_EQUAL( ( t1 - t0 ).elapsedTime<System::Sec>(), 1LL );

      // This should result in non-zero user and kernel times
      t0       = System::getProcessTime();
      double x = 1.5;

      // this variable is only needed to ensure that the timed loop
      // takes a measurable time
      [[maybe_unused]] long m = 0;
      for ( int i = 0; i < 10000; i++ ) {
        x *= sin( x ) / atan( x ) * tanh( x ) * sqrt( x );
        m += System::virtualMemory();
      }
      t1 = System::getProcessTime();

      CPPUNIT_ASSERT( ( t1 - t0 ).userTime<System::microSec>() > 0 );
      CPPUNIT_ASSERT( ( t1 - t0 ).kernelTime<System::microSec>() > 0 );
    }

    void test_ProcessTimePerf() {
      System::ProcessTime t, t0, t1;

      t0    = System::getProcessTime();
      int N = 1e6;
      for ( int i = 0; i < N; i++ ) { t = System::getProcessTime(); }
      t1 = System::getProcessTime();
      std::cout << " (" << ( t1 - t0 ).elapsedTime<System::nanoSec>() / N << " ns per call)";
    }

    void test_ChronoEntity() {

      ChronoEntity c1;
      for ( int i = 0; i < 10; i++ ) {
        c1.start();
        Gaudi::NanoSleep( 1e7 ); // 10ms
        c1.stop();
      }
      CPPUNIT_ASSERT_EQUAL( c1.nOfMeasurements(), 10UL );
      // average time may be affected by the load of the machine
      CPPUNIT_ASSERT( c1.eMeanTime() >= c1.eMinimalTime() );
      CPPUNIT_ASSERT( c1.eMeanTime() <= c1.eMaximalTime() );
      CPPUNIT_ASSERT( c1.eMeanTime() >= 8000 );
      CPPUNIT_ASSERT( c1.eMeanTime() <= 19000 );
      CPPUNIT_ASSERT( c1.eMaximalTime() >= c1.eMinimalTime() );

      ChronoEntity c2;
      c2 += c1;
      c2 += c1;
      CPPUNIT_ASSERT( c1 < c2 );

      // Performance
      ChronoEntity        c3;
      System::ProcessTime t0 = System::getProcessTime();
      int                 N  = 1e6;
      for ( int i = 0; i < N; i++ ) {
        c3.start();
        c3.stop();
      }
      System::ProcessTime t1 = System::getProcessTime();
      std::cout << " (" << ( t1 - t0 ).elapsedTime<System::nanoSec>() / N << " ns per stop/start)";
    }

    void setUp() override {}
    void tearDown() override {}
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( TimingTest );

  // Copied from the COOL implementation
  /** @class ProgressListener
   *
   *  Simple TestListener printing one line per test in the standard output.
   *
   *  Based on  CppUnit::BriefTestProgressListener (copy and paste)
   *  using std::cout instead of std::cerr.
   *
   *  @author Marco Clemencic
   *  @date   2006-11-13
   */
  class ProgressListener : public CppUnit::TestListener {

  public:
    /// Default constructor.
    ProgressListener() : m_lastTestFailed( false ) {}

    /// Destructor.
    virtual ~ProgressListener() {}

    void startTest( CppUnit::Test* test ) override {
      std::cout << test->getName();
      std::cout.flush();
      m_lastTestFailed = false;
    }

    void addFailure( const CppUnit::TestFailure& failure ) override {
      std::cout << " : " << ( failure.isError() ? "error" : "assertion" );
      m_lastTestFailed = true;
    }

    void endTest( CppUnit::Test* /*test*/ ) override {
      if ( !m_lastTestFailed ) std::cout << " : OK";
      std::cout << std::endl;
    }

  private:
    bool m_lastTestFailed;
  };
} // namespace GaudiKernelTest

// Copied from the COOL implementation
#include <stdexcept>
int main( int argc, char* argv[] ) {
  // Retrieve test path from command line first argument.
  // Default to "" which resolve to the top level suite.
  std::string testPath = ( argc > 1 ) ? std::string( argv[1] ) : std::string( "" );

  // Add a listener that collects test result
  // CppUnit::TestResultCollector result;
  // controller.addListener( &result );

  /// Get the top level suite from the registry
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  /// Adds the test to the list of test to run
  // CppUnit::TestRunner runner;
  CppUnit::TextTestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  // uncomment the following line if you need a compiler outputter.
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), std::cout ) );

  // Change the default outputter to a xml error format outputter
  // uncomment the following line if you need a xml outputter.
  // runner.setOutputter( new CppUnit::XmlOutputter( &runner.result(),
  //                                                    std::cout ) );

  auto l = std::make_unique<GaudiKernelTest::ProgressListener>();
  runner.eventManager().addListener( l.get() );

  // CppUnit::TestResultCollector *collector =
  //  new CppUnit::TestResultCollector();
  // runner.eventManager().addListener(collector);

  bool wasSuccessful = false;

  try {
    wasSuccessful = runner.run( testPath, false, true, false );
  }

  // Test path not resolved
  catch ( std::invalid_argument& e ) {
    std::cout << std::endl << "ERROR: " << e.what() << std::endl;
    return 0;
  }

  // Should never happen?
  catch ( std::exception& e ) {
    std::cout << std::endl << "UNEXPECTED STD EXCEPTION CAUGHT: " << e.what() << std::endl;
    return 0;
  }

  // Should never happen?
  catch ( ... ) {
    std::cout << std::endl << "UNKNOWN EXCEPTION CAUGHT" << std::endl;
    return 0;
  }

  // Return error code 1 if the one of tests failed.
  // Print a message on standard error if something failed (for QMTest)
  if ( !wasSuccessful ) std::cerr << "Error: CppUnit Failures" << std::endl;
  int retcode = wasSuccessful ? 0 : 1;

  // Uncomment the next line if you want to integrate CppUnit with Oval
  // std::cout << "[OVAL] Cppunit-result =" << retcode << std::endl;
  return retcode;
}

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
/*
 * SerialTaskQueue_test.cpp
 *
 * @date   2012-10-21
 * @author Marco Clemencic
 */

#include "GaudiKernel/SerialTaskQueue.h"

// Default serialization of STL containers.
#include "GaudiKernel/SerializeSTL.h"
using namespace GaudiUtils;

// from SPI version of the testdriver
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

// to produce one line per test
#include <cppunit/TestResult.h>
//#include <cppunit/BriefTestProgressListener.h>
//#include <cppunit/TestResultCollector.h>

// needed by the implementation of cool::ProgressListener
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>

// provides macros for the tests
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include <tbb/task_group.h>

// #include <thread>
#include "GaudiKernel/Sleep.h"
#include <chrono>

#include <algorithm>
#include <memory>

namespace GaudiKernelTest {
  class SerialTaskQueueTest : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( SerialTaskQueueTest );

    CPPUNIT_TEST( test_basic_serial );
    CPPUNIT_TEST( test_basic_parallel );

    CPPUNIT_TEST_SUITE_END();

  public:
    std::vector<int> resultData;

    class PushBackTask : public Gaudi::SerialTaskQueue::WorkItem {
    public:
      PushBackTask( std::vector<int>& results, int value ) : m_results( results ), m_value( value ) {}
      void run() override {
        using namespace std::chrono;

        milliseconds duration( 200 );
        // std::this_thread::sleep_for( duration );

        const nanoseconds ns = duration_cast<nanoseconds>( duration );
        Gaudi::NanoSleep( ns.count() );

        std::cout << "PushBackTask: " << m_value << std::endl;
        m_results.push_back( m_value );
      }

    private:
      std::vector<int>& m_results;
      int               m_value;
    };

    class Enqueuer {
    public:
      Enqueuer( Gaudi::SerialTaskQueue& q, std::vector<int>& r, int _n ) : queue( q ), results( r ), n( _n ) {}
      void operator()() const { queue.add( new PushBackTask( results, n ) ); }

    private:
      Gaudi::SerialTaskQueue& queue;
      std::vector<int>&       results;
      int                     n;
    };

    SerialTaskQueueTest() {}
    virtual ~SerialTaskQueueTest() {}

    void setUp() override {}

    void tearDown() override {}

    void test_basic_serial() {
      std::cout << std::endl;

      {

        Gaudi::SerialTaskQueue queue;
        std::cout << "enqueue tasks" << std::endl;
        for ( int i = 0; i != 10; ++i ) { queue.add( new PushBackTask( resultData, i ) ); }
        std::cout << "enqueueing completed" << std::endl;

      } // Gaudi::SerialTaskQueue ensures the synchronization.
      std::cout << "tasks completed" << std::endl;

      const std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      CPPUNIT_ASSERT_EQUAL( expected, resultData );
    }

    void test_basic_parallel() {
      std::cout << std::endl;
      {

        Gaudi::SerialTaskQueue queue;
        std::cout << "enqueue tasks" << std::endl;
        tbb::task_group g;
        for ( int i = 0; i != 10; ++i ) { g.run( Enqueuer( queue, resultData, i ) ); }
        g.wait();
        std::cout << "enqueueing completed" << std::endl;
        // Insertion should not have finished yet
        CPPUNIT_ASSERT( resultData.size() < 10 );

      } // Gaudi::SerialTaskQueue ensures the synchronization.
      std::cout << "tasks completed" << std::endl;

      CPPUNIT_ASSERT( resultData.size() == 10 );

      std::sort( resultData.begin(), resultData.end() );
      const std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      CPPUNIT_ASSERT_EQUAL( expected, resultData );
    }
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( SerialTaskQueueTest );

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

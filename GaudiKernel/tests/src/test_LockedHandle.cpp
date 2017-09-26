/*
 * test_LockedHandle.cpp
 *
 *  Created on: 2016-08-15
 *      Author: Charles Leggett (copied from test_GaudiTiming.cpp)
 */

#include "GaudiKernel/LockedHandle.h"
#include "GaudiKernel/Timing.h"

#include "boost/thread/mutex.hpp"
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

// from SPI version of the testdriver
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

// to produce one line per test
#include <cppunit/TestResult.h>

// needed by the implementation of cool::ProgressListener
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>

// provides macros for the tests
#include <cppunit/extensions/HelperMacros.h>

namespace GaudiKernelTest
{

  template <class M = std::mutex>
  class Hist
  {
  public:
    Hist( const std::string& id ) : m_id( id ) {}
    const std::string& id() const { return m_id; }
    void acc()
    {
      std::cout << "in Hist::access\n";
      m_i++;
    }
    void incr() { m_i++; }
    int val() const { return m_i; }

    M& mut() { return m_mut; }

  private:
    int m_i{0};
    std::string m_id;
    M m_mut;
  };

  template <class T>
  void Task( T* lh, int i, size_t nIter )
  {

    std::cout << "spawn Task " << i << "\n";
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    for ( size_t j = 0; j < nIter; ++j ) {
      ( *lh )->incr();
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );
    std::cout << "done Task " << i << "\n";
  }

  class LockedHandleTest : public CppUnit::TestFixture
  {

    CPPUNIT_TEST_SUITE( LockedHandleTest );

    CPPUNIT_TEST( test_handle );

    CPPUNIT_TEST_SUITE_END();

  public:
    LockedHandleTest() {}
    virtual ~LockedHandleTest() {}

    void test_handle()
    {

      // test default template args
      Hist<>* h1 = new Hist<>( "first" );
      LockedHandle<Hist<>> lh1( h1, h1->mut() );
      lh1->acc();

      // test explicit mutex type
      typedef std::mutex mut_t;
      Hist<mut_t>* h2 = new Hist<mut_t>( "first" );
      LockedHandle<Hist<mut_t>, mut_t> lh2( h2, h2->mut() );
      lh2->acc();

      // // test a different mutex type
      typedef boost::mutex mut_b_t;
      Hist<mut_b_t>* h3 = new Hist<mut_b_t>( "first" );
      LockedHandle<Hist<mut_b_t>, mut_b_t> lh3( h3, h3->mut() );
      lh3->acc();

      // do a lot to see if we can get a race
      std::vector<std::thread> threads;
      size_t nthreads{10};
      size_t nIter{10000};
      for ( size_t i = 0; i < nthreads; ++i ) {
        threads.push_back( std::thread{Task<LockedHandle<Hist<>>>, &lh1, i, nIter} );
      }

      for ( auto& t : threads ) {
        t.join();
      }

      CPPUNIT_ASSERT_EQUAL( int( 1 + ( nthreads * nIter ) ), int( h1->val() ) );
    }

    void setUp() override {}
    void tearDown() override {}
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( LockedHandleTest );

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
  class ProgressListener : public CppUnit::TestListener
  {

  public:
    /// Default constructor.
    ProgressListener() : m_lastTestFailed( false ) {}

    /// Destructor.
    virtual ~ProgressListener() {}

    void startTest( CppUnit::Test* test ) override
    {
      std::cout << test->getName();
      std::cout.flush();
      m_lastTestFailed = false;
    }

    void addFailure( const CppUnit::TestFailure& failure ) override
    {
      std::cout << " : " << ( failure.isError() ? "error" : "assertion" );
      m_lastTestFailed = true;
    }

    void endTest( CppUnit::Test* /*test*/ ) override
    {
      if ( !m_lastTestFailed ) std::cout << " : OK";
      std::cout << std::endl;
    }

  private:
    bool m_lastTestFailed;
  };
}

// Copied from the COOL implementation
#include <stdexcept>
int main( int argc, char* argv[] )
{
  // Retrieve test path from command line first argument.
  // Default to "" which resolve to the top level suite.
  std::string testPath = ( argc > 1 ) ? std::string( argv[1] ) : std::string( "" );

  /// Get the top level suite from the registry
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  /// Adds the test to the list of test to run
  CppUnit::TextTestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  // uncomment the following line if you need a compiler outputter.
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), std::cout ) );

  // Change the default outputter to a xml error format outputter
  // uncomment the following line if you need a xml outputter.
  // runner.setOutputter( new CppUnit::XmlOutputter( &runner.result(),
  //                                                    std::cout ) );

  runner.eventManager().addListener( new GaudiKernelTest::ProgressListener() );

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

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
#include "GaudiKernel/AnyDataWrapper.h"

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

// Standard headers
#include <memory>
#include <vector>

namespace GaudiKernelTest {

  class AnyDataObject : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( AnyDataObject );

    CPPUNIT_TEST( test_wrapper );
    CPPUNIT_TEST( test_wrapperVector );

    CPPUNIT_TEST_SUITE_END();

  public:
    AnyDataObject() {}
    virtual ~AnyDataObject() {}

    /**
     * Simple wrapper test
     */
    void test_wrapper() {
      auto adw = AnyDataWrapper<int>( 42 );
      CPPUNIT_ASSERT_EQUAL( 42, adw.getData() );

      int myval = 43;
      {
        auto adw2 = AnyDataWrapper<int>( std::move( myval ) );
        CPPUNIT_ASSERT_EQUAL( 43, adw2.getData() );
      }
    }

    /**
     * wrapping std::vectors
     */
    void test_wrapperVector() {

      using V     = std::vector<int>;
      using VSize = V::size_type;

      // Test move of a vector
      {
        auto  adw = AnyDataWrapper<V>( {1, 2, 3, 4} );
        VSize s4( 4 );
        CPPUNIT_ASSERT_EQUAL( s4, ( adw.getData() ).size() );
      }

      // Making sure that the vector is copied
      {
        V myvector{1, 2, 3, 4};
        CPPUNIT_ASSERT_EQUAL( static_cast<VSize>( 4 ), myvector.size() );

        auto adw = AnyDataWrapper<V>( std::move( myvector ) );

        CPPUNIT_ASSERT_EQUAL( static_cast<VSize>( 4 ), ( adw.getData() ).size() );

        CPPUNIT_ASSERT_EQUAL( static_cast<VSize>( 0 ), myvector.size() );
      }

      // Trying with std::unique_ptr
      {
        auto myvector = std::make_unique<V>( V{1, 2, 3, 4} );
        auto adw      = AnyDataWrapper<decltype( myvector )>( std::move( myvector ) );
        CPPUNIT_ASSERT_EQUAL( static_cast<VSize>( 4 ), ( adw.getData() )->size() );
      }
    }
  };

  CPPUNIT_TEST_SUITE_REGISTRATION( AnyDataObject );

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

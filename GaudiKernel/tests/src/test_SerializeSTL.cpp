#include "GaudiKernel/SerializeSTL.h"

// from SPI version of the testdriver
#include <cppunit/extensions/TestFactoryRegistry.h> 
#include <cppunit/TextTestRunner.h> 
#include <cppunit/CompilerOutputter.h> 
#include <cppunit/TextOutputter.h> 
#include <cppunit/XmlOutputter.h> 

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

namespace GaudiKernelTest {
  class SerializeSTL: public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE( SerializeSTL );
    
    CPPUNIT_TEST( test_vector );
    CPPUNIT_TEST( test_list );
    CPPUNIT_TEST( test_pair );
    CPPUNIT_TEST( test_map );
    CPPUNIT_TEST( test_gaudiutils_map );
    CPPUNIT_TEST( test_gaudiutils_hashmap );
    
    CPPUNIT_TEST_SUITE_END();
      
  public:
    
    SerializeSTL() {}
    virtual ~SerializeSTL() {}

#define CHECK_SERIALIZATION(exp,obj) { \
  std::ostringstream __o; \
  __o << obj; \
  CPPUNIT_ASSERT_EQUAL( std::string(exp), __o.str() ); \
  }

    void test_vector() {
      using namespace GaudiUtils;
      
      std::vector<int> v_i; CHECK_SERIALIZATION("[]",v_i)
      v_i.push_back(123); CHECK_SERIALIZATION("[123]",v_i)
      v_i.push_back(456);
      v_i.push_back(789);
      v_i.push_back(10000);
      CHECK_SERIALIZATION("[123, 456, 789, 10000]",v_i)
      
      std::vector<std::string> v_s;
      v_s.push_back("a"); CHECK_SERIALIZATION("[a]",v_s)
      v_s.push_back("bc");
      v_s.push_back("def");
      v_s.push_back("Hello World!");
      CHECK_SERIALIZATION("[a, bc, def, Hello World!]",v_s)
    }
    
    void test_list() {
      using namespace GaudiUtils;
      
      std::list<int> l_i; CHECK_SERIALIZATION("[]",l_i)
      l_i.push_back(123); CHECK_SERIALIZATION("[123]",l_i)
      l_i.push_back(456);
      l_i.push_back(789);
      l_i.push_back(10000);
      CHECK_SERIALIZATION("[123, 456, 789, 10000]",l_i)
      
      std::list<std::string> l_s;
      l_s.push_back("a"); CHECK_SERIALIZATION("[a]",l_s)
      l_s.push_back("bc");
      l_s.push_back("def");
      l_s.push_back("Hello World!");
      CHECK_SERIALIZATION("[a, bc, def, Hello World!]",l_s)
    }

    void test_pair() {
      using namespace GaudiUtils;
      
      std::pair<int,int> p1(20,11);
      CHECK_SERIALIZATION("(20, 11)",p1)
      
      std::pair<std::string,std::string> p2("Hello", "World");
      CHECK_SERIALIZATION("(Hello, World)",p2)
      
      std::pair<std::string,int> p3("November",20);
      CHECK_SERIALIZATION("(November, 20)",p3)
      
      std::list<int> l;
      l.push_back(1);
      l.push_back(2);
      l.push_back(3);
      // <char,list>
      std::pair<char,std::list<int> > p4('x',l);
      CHECK_SERIALIZATION("(x, [1, 2, 3])",p4)
    }

    void test_map() {
      using namespace GaudiUtils;
      
      std::map<int,int> m1; CHECK_SERIALIZATION("{}",m1)
      m1[1] = 1; CHECK_SERIALIZATION("{1: 1}",m1)
      m1[2] = 4;
      m1[3] = 9;
      CHECK_SERIALIZATION("{1: 1, 2: 4, 3: 9}",m1)
      
      std::map<std::string,int> m2;
      m2["a"] = 1; CHECK_SERIALIZATION("{a: 1}",m2)
      m2["b"] = 4;
      m2["c"] = 9;
      CHECK_SERIALIZATION("{a: 1, b: 4, c: 9}",m2)
    }

    void test_gaudiutils_map() {
      //using namespace GaudiUtils;
      GaudiUtils::Map<std::string,int> m1; CHECK_SERIALIZATION("{}",m1)
      m1["a"] = 1; CHECK_SERIALIZATION("{a: 1}",m1)
      m1["b"] = 4;
      m1["c"] = 9;
      CHECK_SERIALIZATION("{a: 1, b: 4, c: 9}",m1)
    }

    void test_gaudiutils_hashmap() {
      //using namespace GaudiUtils;
      GaudiUtils::HashMap<std::string,int> m1; CHECK_SERIALIZATION("{}",m1)
      m1["a"] = 1; CHECK_SERIALIZATION("{a: 1}",m1)
      m1["b"] = 4;
      m1["c"] = 9;
      CHECK_SERIALIZATION("{a: 1, b: 4, c: 9}",m1)
    }

    void setUp() {}
    
    void tearDown() {}

  };
  
  CPPUNIT_TEST_SUITE_REGISTRATION( SerializeSTL );

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
    ProgressListener(): m_lastTestFailed(false) {}
    
    /// Destructor.
    virtual ~ProgressListener() {}
    
    void startTest( CppUnit::Test *test )
    {
      std::cout << test->getName();
      std::cout.flush();      
      m_lastTestFailed = false;
    }

    void addFailure( const CppUnit::TestFailure &failure )
    {
      std::cout << " : " << (failure.isError() ? "error" : "assertion");
      m_lastTestFailed  = true;
    }
    
    void endTest( CppUnit::Test * /*test*/ )
    {
      if ( !m_lastTestFailed )
        std::cout  <<  " : OK";
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
  std::string testPath =
    (argc > 1) ? std::string(argv[1]) : std::string("");

  // Add a listener that collects test result
  //CppUnit::TestResultCollector result;
  //controller.addListener( &result );

  /// Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  /// Adds the test to the list of test to run
  // CppUnit::TestRunner runner;
  CppUnit::TextTestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  // uncomment the following line if you need a compiler outputter.
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), 
                                                       std::cout ) );

  // Change the default outputter to a xml error format outputter
  // uncomment the following line if you need a xml outputter.
  //runner.setOutputter( new CppUnit::XmlOutputter( &runner.result(),
  //                                                    std::cout ) );

  runner.eventManager().addListener( new GaudiKernelTest::ProgressListener() );

  //CppUnit::TestResultCollector *collector = 
  //  new CppUnit::TestResultCollector();
  //runner.eventManager().addListener(collector);

  bool wasSuccessful = false;

  try 
  {
    wasSuccessful = runner.run( testPath, false, true, false );
  }

  // Test path not resolved
  catch ( std::invalid_argument &e )
  {
    std::cout  << std::endl << "ERROR: " << e.what() << std::endl;
    return 0;
  }

  // Should never happen?
  catch ( std::exception& e )
  {
    std::cout  << std::endl
               << "UNEXPECTED STD EXCEPTION CAUGHT: "
               << e.what() << std::endl;
    return 0;
  }

  // Should never happen?
  catch ( ... )
  {
    std::cout  << std::endl
               << "UNKNOWN EXCEPTION CAUGHT" << std::endl;
    return 0;
  }

  // Return error code 1 if the one of tests failed.
  // Print a message on standard error if something failed (for QMTest)
  if ( ! wasSuccessful ) std::cerr << "Error: CppUnit Failures" << std::endl;
  int retcode = wasSuccessful ? 0 : 1;
  
  // Uncomment the next line if you want to integrate CppUnit with Oval
  // std::cout << "[OVAL] Cppunit-result =" << retcode << std::endl;
  return retcode;

}

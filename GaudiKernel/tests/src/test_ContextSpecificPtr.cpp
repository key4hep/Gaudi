#include "GaudiKernel/ContextSpecificPtr.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace Gaudi::Hive;

BOOST_AUTO_TEST_CASE( basic_usage )
{
  ContextSpecificPtr<int> p;

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( !p.isValid() );

  p = new int( 42 );

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 42 );
  int* p0 = p.get();

  setCurrentContextId( 1 );

  BOOST_CHECK( !p.isValid() );

  p = new int( 23 );

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 23 );
  int* p1 = p.get();

  BOOST_CHECK( p0 != p1 );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_REQUIRE( p.isValid() );
  BOOST_CHECK( *p == 42 );

  int sum = p.accumulate( []( const int* x ) -> int { return *x; }, 0 );
  BOOST_CHECK( sum == ( 42 + 23 ) );

  int prod = p.accumulate( []( const int* x ) -> int { return *x; }, 1, []( int a, int b ) -> int { return a * b; } );
  BOOST_CHECK( prod == ( 42 * 23 ) );

  p.deleteAll();
}

BOOST_AUTO_TEST_CASE( ctx_data )
{
  ContextSpecificData<int> i;

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( i == 0 );

  i = 42;
  BOOST_CHECK( i == 42 );
  int i0 = i;

  setCurrentContextId( 1 );

  BOOST_CHECK( i == 0 );

  i = 23;
  BOOST_CHECK( i == 23 );
  int i1 = i;

  BOOST_CHECK( i0 != i1 );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( i == 42 );

  /*
  int sum = p.accumulate([](const int*x) -> int {return *x;}, 0);
  BOOST_CHECK( sum == (42 + 23) );

  int prod = p.accumulate([](const int*x) -> int {return *x;}, 1,
      [](int a, int b) -> int {return a * b;});
  BOOST_CHECK( prod == (42 * 23) );
  */
}

BOOST_AUTO_TEST_CASE( ctx_data_with_proto )
{
  ContextSpecificData<int> i( 123 );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( i == 123 );

  i = 42;
  BOOST_CHECK( i == 42 );
  int i0 = i;

  setCurrentContextId( 1 );

  BOOST_CHECK( i == 123 );

  i = 23;
  BOOST_CHECK( i == 23 );
  int i1 = i;

  BOOST_CHECK( i0 != i1 );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( i == 42 );

  BOOST_CHECK( i.accumulate( 0 ) == ( 42 + 23 ) );
  BOOST_CHECK( i.accumulate( 1, std::multiplies<>() ) == ( 42 * 23 ) );
}

class TestClass
{
public:
  TestClass( int _a = 0, int _b = 0 ) : a( _a ), b( _b ) {}
  int a, b;
};

BOOST_AUTO_TEST_CASE( ctx_complex_data )
{
  ContextSpecificData<TestClass> i;
  ContextSpecificData<TestClass> j( TestClass( 1, 2 ) );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( ( (TestClass&)i ).a == 0 );
  BOOST_CHECK( ( (TestClass&)i ).b == 0 );

  BOOST_CHECK( ( (TestClass&)j ).a == 1 );
  BOOST_CHECK( ( (TestClass&)j ).b == 2 );

  i = TestClass( 3, 4 );

  setCurrentContextId( 1 );

  BOOST_CHECK( ( (TestClass&)i ).a == 0 );
  BOOST_CHECK( ( (TestClass&)i ).b == 0 );

  BOOST_CHECK( ( (TestClass&)j ).a == 1 );
  BOOST_CHECK( ( (TestClass&)j ).b == 2 );

  setCurrentContextId( (ContextIdType)0 );

  BOOST_CHECK( ( (TestClass&)i ).a == 3 );
  BOOST_CHECK( ( (TestClass&)i ).b == 4 );

  BOOST_CHECK( ( (TestClass&)j ).a == 1 );
  BOOST_CHECK( ( (TestClass&)j ).b == 2 );
}

#include <chrono>
#include <iostream>
#include <thread>

class ThreadingTest
{
public:
  ContextSpecificData<int> data;
  void set( int v ) { data = v; }
};

class Runner
{
public:
  Runner( ThreadingTest& t ) : test( t ) {}

  void operator()( ContextIdType c )
  {
    setCurrentContextId( c );
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    test.set( c );
  }

  ThreadingTest& test;
};

BOOST_AUTO_TEST_CASE( ctx_threaded )
{
  ThreadingTest test;
  Runner runner( test );
  std::thread t1( runner, (ContextIdType)1 );
  std::thread t2( runner, (ContextIdType)2 );
  t1.join();
  t2.join();

  BOOST_CHECK( test.data.accumulate( 0 ) == 3 );
  BOOST_CHECK( test.data.accumulate( 1, std::multiplies<int>() ) == 2 );
}

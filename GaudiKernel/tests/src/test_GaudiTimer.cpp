#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_GaudiTimer
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <string>
#include <thread>

#include "Gaudi/Timers.h"

// Measure same time interval with different clocks
BOOST_AUTO_TEST_CASE( timer ) {
  Gaudi::Timer     timerA;
  Gaudi::FastTimer timerB;
  {
    auto t1 = timerA();
    auto t2 = timerB();
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  }
  {
    auto t1 = timerA();
    auto t2 = timerB();
    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
  }

  std::cout << "timerA: " << timerA.stats() << std::endl;
  std::cout << "timerB: " << timerB.stats() << std::endl;
}

// Measure timer overhead
template <typename T>
void perfTimer( const std::string& label, int N ) {
  T            timerA;
  Gaudi::Timer timeit;
  {
    auto twall = timeit();
    for ( int i = 0; i < N; ++i ) { auto t = timerA(); }
  }
  std::cout << std::left << std::setw( 20 ) << label << timeit.stats().mean() << " us per " << N << " calls"
            << std::endl;
}

BOOST_AUTO_TEST_CASE( timer_overhead ) {
  const int N = 10000;
  perfTimer<Gaudi::Timer>( "Gaudi::Timer", N );
  perfTimer<Gaudi::FastTimer>( "Gaudi::FastTimer", N );
}

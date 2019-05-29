#include "Gaudi/Chrono/Counters.h"
#include "GaudiKernel/Counters.h"

#include <chrono>
#include <iostream>

using namespace Gaudi::Accumulators;

int main() {

  // Testing Buffer moving
  {
    Counter<double, atomicity::full> c;
    c += 3.5;
    c += 1.2;
    {
      auto buf = c.buffer();
      buf += 7.2; // buf1 internal count = 1
      decltype( buf ) buf2( std::move( buf ) );
      buf2 += 3.5; // buf3 internal count = 2
      decltype( buf ) buf3 = std::move( buf2 );
      buf3 += 3.7; // buf3 internal count = 3
    }
    std::cout << c << std::endl; // should output 5
  }

  // Testing counter resets
  {
    SigmaCounter<> sig;
    sig += 3;
    sig += 5;
    sig += 6;
    std::cout << sig << std::endl;
    sig.reset();
    std::cout << sig << std::endl;
  }

  // Testing AveragingCounter
  {
    AveragingCounter<> avg;
    avg += 3;
    avg += 5;
    avg += 6;
    std::cout << avg << std::endl;
  }

  // Testing integer AveragingCounter
  {
    AveragingCounter<int> avg;
    avg += 3;
    avg += 5;
    avg += 6;
    std::cout << avg << std::endl;

    static_assert( std::is_integral_v<decltype( avg.mean<int>() )>, "Integral return type expected" );
    static_assert( std::is_floating_point_v<decltype( avg.mean() )>, "Floating point return type expected" );
  }

  // Testing SigmaCounter
  {
    SigmaCounter<> sig;
    sig += 3;
    sig += 5;
    sig += 6;
    std::cout << sig << std::endl;
  }

  // Testing AveragingCounter with buffering
  {
    AveragingCounter<float, atomicity::full> avg2;
    {
      auto bufAvg = avg2.buffer();
      for ( int i = 0; i < 1000; i++ ) bufAvg += i;
    }
    std::cout << avg2 << std::endl;
  }

  // Testing Binomialcounter
  {
    BinomialCounter<> bin;
    bin += false;
    bin += true;
    bin += true;
    bin += false;
    bin += false;
    std::cout << bin << std::endl;
  }

  // Testing Binomialcounter with buffers
  {
    BinomialCounter<> bin;
    {
      auto bufBin = bin.buffer();
      bufBin += false;
      bufBin += true;
      bufBin += true;
      bufBin += false;
      bufBin += false;
    }
    std::cout << bin << std::endl;
  }

  // Testing StatEntity, the backward compatible counter
  {
    StatEntity se;
    se += 3;
    se += 5;
    se += 6;
    std::cout << se << std::endl;
  }

  // Testing StatEntity, the backward compatible counter with binomial usage
  {
    StatEntity sb;
    sb += 0;
    sb += 1;
    sb += 1;
    sb += 0;
    sb += 0;
    std::cout << sb << std::endl;
  }

  // Testing StatEntity, setting directly values
  {
    StatEntity sb{3, 14, 70, 3, 6};
    std::cout << sb << std::endl;
  }

  // Testing Counters of std::chrono::duration
  {
    typedef std::chrono::milliseconds  Unit;
    StatCounter<Unit, atomicity::full> c;
    c += Unit( 5 );
    c += Unit( 3 );
    c += std::chrono::seconds( 1 ); // mixing with larger units is OK
    std::cout << c << std::endl;
    // Check compilation of all stat methods:
    static_assert( std::is_same_v<decltype( c.mean() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.standard_deviation() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.unbiased_sample_variance() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.biased_sample_variance() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.meanErr() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.min() ), Unit>, "Return type should be Unit" );
    static_assert( std::is_same_v<decltype( c.max() ), Unit>, "Return type should be Unit" );
  }
}

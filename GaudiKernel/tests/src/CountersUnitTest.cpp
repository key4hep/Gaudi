#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyHolder
#include <boost/test/unit_test.hpp>

#include "Gaudi/Accumulators.h"
#include "Gaudi/Chrono/Counters.h"
#include "GaudiKernel/StatEntity.h"

#include <chrono>

using namespace Gaudi::Accumulators;

namespace utf = boost::unit_test;
namespace tt  = boost::test_tools;

BOOST_AUTO_TEST_CASE( test_buffer_moving, *utf::tolerance( 1e-14 ) ) {
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

  BOOST_TEST( c.nEntries() == 5 );
  BOOST_TEST( c.toString() == "#=5      " );
}

BOOST_AUTO_TEST_CASE( test_counter_resets, *utf::tolerance( 1e-14 ) ) {
  SigmaCounter<> sig;
  sig += 3;
  sig += 5;
  sig += 6;

  BOOST_TEST( sig.nEntries() == 3 );
  BOOST_TEST( sig.sum() == 14 );
  BOOST_TEST( sig.mean() == 4.666666666666667 );
  BOOST_TEST( sig.standard_deviation() == 1.2472191289246459 );
  BOOST_TEST( sig.toString() == "#=3       Sum=14          Mean=     4.667 +- 1.2472    " );

  sig.reset();

  BOOST_TEST( sig.nEntries() == 0 );
  BOOST_TEST( sig.sum() == 0, tt::tolerance( 0. ) );
  BOOST_TEST( sig.mean() == 0, tt::tolerance( 0. ) );
  BOOST_TEST( sig.standard_deviation() == 0, tt::tolerance( 0. ) );
  BOOST_TEST( sig.toString() == "#=0       Sum=0           Mean=     0.000 +- 0.0000    " );
}

BOOST_AUTO_TEST_CASE( test_AveragingCounter, *utf::tolerance( 1e-14 ) ) {
  AveragingCounter<> avg;
  avg += 3;
  avg += 5;
  avg += 6;

  BOOST_TEST( avg.nEntries() == 3 );
  BOOST_TEST( avg.sum() == 14 );
  BOOST_TEST( avg.mean() == 4.666666666666667 );
  BOOST_TEST( avg.toString() == "#=3       Sum=14          Mean=     4.667" );
}

BOOST_AUTO_TEST_CASE( test_integer_AveragingCounter, *utf::tolerance( 1e-14 ) ) {
  AveragingCounter<int> avg;
  avg += 3;
  avg += 5;
  avg += 6;

  BOOST_TEST( avg.nEntries() == 3 );
  BOOST_TEST( avg.sum() == 14 );
  BOOST_TEST( avg.mean() == 4.666666666666667 );
  BOOST_TEST( avg.toString() == "#=3       Sum=14          Mean=     4.667" );

  BOOST_TEST( std::is_integral_v<decltype( avg.mean<int>() )>, "Integral return type expected" );
  BOOST_TEST( std::is_floating_point_v<decltype( avg.mean() )>, "Floating point return type expected" );
}

BOOST_AUTO_TEST_CASE( test_SigmaCounter, *utf::tolerance( 1e-14 ) ) {
  SigmaCounter<> sig;
  sig += 3;
  sig += 5;
  sig += 6;

  BOOST_TEST( sig.nEntries() == 3 );
  BOOST_TEST( sig.sum() == 14 );
  BOOST_TEST( sig.mean() == 4.666666666666667 );
  BOOST_TEST( sig.standard_deviation() == 1.2472191289246459 );
  BOOST_TEST( sig.toString() == "#=3       Sum=14          Mean=     4.667 +- 1.2472    " );
}

BOOST_AUTO_TEST_CASE( test_AveragingCounter_with_buffering, *utf::tolerance( 1e-14 ) ) {
  AveragingCounter<float, atomicity::full> avg2;
  {
    auto bufAvg = avg2.buffer();
    for ( int i = 0; i < 1000; i++ ) bufAvg += i;
  }

  BOOST_TEST( avg2.nEntries() == 1000 );
  BOOST_TEST( avg2.sum() == 499500 );
  BOOST_TEST( avg2.mean() == 499.5 );
  BOOST_TEST( avg2.toString() == "#=1000    Sum=4.995e+05   Mean=     499.5" );
}

BOOST_AUTO_TEST_CASE( test_Binomialcounter, *utf::tolerance( 1e-14 ) ) {
  BinomialCounter<> bin;
  bin += false;
  bin += true;
  bin += true;
  bin += false;
  bin += false;

  BOOST_TEST( bin.nEntries() == 5 );
  BOOST_TEST( bin.nTrueEntries() == 2 );
  BOOST_TEST( bin.eff() == 0.40 );
  BOOST_TEST( bin.effErr() == 0.21908902300206642 );
  BOOST_TEST( bin.toString() == "#=5       Sum=2           Eff=|( 40.00000 +- 21.9089 )%|" );
}

BOOST_AUTO_TEST_CASE( test_Binomialcounter_with_buffers, *utf::tolerance( 1e-14 ) ) {
  BinomialCounter<> bin;
  {
    auto bufBin = bin.buffer();
    bufBin += false;
    bufBin += true;
    bufBin += true;
    bufBin += false;
    bufBin += false;
  }

  BOOST_TEST( bin.nEntries() == 5 );
  BOOST_TEST( bin.nTrueEntries() == 2 );
  BOOST_TEST( bin.eff() == 0.40 );
  BOOST_TEST( bin.effErr() == 0.21908902300206642 );
  BOOST_TEST( bin.toString() == "#=5       Sum=2           Eff=|( 40.00000 +- 21.9089 )%|" );
}

BOOST_AUTO_TEST_CASE( test_bulk_increment_with_Binomialcounter, *utf::tolerance( 1e-14 ) ) {

  BinomialCounter<> bin;
  // bin += { .nPass = 4, .nTotal = 10 }; // C++20 only...
  bin += {4, 10};
  bin += {1, 9};
  bin += {0, 1};

  BOOST_TEST( bin.nEntries() == 20 );
  BOOST_TEST( bin.nTrueEntries() == 5 );
  BOOST_TEST( bin.eff() == 0.25 );
  BOOST_TEST( bin.effErr() == 0.096824583655185426 );
  BOOST_TEST( bin.toString() == "#=20      Sum=5           Eff=|( 25.00000 +- 9.68246 )%|" );
}

BOOST_AUTO_TEST_CASE( test_StatEntity, *utf::tolerance( 1e-14 ) ) // the backward compatible counter
{
  StatEntity se;
  se += 3;
  se += 5;
  se += 6;

  BOOST_TEST( se.nEntries() == 3 );
  BOOST_TEST( se.sum() == 14 );
  BOOST_TEST( se.mean() == 4.666666666666667 );
  BOOST_TEST( se.rms() == 1.2472191289246459 );
  BOOST_TEST( se.min() == 3 );
  BOOST_TEST( se.max() == 6 );
  BOOST_TEST( se.toString() ==
              "#=3       Sum=14          Mean=     4.667 +- 1.2472     Min/Max=     3.000/6.000     " );
}

BOOST_AUTO_TEST_CASE( test_StatEntity_binomial,
                      *utf::tolerance( 1e-14 ) ) // the backward compatible counter with binomial usage
{
  StatEntity sb;
  sb += 0;
  sb += 1;
  sb += 1;
  sb += 0;
  sb += 0;

  BOOST_TEST( sb.nEntries() == 5 );
  BOOST_TEST( sb.sum() == 2 );
  BOOST_TEST( sb.mean() == 0.40, tt::tolerance( 1e-14 ) );
  BOOST_TEST( sb.rms() == 0.4898979485566356 );
  BOOST_TEST( sb.min() == 0 );
  BOOST_TEST( sb.max() == 1 );
  BOOST_TEST( sb.toString() ==
              "#=5       Sum=2           Mean=    0.4000 +- 0.48990    Min/Max=     0.000/1.000     " );
}

BOOST_AUTO_TEST_CASE( test_StatEntity_direct_set, *utf::tolerance( 1e-14 ) ) {
  StatEntity sb{3, 14, 70, 3, 6};

  BOOST_TEST( sb.nEntries() == 3 );
  BOOST_TEST( sb.sum() == 14 );
  BOOST_TEST( sb.mean() == 4.666666666666667 );
  BOOST_TEST( sb.rms() == 1.2472191289246459 );
  BOOST_TEST( sb.min() == 3 );
  BOOST_TEST( sb.max() == 6 );
  BOOST_TEST( sb.toString() ==
              "#=3       Sum=14          Mean=     4.667 +- 1.2472     Min/Max=     3.000/6.000     " );
}

using Unit = std::chrono::milliseconds;
template <typename T>
inline constexpr bool is_Unit_v = std::is_same_v<T, Unit>;

BOOST_AUTO_TEST_CASE( test_Counters_duration ) {
  StatCounter<Unit, atomicity::full> c;
  c += Unit( 5 );
  c += Unit( 3 );
  c += std::chrono::seconds( 1 ); // mixing with larger units is OK

  BOOST_TEST( c.toString() == "#=3       Sum=1008ms      Mean=     336ms +- 470ms      Min/Max=       0ms/1000ms    " );

  // Check compilation of all stat methods:
  BOOST_TEST( is_Unit_v<decltype( c.mean() )>, "mean return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.standard_deviation() )>, "standard_deviation return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.unbiased_sample_variance() )>,
              "unbiased_sample_variance return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.biased_sample_variance() )>, "biased_sample_variance return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.meanErr() )>, "meanErr return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.min() )>, "min return type should be Unit" );
  BOOST_TEST( is_Unit_v<decltype( c.max() )>, "max return type should be Unit" );
}

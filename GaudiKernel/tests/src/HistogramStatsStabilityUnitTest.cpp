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
#define BOOST_TEST_MODULE test_HistogramStatsStability

#include <Gaudi/Accumulators/StaticHistogram.h>
#include <Gaudi/Histograming/Sink/Utils.h>

#include <boost/test/unit_test.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

  // Mock code for the test
  struct MonitoringHub : Gaudi::Monitoring::Hub {};
  struct ServiceLocator {
    MonitoringHub& monitoringHub() { return m_monitHub; }
    MonitoringHub  m_monitHub{};
  };
  struct Algo {
    ServiceLocator* serviceLocator() { return &m_serviceLocator; }
    std::string     name() { return ""; }
    ServiceLocator  m_serviceLocator{};
  };
  struct HistSink : public Gaudi::Monitoring::Hub::Sink {
    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override { m_entities.push_back( ent ); }
    void removeEntity( Gaudi::Monitoring::Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_entities ), end( m_entities ), ent );
      if ( it != m_entities.end() ) { m_entities.erase( it ); }
    }
    std::deque<Gaudi::Monitoring::Hub::Entity> m_entities;
  };

  using Gaudi::Histograming::Sink::detail::WeightedMoments;

  /// Reference weighted central moments, by the two pass algorithm in long double
  struct Reference {
    long double mean{ 0 }, variance{ 0 }, skewness{ 0 }, kurtosis{ 0 };
  };
  Reference reference( const std::vector<std::pair<double, double>>& data ) {
    auto sumW  = 0.0L;
    auto sumWX = 0.0L;
    for ( const auto& [x, w] : data ) {
      sumW += w;
      sumWX += ( w * static_cast<long double>( x ) );
    }
    Reference r;
    r.mean  = ( sumWX / sumW );
    auto m2 = 0.0L;
    auto m3 = 0.0L;
    auto m4 = 0.0L;
    for ( const auto& [x, w] : data ) {
      const auto d  = ( static_cast<long double>( x ) - r.mean );
      const auto d2 = ( d * d );
      m2 += ( w * d2 );
      m3 += ( ( w * d2 ) * d );
      m4 += ( ( w * d2 ) * d2 );
    }
    r.variance = ( m2 / sumW );
    r.skewness = ( ( m3 / sumW ) / ( r.variance * std::sqrt( r.variance ) ) );
    r.kurtosis = ( ( ( m4 / sumW ) / ( r.variance * r.variance ) ) - 3.0L );
    return r;
  }
} // namespace

// The moments agree with the two pass reference for a well conditioned distribution, whatever the order
BOOST_AUTO_TEST_CASE( test_weighted_moments_against_reference ) {
  auto                                   gen    = std::mt19937_64{ 12345 };
  auto                                   gamma  = std::gamma_distribution<double>{ 2.0, 1.5 };
  auto                                   weight = std::uniform_real_distribution<double>{ 0.01, 100.0 };
  std::vector<std::pair<double, double>> data;
  for ( int i = 0; i < 1000; ++i ) { data.emplace_back( gamma( gen ), weight( gen ) ); }
  const auto ref = reference( data );
  for ( int order = 0; order < 3; ++order ) {
    WeightedMoments m;
    for ( const auto& [x, w] : data ) { m.add( x, w ); }
    BOOST_CHECK_CLOSE( m.mean(), static_cast<double>( ref.mean ), 1e-10 );
    BOOST_CHECK_CLOSE( m.variance(), static_cast<double>( ref.variance ), 1e-9 );
    BOOST_CHECK_CLOSE( m.skewness(), static_cast<double>( ref.skewness ), 1e-8 );
    BOOST_CHECK_CLOSE( m.kurtosis(), static_cast<double>( ref.kurtosis ), 1e-8 );
    std::shuffle( data.begin(), data.end(), gen );
  }
}

// A distribution far from zero but narrow relative to that distance, where raw sums of powers cancel completely
BOOST_AUTO_TEST_CASE( test_weighted_moments_far_from_zero ) {
  auto                                   gen    = std::mt19937_64{ 54321 };
  auto                                   gauss  = std::normal_distribution<double>{ 0.0, 1e-6 };
  auto                                   weight = std::uniform_real_distribution<double>{ 0.5, 2.0 };
  std::vector<std::pair<double, double>> data;
  for ( int i = 0; i < 1000; ++i ) { data.emplace_back( ( 1000.0 + gauss( gen ) ), weight( gen ) ); }
  const auto      ref = reference( data );
  WeightedMoments m;
  for ( const auto& [x, w] : data ) { m.add( x, w ); }
  BOOST_CHECK_CLOSE( m.mean(), static_cast<double>( ref.mean ), 1e-10 );
  BOOST_CHECK_CLOSE( m.variance(), static_cast<double>( ref.variance ), 1e-4 );
  BOOST_CHECK_CLOSE( m.skewness(), static_cast<double>( ref.skewness ), 1e-2 );
  BOOST_CHECK_CLOSE( m.kurtosis(), static_cast<double>( ref.kurtosis ), 1e-2 );
}

// A single populated bin has no width, whatever the weight: the statistics are exactly zero, not rounding noise
BOOST_AUTO_TEST_CASE( test_weighted_moments_single_value ) {
  const auto x = 0.001;
  auto       w = 6.0e7;
  for ( int i = 0; i < 1000; ++i ) {
    WeightedMoments m;
    m.add( x, w );
    BOOST_CHECK_EQUAL( m.mean(), x );
    BOOST_CHECK_EQUAL( m.stddev(), 0.0 );
    BOOST_CHECK_EQUAL( m.skewness(), 0.0 );
    BOOST_CHECK_EQUAL( m.kurtosis(), 0.0 );
    w = std::nextafter( w, 1e9 );
  }
  // ignores empty and negative weights
  WeightedMoments m;
  m.add( 1.0, 0.0 );
  m.add( 2.0, -1.0 );
  BOOST_CHECK_EQUAL( m.sumW(), 0.0 );
  BOOST_CHECK_EQUAL( m.stddev(), 0.0 );
}

// ---------------------------------------------------------------------------------------------------------------
// The printed statistics of every kind of histogram: Histogram, WeightedHistogram, ProfileHistogram and
// WeightedProfileHistogram, in 1, 2 and 3 dimensions, and with integer arithmetic. They are printed by
// printHistogramND from the json of the histogram, and the profile histograms also by printProfileHistoND from the
// monitoring entity, and all of them are checked.
// ---------------------------------------------------------------------------------------------------------------
namespace {
  namespace Acc = Gaudi::Accumulators;
  using Acc::atomicity;
  using Gaudi::Histograming::Sink::printHistogram1D;
  using Gaudi::Histograming::Sink::printHistogram2D;
  using Gaudi::Histograming::Sink::printHistogram3D;
  using Gaudi::Histograming::Sink::printProfileHisto1D;
  using Gaudi::Histograming::Sink::printProfileHisto2D;
  using Gaudi::Histograming::Sink::printProfileHisto3D;

  /// Keeps the mock algorithm and the sink that receives the entities of the histograms
  struct Fixture {
    Fixture() { algo.serviceLocator()->monitoringHub().addSink( &sink ); }
    Algo     algo;
    HistSink sink;
  };

  // NB bins from 0 to 0.002 * NB: the first bin has its centre at 0.001, the value of the origin time in the MC of the
  // test that showed the problem (100 bins from 0 to 0.2 by default, fewer for the 2D and 3D histograms of the no width
  // tests to keep them fast). For integer arithmetic NB bins from 0 to NB, and the value 5.
  constexpr double nearZero = 0.001;
  constexpr double axisMax  = 0.2;
  template <typename Arith>
  constexpr Arith origin() {
    return ( std::is_integral_v<Arith> ? Arith( 5 ) : Arith( nearZero ) );
  }
  template <typename Arith, unsigned int NB>
  constexpr Arith axisEnd() {
    return ( std::is_integral_v<Arith> ? Arith( NB ) : Arith( 0.002 * NB ) );
  }
  /// Fewer bins per axis for the 2D and 3D histograms of the no width tests: the cost is in their bins
  constexpr unsigned int binsOfNoWidthTests( const unsigned int n ) { return ( ( n == 1 ) ? 100 : 10 ); }
  template <unsigned int N, typename Histo, typename Arith = double, unsigned int NB = 100>
  Histo makeHisto( Fixture& f, const std::string& name ) {
    using Ax = Acc::Axis<Arith>;
    const Ax axis{ NB, Arith( 0 ), axisEnd<Arith, NB>() };
    if constexpr ( N == 1 ) {
      return Histo{ &f.algo, name, name, axis };
    } else if constexpr ( N == 2 ) {
      return Histo{ &f.algo, name, name, axis, axis };
    } else {
      return Histo{ &f.algo, name, name, axis, axis, axis };
    }
  }

  /// The printed line of a histogram, from its json
  template <typename Histo>
  std::string jsonLine( const Histo& h ) {
    const nlohmann::json j     = h;
    const auto           type  = j.at( "type" ).template get<std::string>();
    const auto           title = j.at( "title" ).template get<std::string>();
    switch ( j.at( "dimension" ).template get<int>() ) {
    case 1:
      return printHistogram1D( type, "H", title, j );
    case 2:
      return printHistogram2D( type, "H", title, j );
    default:
      return printHistogram3D( type, "H", title, j );
    }
  }

  /// The printed line of a profile histogram, from its monitoring entity
  template <unsigned int N, atomicity A, typename Arith>
  std::string entityLine( const Fixture& f ) {
    BOOST_REQUIRE_EQUAL( f.sink.m_entities.size(), 1 );
    const auto& ent = f.sink.m_entities.front();
    if constexpr ( N == 1 ) {
      return printProfileHisto1D<A, Arith>( "H", ent );
    } else if constexpr ( N == 2 ) {
      return printProfileHisto2D<A, Arith>( "H", ent );
    } else {
      return printProfileHisto3D<A, Arith>( "H", ent );
    }
  }

  struct Printed {
    std::array<double, 3> mean{ 0, 0, 0 }, rms{ 0, 0, 0 };
    double                skewness{ 0 }, kurtosis{ 0 };
  };

  /// Parse the numbers of a printed line. Only the first nd axes are filled.
  Printed parse( const std::string& line, const unsigned int nd ) {
    Printed p;
    if ( nd == 1 ) {
      // " | ID | title | entries | mean | rms | skewness | kurtosis |"
      std::vector<std::string> fields;
      std::stringstream        ss( line );
      for ( std::string field; std::getline( ss, field, '|' ); ) { fields.push_back( field ); }
      BOOST_REQUIRE_GE( fields.size(), 8 );
      p.mean[0]  = std::stod( fields[4] );
      p.rms[0]   = std::stod( fields[5] );
      p.skewness = std::stod( fields[6] );
      p.kurtosis = std::stod( fields[7] );
    } else {
      // "<X>/sX=mean/rms,<Y>/sY=mean/rms,..."
      const char axes[] = { 'X', 'Y', 'Z' };
      for ( unsigned int a = 0; a < nd; ++a ) {
        const std::regex re( std::string( "<" ) + axes[a] + ">/s" + axes[a] + "=([-+0-9.eE]+)/([-+0-9.eE]+)" );
        std::smatch      m;
        BOOST_REQUIRE_MESSAGE( std::regex_search( line, m, re ), line );
        p.mean[a] = std::stod( m[1] );
        p.rms[a]  = std::stod( m[2] );
      }
    }
    return p;
  }

  /// Check that the lines of all the variants are the same, and that there is no width: zero RMS on every axis and
  /// for 1D zero skewness and kurtosis
  void checkNoWidth( const std::vector<std::string>& lines, const unsigned int nd ) {
    BOOST_REQUIRE( !lines.empty() );
    for ( const auto& line : lines ) { BOOST_CHECK_EQUAL( line, lines.front() ); }
    const auto p = parse( lines.front(), nd );
    for ( unsigned int a = 0; a < nd; ++a ) {
      BOOST_CHECK_CLOSE( p.mean[a], nearZero, 1e-2 );
      BOOST_CHECK_MESSAGE( p.rms[a] == 0.0, lines.front() );
    }
    if ( nd == 1 ) {
      BOOST_CHECK_MESSAGE( p.skewness == 0.0, lines.front() );
      BOOST_CHECK_MESSAGE( p.kurtosis == 0.0, lines.front() );
    }
  }

  /// Fill one entry at the point where all axes have the origin value
  template <unsigned int N, typename Arith = double, typename Histo, typename Value>
  void fillAtOrigin( Histo& h, Value&& value ) {
    constexpr Arith o = origin<Arith>();
    if constexpr ( N == 1 ) {
      h[o] += std::forward<Value>( value );
    } else if constexpr ( N == 2 ) {
      h[{ o, o }] += std::forward<Value>( value );
    } else {
      h[{ o, o, o }] += std::forward<Value>( value );
    }
  }

  // The kinds of histograms by their fill value: nothing (a count), a weight, a Y value, or a Y value and a weight
  template <unsigned int N, atomicity A = atomicity::full, typename Arith = double>
  void checkDegenerateCounting() {
    // the same every time, whatever the number of entries
    std::vector<std::string> lines;
    for ( unsigned int variant = 0; variant < 5; ++variant ) {
      Fixture f;
      auto    h = makeHisto<N, Acc::StaticHistogram<N, A, Arith>, Arith, binsOfNoWidthTests( N )>( f, "H" );
      for ( unsigned int i = 0; i < ( 1000 + ( 77 * variant ) ); ++i ) { fillAtOrigin<N, Arith>( h, Arith{ 1 } ); }
      lines.push_back( jsonLine( h ) );
    }
    // the number of entries differs between the variants, so compare the statistics only
    for ( const auto& line : lines ) {
      const auto p = parse( line, N );
      for ( unsigned int a = 0; a < N; ++a ) { BOOST_CHECK_MESSAGE( p.rms[a] == 0.0, line ); }
      if ( N == 1 ) { BOOST_CHECK_MESSAGE( ( ( p.skewness == 0.0 ) && ( p.kurtosis == 0.0 ) ), line ); }
    }
  }

  template <unsigned int N>
  void checkDegenerateWeighted() {
    std::vector<std::string> lines;
    for ( unsigned int variant = 0; variant < 8; ++variant ) {
      Fixture f;
      auto    h   = makeHisto<N, Acc::StaticWeightedHistogram<N>, double, binsOfNoWidthTests( N )>( f, "H" );
      auto    gen = std::mt19937_64{ ( 1000 + variant ) };
      auto    w   = std::uniform_real_distribution<double>{ 0.1, 1.0 };
      for ( int i = 0; i < 5000; ++i ) { fillAtOrigin<N>( h, w( gen ) ); }
      lines.push_back( jsonLine( h ) );
    }
    // the sums of the weights differ in the last bits (and more) between the variants: only the number of entries is
    // common, so compare the parsed statistics
    for ( const auto& line : lines ) {
      const auto p = parse( line, N );
      for ( unsigned int a = 0; a < N; ++a ) { BOOST_CHECK_MESSAGE( p.rms[a] == 0.0, line ); }
      if ( N == 1 ) { BOOST_CHECK_MESSAGE( ( ( p.skewness == 0.0 ) && ( p.kurtosis == 0.0 ) ), line ); }
    }
  }

  template <unsigned int N>
  void checkDegenerateProfile() {
    std::vector<std::string> jsonLines, entityLines;
    for ( unsigned int variant = 0; variant < 8; ++variant ) {
      Fixture f;
      auto    h   = makeHisto<N, Acc::StaticProfileHistogram<N>, double, binsOfNoWidthTests( N )>( f, "H" );
      auto    gen = std::mt19937_64{ ( 2000 + variant ) };
      auto    y   = std::uniform_real_distribution<double>{ 10.0, 30.0 };
      for ( int i = 0; i < 5000; ++i ) { fillAtOrigin<N>( h, y( gen ) ); }
      jsonLines.push_back( jsonLine( h ) );
      entityLines.push_back( entityLine<N, atomicity::full, double>( f ) );
    }
    for ( const auto& line : jsonLines ) {
      const auto p = parse( line, N );
      for ( unsigned int a = 0; a < N; ++a ) { BOOST_CHECK_MESSAGE( p.rms[a] == 0.0, line ); }
      if ( N == 1 ) { BOOST_CHECK_MESSAGE( ( ( p.skewness == 0.0 ) && ( p.kurtosis == 0.0 ) ), line ); }
    }
    checkNoWidth( entityLines, N );
  }

  template <unsigned int N>
  void checkDegenerateWeightedProfile() {
    std::vector<std::string> lines;
    for ( unsigned int variant = 0; variant < 8; ++variant ) {
      Fixture f;
      auto    h   = makeHisto<N, Acc::StaticWeightedProfileHistogram<N>, double, binsOfNoWidthTests( N )>( f, "H" );
      auto    gen = std::mt19937_64{ ( 3000 + variant ) };
      auto    y   = std::uniform_real_distribution<double>{ 10.0, 30.0 };
      auto    w   = std::uniform_real_distribution<double>{ 0.1, 1.0 };
      for ( int i = 0; i < 5000; ++i ) { fillAtOrigin<N>( h, std::make_pair( y( gen ), w( gen ) ) ); }
      lines.push_back( jsonLine( h ) );
    }
    for ( const auto& line : lines ) {
      const auto p = parse( line, N );
      for ( unsigned int a = 0; a < N; ++a ) { BOOST_CHECK_MESSAGE( p.rms[a] == 0.0, line ); }
      if ( N == 1 ) { BOOST_CHECK_MESSAGE( ( ( p.skewness == 0.0 ) && ( p.kurtosis == 0.0 ) ), line ); }
    }
  }
} // namespace

// All the entries of a histogram in one bin have no width, whatever the kind of histogram, number of dimensions and
// the last bits of the sums in the bin: this is the case that gave rounding noise, 1e-11 for the RMS and values of
// order 1e8 and 1e16 for the skewness and kurtosis.
BOOST_AUTO_TEST_CASE( test_no_width_histogram ) {
  checkDegenerateCounting<1>();
  checkDegenerateCounting<2>();
  checkDegenerateCounting<3>();
}
BOOST_AUTO_TEST_CASE( test_no_width_histogram_integer ) {
  // integer arithmetic takes the average of the integers in a bin as its value
  checkDegenerateCounting<1, atomicity::full, int>();
  checkDegenerateCounting<2, atomicity::full, int>();
}
BOOST_AUTO_TEST_CASE( test_no_width_weighted_histogram ) {
  checkDegenerateWeighted<1>();
  checkDegenerateWeighted<2>();
  checkDegenerateWeighted<3>();
}
BOOST_AUTO_TEST_CASE( test_no_width_profile_histogram ) {
  checkDegenerateProfile<1>();
  checkDegenerateProfile<2>();
  checkDegenerateProfile<3>();
}
BOOST_AUTO_TEST_CASE( test_no_width_weighted_profile_histogram ) {
  // the kind of the histogram of the Panoptes test that showed the problem, printed from its json
  checkDegenerateWeightedProfile<1>();
  checkDegenerateWeightedProfile<2>();
  checkDegenerateWeightedProfile<3>();
}

// A histogram with a distribution, with all the entries at the bin centres so that the exact values are known: the
// printed numbers are those of the two pass reference, to the 5 digits that are printed
BOOST_AUTO_TEST_CASE( test_distribution_histograms ) {
  // 1D: counts, weighted counts, and (weighted) profiles, with a different distribution for each
  auto gen    = std::mt19937_64{ 777 };
  auto y      = std::uniform_real_distribution<double>{ 1.0, 10.0 };
  auto w      = std::uniform_real_distribution<double>{ 0.5, 2.0 };
  auto center = []( const unsigned int bin ) { return ( 0.0 + ( ( ( bin + 0.5 ) * axisMax ) / 100.0 ) ); };

  auto check1D = [&]( const std::vector<std::pair<double, double>>& data, const std::string& line ) {
    const auto ref = reference( data );
    const auto p   = parse( line, 1 );
    BOOST_CHECK_CLOSE( p.mean[0], static_cast<double>( ref.mean ), 1e-2 );
    BOOST_CHECK_CLOSE( p.rms[0], static_cast<double>( std::sqrt( ref.variance ) ), 1e-2 );
    BOOST_CHECK_CLOSE( p.skewness, static_cast<double>( ref.skewness ), 1e-2 );
    BOOST_CHECK_CLOSE( p.kurtosis, static_cast<double>( ref.kurtosis ), 1e-2 );
  };

  {
    // counts
    Fixture                                f;
    auto                                   h = makeHisto<1, Acc::StaticHistogram<1>>( f, "H" );
    std::vector<std::pair<double, double>> data;
    for ( unsigned int bin = 5; bin < 60; ++bin ) {
      const auto n = ( 1 + ( ( bin * 37 ) % 11 ) );
      for ( unsigned int i = 0; i < n; ++i ) { ++h[center( bin )]; }
      data.emplace_back( center( bin ), n );
    }
    check1D( data, jsonLine( h ) );
  }
  {
    // weights
    Fixture                                f;
    auto                                   h = makeHisto<1, Acc::StaticWeightedHistogram<1>>( f, "H" );
    std::vector<std::pair<double, double>> data;
    for ( unsigned int bin = 5; bin < 60; ++bin ) {
      const auto weight = w( gen );
      h[center( bin )] += weight;
      data.emplace_back( center( bin ), weight );
    }
    check1D( data, jsonLine( h ) );
  }
  {
    // profile: the bins are weighted by the sum of the Y values in them
    Fixture                                f;
    auto                                   h = makeHisto<1, Acc::StaticProfileHistogram<1>>( f, "H" );
    std::vector<std::pair<double, double>> data;
    for ( unsigned int bin = 5; bin < 60; ++bin ) {
      double sumY = 0;
      for ( int i = 0; i < 20; ++i ) {
        const auto v = y( gen );
        h[center( bin )] += v;
        sumY += v;
      }
      data.emplace_back( center( bin ), sumY );
    }
    check1D( data, jsonLine( h ) );
    check1D( data, entityLine<1, atomicity::full, double>( f ) );
  }
  {
    // weighted profile
    Fixture                                f;
    auto                                   h = makeHisto<1, Acc::StaticWeightedProfileHistogram<1>>( f, "H" );
    std::vector<std::pair<double, double>> data;
    for ( unsigned int bin = 5; bin < 60; ++bin ) {
      double sumYW = 0;
      for ( int i = 0; i < 20; ++i ) {
        const auto v = y( gen ), weight = w( gen );
        h[center( bin )] += { v, weight };
        sumYW += ( v * weight );
      }
      data.emplace_back( center( bin ), sumYW );
    }
    check1D( data, jsonLine( h ) );
  }

  // 2D and 3D, mean and RMS of each axis, with a different distribution on each axis
  auto checkND = [&]( const std::array<std::vector<std::pair<double, double>>, 3>& axes, const std::string& line,
                      const unsigned int nd ) {
    const auto p = parse( line, nd );
    for ( unsigned int a = 0; a < nd; ++a ) {
      const auto ref = reference( axes[a] );
      BOOST_CHECK_CLOSE( p.mean[a], static_cast<double>( ref.mean ), 1e-2 );
      BOOST_CHECK_CLOSE( p.rms[a], static_cast<double>( std::sqrt( ref.variance ) ), 1e-2 );
    }
  };
  {
    // 2D counts on a grid of bins (20 per axis), the number in a bin a function of the X and the Y bin
    constexpr unsigned int nBins = 20;
    Fixture                f;
    auto                   h = makeHisto<2, Acc::StaticHistogram<2>, double, nBins>( f, "H" );
    std::array<std::vector<std::pair<double, double>>, 3> axes;
    std::array<std::array<double, nBins>, 3>              sums{};
    for ( unsigned int bx = 2; bx < 8; ++bx ) {
      for ( unsigned int by = 8; by < 18; ++by ) {
        const auto n = ( 1 + ( ( ( bx * 3 ) + ( by * 5 ) ) % 7 ) );
        for ( unsigned int i = 0; i < n; ++i ) { ++h[{ center( bx ), center( by ) }]; }
        sums[0][bx] += n;
        sums[1][by] += n;
      }
    }
    for ( unsigned int b = 0; b < nBins; ++b ) {
      if ( sums[0][b] > 0 ) { axes[0].emplace_back( center( b ), sums[0][b] ); }
      if ( sums[1][b] > 0 ) { axes[1].emplace_back( center( b ), sums[1][b] ); }
    }
    checkND( axes, jsonLine( h ), 2 );
  }
  {
    // 3D counts (10 bins per axis)
    constexpr unsigned int nBins = 10;
    Fixture                f;
    auto                   h = makeHisto<3, Acc::StaticHistogram<3>, double, nBins>( f, "H" );
    std::array<std::vector<std::pair<double, double>>, 3> axes;
    std::array<std::array<double, nBins>, 3>              sums{};
    for ( unsigned int bx = 1; bx < 5; ++bx ) {
      for ( unsigned int by = 3; by < 8; ++by ) {
        for ( unsigned int bz = 3; bz < 9; bz += 2 ) {
          const auto n = ( 1 + ( ( ( ( bx * 3 ) + ( by * 5 ) ) + ( bz * 2 ) ) % 5 ) );
          for ( unsigned int i = 0; i < n; ++i ) { ++h[{ center( bx ), center( by ), center( bz ) }]; }
          sums[0][bx] += n;
          sums[1][by] += n;
          sums[2][bz] += n;
        }
      }
    }
    for ( unsigned int b = 0; b < nBins; ++b ) {
      for ( unsigned int a = 0; a < 3; ++a ) {
        if ( sums[a][b] > 0 ) { axes[a].emplace_back( center( b ), sums[a][b] ); }
      }
    }
    checkND( axes, jsonLine( h ), 3 );
  }
}

/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_CounterHistos

#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Accumulators/HistogramArray.h>
#include <Gaudi/Accumulators/StaticHistogram.h>

#include <GaudiKernel/PropertyHolder.h>

#include <boost/test/unit_test.hpp>

#include <iostream>

namespace {

  // Mock code for the test
  struct MonitoringHub : Gaudi::Monitoring::Hub {};
  struct ServiceLocator {
    MonitoringHub& monitoringHub() { return m_monitHub; }
    MonitoringHub  m_monitHub{};
  };
  struct BaseAlgo : INamedInterface, IProperty {
    unsigned long            addRef() const override { return 1; };
    unsigned long            release() const override { return 1; };
    void const*              i_cast( const InterfaceID& ) const override { return nullptr; }
    std::vector<std::string> getInterfaceNames() const override { return {}; }
    unsigned long            refCount() const override { return 1; }
    unsigned long            decRef() const override { return 1; }
    StatusCode               queryInterface( const InterfaceID&, void** ) override { return StatusCode::FAILURE; };
    const std::string&       name() const override { return m_name; };
    std::string              m_name{};
  };
  struct Algo : PropertyHolder<BaseAlgo> {
    ServiceLocator*            serviceLocator() { return &m_serviceLocator; }
    ServiceLocator             m_serviceLocator{};
    void                       registerCallBack( Gaudi::StateMachine::Transition, std::function<void()> ) {}
    Gaudi::StateMachine::State FSMState() const { return Gaudi::StateMachine::CONFIGURED; }
  };

  // Little helper for using automatic nlohmann conversion mechanism
  template <typename T>
  nlohmann::json toJSON( T const& t ) {
    nlohmann::json j = t;
    return t;
  }
} // namespace

BOOST_AUTO_TEST_CASE( test_static_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    // testing an array of 5 1D, regular histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticHistogram<1>, 5> histo1d{
        &algo, "SGaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[2] ).at( "title" ) == "A Gaudi 1D histogram - number 2" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }

  {
    // testing an array of 7 2D, weighted histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticWeightedHistogram<2>, 7> histo2dw{
        &algo, "SName{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.25; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.25 );
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.5; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.75 );
  }

  {
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticHistogram<1>, 5> histo1d{
        &algo,
        []( int n ) { return fmt::format( "SGaudiH1D-{}-{}", n, n ^ 2 ); },
        [nb = 5]( int n ) {
          return fmt::format( "Title number {} of Histogram arrays of {} histograms in total", n, nb );
        },
        { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[3] ).at( "title" ) == "Title number 3 of Histogram arrays of 5 histograms in total" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }
}

BOOST_AUTO_TEST_CASE( test_large_static_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo                  algo;
  constexpr std::size_t N = 100;
  {
    // testing an array of 100 1D, regular histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticHistogram<1>, N> histo1d{
        &algo, "SGaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < N; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < N; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    for ( unsigned int i = 0; i < N; i++ ) {
      BOOST_TEST( toJSON( histo1d[i] ).at( "title" ) == fmt::format( "A Gaudi 1D histogram - number {}", i ) );
      ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
      BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 2 );
    }
  }

  {
    // testing an array of 100 2D, weighted histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticWeightedHistogram<2>, 100> histo2dw{
        &algo, "SName{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
    for ( unsigned int i = 0; i < N; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.25; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < N; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.25 );
    for ( unsigned int i = 0; i < N; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.5; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < N; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.75 );
  }

  {
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::StaticHistogram<1>, 100> histo1d{
        &algo,
        []( int n ) { return fmt::format( "SGaudiH1D-{}-{}", n, n ^ 2 ); },
        [nb = N]( int n ) {
          return fmt::format( "Title number {} of Histogram arrays of {} histograms in total", n, nb );
        },
        { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < N; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < N; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    for ( unsigned int i = 0; i < N; i++ ) {
      BOOST_TEST( toJSON( histo1d[i] ).at( "title" ) ==
                  fmt::format( "Title number {} of Histogram arrays of {} histograms in total", i, N ) );
      ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
      BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 2 );
    }
  }
}

BOOST_AUTO_TEST_CASE( test_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    // testing an array of 5 1D, regular histograms, with "standard" names / titles
    // Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
        &algo, "GaudiH1D-{}", "WRONG TITLE !", { 1, -1, 1, "WRONG" } };
    for ( unsigned int i = 0; i < 5; i++ ) {
      algo.setProperty( fmt::format( "GaudiH1D_{}_Title", i ), fmt::format( "A Gaudi 1D histogram - number {}", i ) )
          .ignore();
      algo.setProperty( fmt::format( "GaudiH1D_{}_Axis0", i ), "( 21, -10.5, 10.5, \"X\" )" ).ignore();
    }
    for ( unsigned int i = 0; i < 5; i++ ) { histo1d[i].createHistogram( algo ); }
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[2] ).at( "title" ) == "A Gaudi 1D histogram - number 2" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }

  {
    // testing an array of 7 2D, weighted histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::WeightedHistogram<2>, 7> histo2dw{
        &algo, "Name{}", "WRONG TITLE !", { 1, -1, 1, "WRONG" }, { 1, -1, 1, "WRONG" } };
    for ( unsigned int i = 0; i < 7; i++ ) {
      algo.setProperty( fmt::format( "Name{}_Title", i ), fmt::format( "Title {}", i ) ).ignore();
      algo.setProperty( fmt::format( "Name{}_Axis0", i ), "( 21, -10.5, 10.5, \"X\" )" ).ignore();
      algo.setProperty( fmt::format( "Name{}_Axis1", i ), "( 21, -10.5, 10.5, \"Y\" )" ).ignore();
    }
    for ( unsigned int i = 0; i < 7; i++ ) { histo2dw[i].createHistogram( algo ); }
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.25; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.25 );
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.5; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.75 );
  }

  {
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
        &algo,
        []( int n ) { return fmt::format( "GaudiH1D-{}-{}", n, n * n ); },
        [nb = 5]( int n ) {
          return fmt::format( "Title number {} of Histogram arrays of {} histograms in total", n, nb );
        },
        { 1, -1, 1, "WRONG" } };
    for ( unsigned int i = 0; i < 5; i++ ) {
      algo.setProperty( fmt::format( "GaudiH1D_{}_{}_Axis0", i, i * i ), "( 21, -10.5, 10.5, \"X\" )" ).ignore();
    }
    for ( unsigned int i = 0; i < 5; i++ ) { histo1d[i].createHistogram( algo ); }
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[3] ).at( "title" ) == "Title number 3 of Histogram arrays of 5 histograms in total" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }
}

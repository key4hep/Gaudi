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
#include <Gaudi/Accumulators/HistogramMap.h>
#include <Gaudi/Accumulators/StaticHistogram.h>

#include <GaudiKernel/PropertyHolder.h>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

namespace {

  // Mock code for the test
  struct MonitoringHub : Gaudi::Monitoring::Hub {};
  struct ServiceLocator {
    MonitoringHub& monitoringHub() { return m_monitHub; }
    MonitoringHub  m_monitHub{};
  };
  struct BaseAlgo : INamedInterface, IProperty {
    unsigned long            addRef() override { return 0; };
    unsigned long            release() override { return 0; };
    void*                    i_cast( const InterfaceID& ) const override { return nullptr; }
    std::vector<std::string> getInterfaceNames() const override { return {}; }
    unsigned long            refCount() const override { return 0; }
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
    // testing a map int to 1D, regular histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::StaticHistogram<1>> histo1d{
        &algo, "SGaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[2] ).at( "title" ) == "A Gaudi 1D histogram - number 2" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }

  {
    // testing a map string to 1D, regular histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramMap<std::string, Gaudi::Accumulators::StaticHistogram<1>> histo1d{
        &algo, "SGaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
    ++histo1d["one"][-10.0];
    ++histo1d["two"][-10.0];
    ++histo1d["three"][-10.0];
    BOOST_TEST( toJSON( histo1d["one"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["two"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["three"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["four"] ).at( "bins" )[1] == 0 );
    BOOST_TEST( toJSON( histo1d["two"] ).at( "title" ) == "A Gaudi 1D histogram - number two" );
    ++histo1d["three"][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d["three"] ).at( "bins" )[1] == 2 );
  }

  {
    // testing a map int to 2D, weighted histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::StaticWeightedHistogram<2>> histo2dw{
        &algo, "SName{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.25; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.25 );
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.5; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.75 );
  }

  {
    // testing a map int to 1D, regular histograms, with non standard names / titles
    Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::StaticHistogram<1>> histo1d{
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

  {
    // testing a map string to 1D, regular histograms, with non standard names / titles
    Gaudi::Accumulators::HistogramMap<std::string, Gaudi::Accumulators::StaticHistogram<1>> histo1d{
        &algo,
        []( std::string const& s ) { return fmt::format( "SGaudiH1D-{}-{}", s, s ); },
        []( std::string const& s ) { return fmt::format( "Title number {} of Histogram arrays", s ); },
        { 21, -10.5, 10.5, "X" } };
    ++histo1d["one"][-10.0];
    ++histo1d["two"][-10.0];
    ++histo1d["three"][-10.0];
    BOOST_TEST( toJSON( histo1d["one"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["two"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["three"] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d["four"] ).at( "bins" )[1] == 0 );
    BOOST_TEST( toJSON( histo1d["three"] ).at( "title" ) == "Title number three of Histogram arrays" );
    ++histo1d["three"][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d["three"] ).at( "bins" )[1] == 2 );
  }

  {
    // testing a map pair<int, string> to 1D, regular histograms, with non standard names / titles
    Gaudi::Accumulators::HistogramMap<std::pair<int, std::string>, Gaudi::Accumulators::StaticHistogram<1>> histo1d{
        &algo,
        []( std::pair<int, std::string> const& p ) { return fmt::format( "GaudiH1D-{}-{}", p.first, p.second ); },
        []( std::pair<int, std::string> const& p ) { return fmt::format( "Title {} ({})", p.second, p.first ); },
        { 21, -10.5, 10.5, "X" } };
    ++histo1d[{ 1, "one" }][-10.0];
    ++histo1d[{ 2, "two" }][-10.0];
    ++histo1d[{ 3, "three" }][-10.0];
    BOOST_TEST( toJSON( histo1d[{ 1, "one" }] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[{ 2, "two" }] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[{ 3, "three" }] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[{ 4, "four" }] ).at( "bins" )[1] == 0 );
    BOOST_TEST( toJSON( histo1d[{ 3, "three" }] ).at( "title" ) == "Title three (3)" );
    ++histo1d[{ 3, "three" }][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[{ 3, "three" }] ).at( "bins" )[1] == 2 );
  }
}

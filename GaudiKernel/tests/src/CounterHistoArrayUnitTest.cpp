/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <Gaudi/Accumulators/HistogramArray.h>

#include <boost/test/unit_test.hpp>

#include <deque>
#include <iostream>

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
    virtual void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override { m_entities.push_back( ent ); }
    virtual void removeEntity( Gaudi::Monitoring::Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_entities ), end( m_entities ), ent );
      if ( it != m_entities.end() ) m_entities.erase( it );
    }
    std::deque<Gaudi::Monitoring::Hub::Entity> m_entities;
  };

  // Little helper for using automatic nlohmann conversion mechanism
  template <typename T>
  nlohmann::json toJSON( T const& t ) {
    nlohmann::json j = t;
    return t;
  }
} // namespace

BOOST_AUTO_TEST_CASE( test_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    // testing an array of 5 1D, regular histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
        &algo, "GaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
    for ( unsigned int i = 0; i < 5; i++ ) ++histo1d[i][-10.0]; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( toJSON( histo1d[i] ).at( "bins" )[1] == 1 );
    BOOST_TEST( toJSON( histo1d[2] ).at( "title" ) == "A Gaudi 1D histogram - number 2" );
    ++histo1d[3][-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d[3] ).at( "bins" )[1] == 2 );
  }

  {
    // testing an array of 7 2D, weighted histograms, with "standard" names / titles
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::WeightedHistogram<2>, 7> histo2dw{
        &algo, "Name{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.25; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.25 );
    for ( unsigned int i = 0; i < 7; i++ ) histo2dw[i][{ -10.0, -10.0 }] += 0.5; // fill the first (non-overflow) bin
    for ( unsigned int i = 0; i < 7; i++ ) BOOST_TEST( toJSON( histo2dw[i] ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 0.75 );
  }

  {
    Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
        &algo,
        []( int n ) { return fmt::format( "GaudiH1D-{}-{}", n, n ^ 2 ); },
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

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
#define BOOST_TEST_MODULE test_ProfileHistoSpeed

#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Histograming/Sink/Utils.h>

#include <boost/test/unit_test.hpp>

#include <chrono>
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

BOOST_AUTO_TEST_CASE( test_profile_histo_speed, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;
  std::cout << "Elapsed time (us) :\n";
  // create a large profile (1M bins) histogram and fill it with 10 million items
  auto                                      startTime = std::chrono::steady_clock::now();
  Gaudi::Accumulators::ProfileHistogram<1u> histo{
      &algo, "GaudiP1D", "A Large Gaudi 1D Profile", { 1000000, 0, 1000000 } };
  for ( int i = 0; i < 10000000; i++ ) { histo[0.1 * double( i ) + 0.05] += double( i * 0.01 ); }
  auto filledTime = std::chrono::steady_clock::now();
  std::cout << "  creation & filling   : "
            << std::chrono::duration_cast<std::chrono::microseconds>( filledTime - startTime ).count() << "\n";

  // convert to json
  nlohmann::json j             = toJSON( histo );
  auto           convertedTime = std::chrono::steady_clock::now();
  std::cout << "  json convertion      : "
            << std::chrono::duration_cast<std::chrono::microseconds>( convertedTime - filledTime ).count() << "\n";

  // retrieve bins
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 10000000 );
  auto const& bincont       = j.at( "bins" ).get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>();
  auto        retrievedTime = std::chrono::steady_clock::now();
  std::cout << "  retrieval            : "
            << std::chrono::duration_cast<std::chrono::microseconds>( retrievedTime - convertedTime ).count() << "\n";

  // check all bins
  for ( size_t w = 0; w < bincont.size() - 2; w++ ) {
    auto& [c, sumw2]  = bincont[w + 1];
    auto [nent, sumw] = c;
    BOOST_TEST( nent == 10 );
    BOOST_TEST( sumw == w + 0.45 );
    BOOST_TEST( sumw2 == 0.01 * ( 10 * w * w + 9 * w + 2.85 ) );
  }
  auto endTestTime = std::chrono::steady_clock::now();
  std::cout << "  testing              : "
            << std::chrono::duration_cast<std::chrono::microseconds>( endTestTime - retrievedTime ).count() << "\n";

  // test fast conversion to ROOT
  Gaudi::Monitoring::Hub::Entity ent{ "TestCase", "GaudiP1D", "histogram:ProfileHistogram", histo };
  auto                           fastConvertionStartTime = std::chrono::steady_clock::now();
  auto                           rootHisto = Gaudi::Histograming::Sink::profileHisto1DToRoot( ent.name, ent );
  auto                           fastConvertionEndTime = std::chrono::steady_clock::now();
  std::cout << "  fast root conversion : "
            << std::chrono::duration_cast<std::chrono::microseconds>( fastConvertionEndTime - fastConvertionStartTime )
                   .count()
            << "\n";

  // check all bins
  for ( size_t i = 1; i <= 1000000; i++ ) {
    auto w = i - 1;
    BOOST_TEST( rootHisto.GetBinEntries( i ) == 10 );
    BOOST_TEST( rootHisto.GetBinContent( i ) * 10 == w + 0.45 );
    BOOST_TEST( rootHisto.getBinW2( i ) == 0.01 * ( 10 * w * w + 9 * w + 2.85 ) );
  }
  auto fastConvertionEndTestTime = std::chrono::steady_clock::now();
  std::cout << "  testing              : "
            << std::chrono::duration_cast<std::chrono::microseconds>( fastConvertionEndTestTime -
                                                                      fastConvertionEndTime )
                   .count()
            << "\n";

  // test fast conversion of 2D histos
  Gaudi::Accumulators::ProfileHistogram<2u> histo2d{
      &algo, "GaudiP3D", "A Large Gaudi 2D Profile", { 1000, 0, 1000 }, { 1000, 0, 1000 } };
  for ( int i = 0; i < 1000; i++ ) {
    for ( int j = 0; j < 1000; j++ ) {
      for ( int l = 0; l < 10; l++ ) { histo2d[{ i + 0.5, j + 0.5 }] += double( l ); }
    }
  }
  Gaudi::Monitoring::Hub::Entity ent2d{ "TestCase", "GaudiP2D", "histogram:ProfileHistogram", histo2d };
  fastConvertionStartTime = std::chrono::steady_clock::now();
  auto rootHisto2d        = Gaudi::Histograming::Sink::profileHisto2DToRoot( ent2d.name, ent2d );
  fastConvertionEndTime   = std::chrono::steady_clock::now();
  std::cout << "  fast root 2D conversion : "
            << std::chrono::duration_cast<std::chrono::microseconds>( fastConvertionEndTime - fastConvertionStartTime )
                   .count()
            << "\n";

  // test fast conversion of 3D histos
  Gaudi::Accumulators::ProfileHistogram<3u> histo3d{ &algo,           "GaudiP3D",      "A Large Gaudi 3D Profile",
                                                     { 100, 0, 100 }, { 100, 0, 100 }, { 100, 0, 100 } };
  for ( int i = 0; i < 100; i++ ) {
    for ( int j = 0; j < 100; j++ ) {
      for ( int k = 0; k < 100; k++ ) {
        for ( int l = 0; l < 10; l++ ) { histo3d[{ i + 0.5, j + 0.5, k + 0.5 }] += double( l ); }
      }
    }
  }
  Gaudi::Monitoring::Hub::Entity ent3d{ "TestCase", "GaudiP3D", "histogram:ProfileHistogram", histo3d };
  fastConvertionStartTime = std::chrono::steady_clock::now();
  auto rootHisto3d        = Gaudi::Histograming::Sink::profileHisto3DToRoot( ent3d.name, ent3d );
  fastConvertionEndTime   = std::chrono::steady_clock::now();
  std::cout << "  fast root 3D conversion : "
            << std::chrono::duration_cast<std::chrono::microseconds>( fastConvertionEndTime - fastConvertionStartTime )
                   .count()
            << "\n";
}

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
#include <Gaudi/Accumulators/Histogram.h>

#include "LogHistogram.h"

#include <boost/test/unit_test.hpp>

#include <deque>
#include <iostream>

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

BOOST_AUTO_TEST_CASE( test_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    Gaudi::Accumulators::Histogram<1> histo1d{ &algo, "GaudiH1D", "A Gaudi 1D histogram", { 21, -10.5, 10.5, "X" } };
    ++histo1d[-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( histo1d.toJSON().at( "bins" )[1] == 1 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo1d += -10.0; // fill the first (non-overflow) bin
#pragma GCC diagnostic pop
    BOOST_TEST( histo1d.toJSON().at( "bins" )[1] == 2 );
  }
  {
    Gaudi::Accumulators::Histogram<2> histo2d{
        &algo, "GaudiH2D", "A Gaudi 2D histogram", { { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } } };
    ++histo2d[{ -10.0, -10.0 }]; // fill the first (non-overflow) bin
    BOOST_TEST( histo2d.toJSON().at( "bins" )[( 1 + 21 + 1 ) + 1] == 1 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo2d += { -10.0, -10.0 }; // fill the first (non-overflow) bin
#pragma GCC diagnostic pop
    BOOST_TEST( histo2d.toJSON().at( "bins" )[( 1 + 21 + 1 ) + 1] == 2 );
  }
  {
    Gaudi::Accumulators::WeightedHistogram<1> histo1dw{ &algo, "", "", { 21, -10.5, 10.5, "X" } };
    histo1dw[-10.0] += 0.25; // fill the first (non-overflow) bin
    BOOST_TEST( histo1dw.toJSON().at( "bins" )[1] == 0.25 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo1dw += { -10.0, 0.5 }; // fill the first (non-overflow) bin
    BOOST_TEST( histo1dw.toJSON().at( "bins" )[1] == 0.75 );
#pragma GCC diagnostic pop
    std::cout << histo1dw.toJSON() << std::endl;
  }

  Gaudi::Accumulators::ProfileHistogram<1u> histo{ &algo, "GaudiP1D", "A Gaudi 1D Profile", { 10, 0, 100 } };

  histo[-0.5] += -0.5;
  for ( int i = 0; i < 10; i++ ) { histo[10.0 * double( i ) + 0.5] += double( i ); }
  histo[120.0] += 120.0;

  nlohmann::json j        = histo.toJSON();
  auto           nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 12 );

  auto bincont = j.at( "bins" ).get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>();
  for ( size_t i = 0; i < bincont.size(); i++ ) {
    auto& [tmp, sumw2] = bincont[i];
    auto& [nent, sumw] = tmp;
    if ( i == 0 ) {
      BOOST_TEST( nent == 1 );
      BOOST_TEST( sumw == -0.5 );
      BOOST_TEST( sumw2 == 0.5 * 0.5 );
    } else if ( i == bincont.size() - 1 ) {
      BOOST_TEST( nent == 1 );
      BOOST_TEST( sumw == 120.0 );
      BOOST_TEST( sumw2 == 120.0 * 120.0 );
    } else {
      auto w = i - 1;
      BOOST_TEST( nent == 1 );
      BOOST_TEST( sumw == w );
      BOOST_TEST( sumw2 == w * w );
    }
  }
}

BOOST_AUTO_TEST_CASE( test_integer_histos ) {
  using namespace Gaudi::Accumulators;
  Algo                                                             algo;
  Gaudi::Accumulators::Histogram<1, atomicity::none, unsigned int> histo{
      &algo, "IntH1D", "A 1D histogram with integer content", { 10, 0, 10, "X" } };
  ++histo[1]; // fill the second (non-overflow) bin
  ++histo[3]; // fill the fourth (non-overflow) bin
  BOOST_TEST( histo.toJSON().at( "bins" )[0] == 0 );
  BOOST_TEST( histo.toJSON().at( "bins" )[2] == 1 );
  BOOST_TEST( histo.toJSON().at( "bins" )[4] == 1 );
}

BOOST_AUTO_TEST_CASE( test_integer_histos_small_ratio ) {
  using namespace Gaudi::Accumulators;
  Algo                                                             algo;
  Gaudi::Accumulators::Histogram<1, atomicity::none, unsigned int> histo{
      &algo, "IntH1D", "A 1D histogram with integer content", { 5, 0, 10, "X" } };
  ++histo[1]; // fill the first (non-overflow) bin
  ++histo[3]; // fill the second (non-overflow) bin
  BOOST_TEST( histo.toJSON().at( "bins" )[1] == 1 );
  BOOST_TEST( histo.toJSON().at( "bins" )[2] == 1 );
}

enum class TestEnum { A, B, C, D };

namespace Gaudi::Accumulators {
  template <>
  struct Axis<TestEnum> {
    // helper to make the code less verbose
    using storage_t = std::underlying_type_t<TestEnum>;
    // nothing to specify in the constructor as everything is fixed in the enum
    Axis() = default;

    unsigned int nBins    = 4;
    storage_t    minValue = static_cast<storage_t>( TestEnum::A ), maxValue = static_cast<storage_t>( TestEnum::D );
    std::string  title              = "TestEnum";
    std::vector<std::string> labels = { "A", "B", "C", "D" };
    // convert the enum value to the index in the bins, taking into account the underflow bin
    unsigned int index( TestEnum val ) const { return static_cast<storage_t>( val ) + 1; }
  };
} // namespace Gaudi::Accumulators

BOOST_AUTO_TEST_CASE( test_custom_axis ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  // note that for default constructible axis, we have to specify {{}} otherwise
  // it is interpreted as an empty array of axes (instead of using the constructor for a single axis)
  Histogram<1, atomicity::full, TestEnum> hist{ &algo, "TestEnumHist", "TestEnum histogram", Axis<TestEnum>{} };

  hist[TestEnum::A] += 1;
  ++hist[TestEnum::B];
  hist[TestEnum::C] += 2;

  auto j = hist.toJSON();

  auto bins = j["bins"];
  BOOST_TEST( bins[0] == 0 );
  BOOST_TEST( bins[1] == 1 );
  BOOST_TEST( bins[2] == 1 );
  BOOST_TEST( bins[3] == 2 );
  BOOST_TEST( bins[4] == 0 );
  BOOST_TEST( bins[5] == 0 );

  BOOST_TEST( j["axis"][0]["nBins"] == 4 );
  BOOST_TEST( j["axis"][0]["title"] == "TestEnum" );

  nlohmann::json expected_labels = { "A", "B", "C", "D" };
  BOOST_TEST( j["axis"][0]["labels"] == expected_labels );
}

BOOST_AUTO_TEST_CASE( test_custom ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  LogHistogram<1> hist{ &algo, "TestLogHist", "TestLog histogram", { 4, 0, 4 } };

  ++hist[.1];
  ++hist[1.];
  ++hist[3.];
  ++hist[9.];
  ++hist[32456789.];

  auto j = hist.toJSON();

  auto bins = j["bins"];
  BOOST_TEST( bins[0] == 1 );
  BOOST_TEST( bins[1] == 1 );
  BOOST_TEST( bins[2] == 1 );
  BOOST_TEST( bins[3] == 1 );
  BOOST_TEST( bins[4] == 0 );
  BOOST_TEST( bins[5] == 1 );

  BOOST_TEST( j["axis"][0]["nBins"] == 4 );
}

BOOST_AUTO_TEST_CASE( test_custom_2d ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  LogHistogram<2, atomicity::none, float> hist{
      &algo, "TestLogHist", "TestLog histogram", { { 2, 0, 2 }, { 2, 0, 2 } } };

  ++hist[{ 2, 2 }];
  ++hist[{ 9, .1 }];
  ++hist[{ .2, .3 }];

  auto j    = hist.toJSON();
  auto bins = j["bins"];
  BOOST_TEST( bins[0] == 1 );
  BOOST_TEST( bins[1] == 0 );
  BOOST_TEST( bins[2] == 0 );
  BOOST_TEST( bins[3] == 1 );
  BOOST_TEST( bins[4] == 0 );
  BOOST_TEST( bins[5] == 1 );
  for ( unsigned int i = 6; i < 16; i++ ) { BOOST_TEST( bins[i] == 0 ); }
  BOOST_TEST( j["axis"][0]["nBins"] == 2 );
  BOOST_TEST( j["axis"][1]["nBins"] == 2 );
}

BOOST_AUTO_TEST_CASE( test_histos_merge_reset, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo     algo;
  HistSink histSink;
  algo.serviceLocator()->monitoringHub().addSink( &histSink );

  Histogram<1, atomicity::full, float> hist1( &algo, "TestHist1", "Test histogram 1", Axis<float>{ 10, 0., 10. } );
  Histogram<1, atomicity::full, float> hist2( &algo, "TestHist2", "Test histogram 2", Axis<float>{ 10, 0., 10. } );

  // test all combinations of entities made (a) as part of the Histogram's constructor and (b) separately
  auto ent1a = histSink.m_entities[0];
  auto ent2a = histSink.m_entities[1];

  Gaudi::Monitoring::Hub::Entity ent1b( "", "TestHist1", std::string( "histogram:Histogram:" ) + typeid( float ).name(),
                                        hist1 );
  Gaudi::Monitoring::Hub::Entity ent2b( "", "TestHist2", std::string( "histogram:Histogram:" ) + typeid( float ).name(),
                                        hist2 );

  for ( int i = 0; i < 10; ++i ) {
    hist1[i] += i;
    hist2[i] += 2 * i;
  }

  ent1a.mergeAndReset( ent2a );
  BOOST_TEST( hist1.toJSON().at( "nEntries" ).get<unsigned long>() == 135 );
  BOOST_TEST( hist2.toJSON().at( "nEntries" ).get<unsigned long>() == 0 );
  ent2b.mergeAndReset( ent1b );
  BOOST_TEST( hist1.toJSON().at( "nEntries" ).get<unsigned long>() == 0 );
  BOOST_TEST( hist2.toJSON().at( "nEntries" ).get<unsigned long>() == 135 );
  ent1a.mergeAndReset( ent2b );
  BOOST_TEST( hist1.toJSON().at( "nEntries" ).get<unsigned long>() == 135 );
  BOOST_TEST( hist2.toJSON().at( "nEntries" ).get<unsigned long>() == 0 );
  ent2b.mergeAndReset( ent1a );
  BOOST_TEST( hist1.toJSON().at( "nEntries" ).get<unsigned long>() == 0 );
  BOOST_TEST( hist2.toJSON().at( "nEntries" ).get<unsigned long>() == 135 );
}

BOOST_AUTO_TEST_CASE( test_2d_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo algo;
  // test filling a 2D histogram with more bins in x than y
  // Buffer will overflow if the wrong axis' nBins is used to calculate the bin index, resulting in a double free
  Histogram<2, atomicity::full, float> hist{
      &algo, "Test2DHist", "Test 2D histogram", { 64, 0., 64. }, { 52, 0., 52. } };

  for ( int i = 0; i < 64; ++i ) {
    for ( int j = 0; j < 52; ++j ) { ++hist[{ i, j }]; }
  }

  auto j        = hist.toJSON();
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 64 * 52 );
}

BOOST_AUTO_TEST_CASE( test_2d_histos_unique_ptr, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo algo;
  // test constructing a 2D histogram inside a deque via emplace_back
  std::deque<Histogram<2, atomicity::full, float>> histos;
  histos.emplace_back( &algo, "Test2DHist", "Test 2D histogram", Axis<float>{ 10, 0., 10. },
                       Axis<float>{ 10, 0., 10. } );
  {
    auto buf = histos[0].buffer();
    for ( int i = 0; i < 10; ++i ) {
      for ( int j = 0; j < 10; ++j ) { ++buf[{ i, j }]; }
    }
  }

  auto j        = histos[0].toJSON();
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 100 );
}

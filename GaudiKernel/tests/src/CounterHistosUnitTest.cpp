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
#include <Gaudi/Accumulators/RootHistogram.h>

#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/StateMachine.h>

#include "LogHistogram.h"

#include <boost/test/unit_test.hpp>

#include <deque>
#include <iostream>
#include <vector>

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
    void*                    i_cast( const InterfaceID& ) const override { return nullptr; }
    std::vector<std::string> getInterfaceNames() const override { return {}; }
    unsigned long            refCount() const override { return 1; }
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
    Gaudi::Accumulators::StaticHistogram<1> histo1d{
        &algo, "GaudiH1D", "A Gaudi 1D histogram", { 21, -10.5, 10.5, "X" } };
    ++histo1d[-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[1] == 1 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo1d += -10.0; // fill the first (non-overflow) bin
#pragma GCC diagnostic pop
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[1] == 2 );
  }
  {
    Gaudi::Accumulators::StaticHistogram<2> histo2d{
        &algo, "GaudiH2D", "A Gaudi 2D histogram", { { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } } };
    ++histo2d[{ -10.0, -10.0 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 1 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo2d += { -10.0, -10.0 }; // fill the first (non-overflow) bin
#pragma GCC diagnostic pop
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 2 );
  }
  {
    Gaudi::Accumulators::StaticWeightedHistogram<1> histo1dw{ &algo, "", "", { 21, -10.5, 10.5, "X" } };
    histo1dw[-10.0] += 0.25; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1dw ).at( "bins" )[1] == 0.25 );
    BOOST_TEST( toJSON( histo1dw ).at( "nEntries" ).get<unsigned long>() == 1 );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    histo1dw += { -10.0, 0.5 }; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1dw ).at( "bins" )[1] == 0.75 );
    BOOST_TEST( toJSON( histo1dw ).at( "nEntries" ).get<unsigned long>() == 2 );
#pragma GCC diagnostic pop
  }

  Gaudi::Accumulators::StaticProfileHistogram<1u> histo{ &algo, "GaudiP1D", "A Gaudi 1D Profile", { 10, 0, 100 } };

  histo[-0.5] += -0.5;
  for ( int i = 0; i < 10; i++ ) { histo[10.0 * double( i ) + 0.5] += double( i ); }
  histo[120.0] += 120.0;

  nlohmann::json j        = toJSON( histo );
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

BOOST_AUTO_TEST_CASE( test_counter_root_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    Gaudi::Accumulators::StaticRootHistogram<1> histo1d{
        &algo, "GaudiH1D", "A Gaudi 1D histogram", { 4, -10, 10, "X" } };
    ++histo1d[1];
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[3] == 1 );
    ++histo1d[2];
    ++histo1d[-100];
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[3] == 2 );
    BOOST_TEST( toJSON( histo1d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo1d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo1d ).at( "sum" ) == 3 );
    BOOST_TEST( toJSON( histo1d ).at( "sum2" ) == 5 );
    BOOST_TEST( toJSON( histo1d ).at( "mean" ) == 1.5 );
    BOOST_TEST( toJSON( histo1d ).at( "standard_deviation" ) == 0.5 );
  }
  {
    Gaudi::Accumulators::StaticRootHistogram<2> histo2d{
        &algo, "GaudiH2D", "A Gaudi 2D histogram", { { 4, -10, 10, "X" }, { 4, -10, 10, "Y" } } };
    ++histo2d[{ 1, 1 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 4 + 1 ) * 3 + 3] == 1 );
    ++histo2d[{ 2, 3 }]; // fill the first (non-overflow) bin
    ++histo2d[{ -100, -100 }];
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 4 + 1 ) * 3 + 3] == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo2d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "sumx" ) == 3 );
    BOOST_TEST( toJSON( histo2d ).at( "sumy" ) == 4 );
    BOOST_TEST( toJSON( histo2d ).at( "sumx2" ) == 5 );
    BOOST_TEST( toJSON( histo2d ).at( "sumy2" ) == 10 );
    BOOST_TEST( toJSON( histo2d ).at( "sumxy" ) == 7 );
    BOOST_TEST( toJSON( histo2d ).at( "meanx" ) == 1.5 );
    BOOST_TEST( toJSON( histo2d ).at( "meany" ) == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "standard_deviationx" ) == .5 );
    BOOST_TEST( toJSON( histo2d ).at( "standard_deviationy" ) == 1 );
  }
  {
    Gaudi::Accumulators::StaticRootHistogram<3> histo3d{
        &algo, "GaudiH3D", "A Gaudi 3D histogram", { { 4, -10, 10, "X" }, { 4, -10, 10, "Y" }, { 4, -10, 10, "Z" } } };
    ++histo3d[{ 1, 1, 1 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo3d ).at( "bins" )[36 * 3 + 6 * 3 + 3] == 1 );
    ++histo3d[{ 2, 3, 2 }]; // fill the first (non-overflow) bin
    ++histo3d[{ -100, -100, -100 }];
    BOOST_TEST( toJSON( histo3d ).at( "bins" )[36 * 3 + 6 * 3 + 3] == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "sumx" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "sumy" ) == 4 );
    BOOST_TEST( toJSON( histo3d ).at( "sumz" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "sumx2" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumy2" ) == 10 );
    BOOST_TEST( toJSON( histo3d ).at( "sumz2" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumxy" ) == 7 );
    BOOST_TEST( toJSON( histo3d ).at( "sumxz" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumyz" ) == 7 );
    BOOST_TEST( toJSON( histo3d ).at( "meanx" ) == 1.5 );
    BOOST_TEST( toJSON( histo3d ).at( "meany" ) == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "meanz" ) == 1.5 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationx" ) == .5 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationy" ) == 1 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationz" ) == .5 );
  }
}

BOOST_AUTO_TEST_CASE( test_integer_histos ) {
  using namespace Gaudi::Accumulators;
  Algo                                                                   algo;
  Gaudi::Accumulators::StaticHistogram<1, atomicity::none, unsigned int> histo{
      &algo, "IntH1D", "A 1D histogram with integer content", { 10, 0, 10, "X" } };
  ++histo[1]; // fill the second (non-overflow) bin
  ++histo[3]; // fill the fourth (non-overflow) bin
  auto j = toJSON( histo );
  BOOST_TEST( j.at( "bins" )[0] == 0 );
  BOOST_TEST( j.at( "bins" )[2] == 1 );
  BOOST_TEST( j.at( "bins" )[4] == 1 );
}

BOOST_AUTO_TEST_CASE( test_integer_histos_small_ratio ) {
  using namespace Gaudi::Accumulators;
  Algo                                                                   algo;
  Gaudi::Accumulators::StaticHistogram<1, atomicity::none, unsigned int> histo{
      &algo, "IntH1D", "A 1D histogram with integer content", { 5, 0, 10, "X" } };
  ++histo[1]; // fill the first (non-overflow) bin
  ++histo[3]; // fill the second (non-overflow) bin
  auto j = toJSON( histo );
  BOOST_TEST( j.at( "bins" )[1] == 1 );
  BOOST_TEST( j.at( "bins" )[2] == 1 );
}

enum class TestEnum { A, B, C, D };
std::ostream& operator<<( std::ostream& o, TestEnum v ) {
  switch ( v ) {
  case TestEnum::A:
    o << 'A';
    break;
  case TestEnum::B:
    o << 'B';
    break;
  case TestEnum::C:
    o << 'C';
    break;
  case TestEnum::D:
    o << 'D';
    break;
  }
  return o;
}

namespace Gaudi::Accumulators {
  struct EnumAxis {
    using ArithmeticType = TestEnum;
    // helper to make the code less verbose
    using storage_t = std::underlying_type_t<TestEnum>;
    // nothing to specify in the constructor as everything is fixed in the enum
    EnumAxis() = default;

    unsigned int numBins() const { return 4; }
    storage_t    minValue = static_cast<storage_t>( TestEnum::A ), maxValue = static_cast<storage_t>( TestEnum::D );
    std::string  title              = "TestEnum";
    std::vector<std::string> labels = { "A", "B", "C", "D" };
    // convert the enum value to the index in the bins, taking into account the underflow bin
    unsigned int         index( TestEnum val ) const { return static_cast<storage_t>( val ) + 1; }
    friend std::ostream& operator<<( std::ostream& o, EnumAxis const& axis ) {
      return o << axis.numBins() << " " << axis.minValue << " " << axis.maxValue;
    }
  };
  void to_json( nlohmann::json& j, const EnumAxis& axis ) {
    j           = nlohmann::json{ { "nBins", axis.numBins() },
                                  { "minValue", axis.minValue },
                                  { "maxValue", axis.maxValue },
                                  { "title", axis.title } };
    j["labels"] = axis.labels;
  }
  StatusCode parse( EnumAxis&, const std::string& ) { return StatusCode::SUCCESS; }
} // namespace Gaudi::Accumulators

BOOST_AUTO_TEST_CASE( test_custom_axis ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  StaticHistogram<1, atomicity::full, TestEnum, std::tuple<EnumAxis>> hist{ &algo, "TestEnumHist", "TestEnum histogram",
                                                                            EnumAxis{} };

  hist[TestEnum::A] += 1;
  ++hist[TestEnum::B];
  hist[TestEnum::C] += 2;

  auto j = toJSON( hist );

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

BOOST_AUTO_TEST_CASE( test_mixed_axis ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  StaticHistogram<2, atomicity::full, float, std::tuple<EnumAxis, Axis<float>>> hist{
      &algo, "TestEnumHist", "TestEnum 2D histogram", EnumAxis{}, { 3, 0, 3, "Value" } };

  hist[{ TestEnum::A, 0.5 }] += 1;
  ++hist[{ TestEnum::B, 1.5 }];
  hist[{ TestEnum::C, 2.5 }] += 2;

  auto j = toJSON( hist );

  auto bins = j["bins"];
  BOOST_TEST( bins[6 + 1] == 1 );
  BOOST_TEST( bins[6 * 2 + 2] == 1 );
  BOOST_TEST( bins[6 * 3 + 3] == 2 );
  BOOST_TEST( bins[6 * 3 + 3] == 2 );

  BOOST_TEST( j["axis"][0]["nBins"] == 4 );
  BOOST_TEST( j["axis"][0]["title"] == "TestEnum" );
  BOOST_TEST( j["axis"][1]["nBins"] == 3 );
  BOOST_TEST( j["axis"][1]["title"] == "Value" );

  nlohmann::json expected_labels = { "A", "B", "C", "D" };
  BOOST_TEST( j["axis"][0]["labels"] == expected_labels );
}

BOOST_AUTO_TEST_CASE( test_custom_input ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  LogHistogram<1> hist{ &algo, "TestLogHist", "TestLog histogram", { 4, 0, 4 } };

  ++hist[.1];
  ++hist[1.];
  ++hist[3.];
  ++hist[9.];
  ++hist[32456789.];

  auto j = toJSON( hist );

  auto bins = j["bins"];
  BOOST_TEST( bins[0] == 1 );
  BOOST_TEST( bins[1] == 1 );
  BOOST_TEST( bins[2] == 1 );
  BOOST_TEST( bins[3] == 1 );
  BOOST_TEST( bins[4] == 0 );
  BOOST_TEST( bins[5] == 1 );

  BOOST_TEST( j["axis"][0]["nBins"] == 4 );
}

BOOST_AUTO_TEST_CASE( test_custom_input_2d ) {
  using namespace Gaudi::Accumulators;
  Algo algo;

  LogHistogram<2, atomicity::none, float> hist{
      &algo, "TestLogHist", "TestLog histogram", { { 2, 0, 2 }, { 2, 0, 2 } } };

  ++hist[{ 2, 2 }];
  ++hist[{ 9, .1 }];
  ++hist[{ .2, .3 }];

  auto j    = toJSON( hist );
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

  StaticHistogram<1, atomicity::full, float> hist1( &algo, "TestHist1", "Test histogram 1",
                                                    Axis<float>{ 10, 0., 10. } );
  StaticHistogram<1, atomicity::full, float> hist2( &algo, "TestHist2", "Test histogram 2",
                                                    Axis<float>{ 10, 0., 10. } );

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

  mergeAndReset( ent1a, ent2a );
  BOOST_TEST( toJSON( hist1 ).at( "nEntries" ).get<unsigned long>() == 135 );
  BOOST_TEST( toJSON( hist2 ).at( "nEntries" ).get<unsigned long>() == 0 );
  mergeAndReset( ent2b, ent1b );
  BOOST_TEST( toJSON( hist1 ).at( "nEntries" ).get<unsigned long>() == 0 );
  BOOST_TEST( toJSON( hist2 ).at( "nEntries" ).get<unsigned long>() == 135 );
  mergeAndReset( ent1a, ent2b );
  BOOST_TEST( toJSON( hist1 ).at( "nEntries" ).get<unsigned long>() == 135 );
  BOOST_TEST( toJSON( hist2 ).at( "nEntries" ).get<unsigned long>() == 0 );
  mergeAndReset( ent2b, ent1a );
  BOOST_TEST( toJSON( hist1 ).at( "nEntries" ).get<unsigned long>() == 0 );
  BOOST_TEST( toJSON( hist2 ).at( "nEntries" ).get<unsigned long>() == 135 );
}

BOOST_AUTO_TEST_CASE( test_2d_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo algo;
  // test filling a 2D histogram with more bins in x than y
  // Buffer will overflow if the wrong axis' nBins is used to calculate the bin index, resulting in a double free
  StaticHistogram<2, atomicity::full, float> hist{
      &algo, "Test2DHist", "Test 2D histogram", { 64, 0., 64. }, { 52, 0., 52. } };

  for ( int i = 0; i < 64; ++i ) {
    for ( int j = 0; j < 52; ++j ) { ++hist[{ i, j }]; }
  }

  auto j        = toJSON( hist );
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 64 * 52 );
}

BOOST_AUTO_TEST_CASE( test_histos_buffer_move, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo algo;
  // test filling a 1D histogram via a moved buffer
  StaticHistogram<1, atomicity::full, float> hist{ &algo, "Test1DHist", "Test 1D histogram", { 64, 0., 64. } };

  {
    auto buff1 = hist.buffer();
    for ( int i = 0; i < 64; ++i ) { ++buff1[i]; }
    decltype( buff1 ) buff2( std::move( buff1 ) );
    for ( int i = 0; i < 64; ++i ) { ++buff2[i]; }
  }

  auto j        = toJSON( hist );
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 64 * 2 );
}

BOOST_AUTO_TEST_CASE( test_2d_histos_unique_ptr, *boost::unit_test::tolerance( 1e-14 ) ) {
  using namespace Gaudi::Accumulators;
  Algo algo;
  // test constructing a 2D histogram inside a deque via emplace_back
  std::deque<StaticHistogram<2, atomicity::full, float>> histos;
  histos.emplace_back( &algo, "Test2DHist", "Test 2D histogram", Axis<float>{ 10, 0., 10. },
                       Axis<float>{ 10, 0., 10. } );
  {
    auto buf = histos[0].buffer();
    for ( int i = 0; i < 10; ++i ) {
      for ( int j = 0; j < 10; ++j ) { ++buf[{ i, j }]; }
    }
  }

  auto j        = toJSON( histos[0] );
  auto nEntries = j.at( "nEntries" ).get<unsigned long>();
  BOOST_TEST( nEntries == 100 );
}

BOOST_AUTO_TEST_CASE( test_custom_Histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    Gaudi::Accumulators::Histogram<1> histo1d{ &algo, "GaudiH1D" };
    algo.setProperty( "GaudiH1D_Title", "A Gaudi 1D histogram" ).ignore();
    algo.setProperty( "GaudiH1D_Axis0", "( 21, -10.5, 10.5, \"X\" )" ).ignore();
    histo1d.createHistogram( algo );
    auto jHisto = toJSON( histo1d );
    BOOST_TEST( jHisto.at( "title" ) == "A Gaudi 1D histogram" );
    auto axis = jHisto.at( "axis" )[0];
    std::cout << jHisto << std::endl;
    BOOST_TEST( axis.at( "nBins" ) == 21 );
    BOOST_TEST( axis.at( "minValue" ) == -10.5 );
    BOOST_TEST( axis.at( "maxValue" ) == 10.5 );
    BOOST_TEST( axis.at( "title" ) == "X" );
    BOOST_TEST( jHisto.at( "bins" ).get<std::vector<double>>().size() == 23 );
    ++histo1d[-10.0]; // fill the first (non-overflow) bin
    std::cout << toJSON( histo1d ) << std::endl;
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[1] == 1 );
    ++histo1d[-10.0]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[1] == 2 );
  }
  {
    Gaudi::Accumulators::Histogram<2> histo2d{ &algo, "GaudiH2D" };
    algo.setProperty( "GaudiH2D_Title", "A Gaudi 2D histogram" ).ignore();
    algo.setProperty( "GaudiH2D_Axis0", "( 21, -10.5, 10.5, \"X\" )" ).ignore();
    algo.setProperty( "GaudiH2D_Axis1", "( 41, -20.5, 20.5 )" ).ignore();
    histo2d.createHistogram( algo );
    auto jHisto = toJSON( histo2d );
    BOOST_TEST( jHisto.at( "title" ) == "A Gaudi 2D histogram" );
    auto axis0 = jHisto.at( "axis" )[0];
    BOOST_TEST( axis0.at( "nBins" ) == 21 );
    BOOST_TEST( axis0.at( "minValue" ) == -10.5 );
    BOOST_TEST( axis0.at( "maxValue" ) == 10.5 );
    BOOST_TEST( axis0.at( "title" ) == "X" );
    auto axis1 = jHisto.at( "axis" )[1];
    BOOST_TEST( axis1.at( "nBins" ) == 41 );
    BOOST_TEST( axis1.at( "minValue" ) == -20.5 );
    BOOST_TEST( axis1.at( "maxValue" ) == 20.5 );
    BOOST_TEST( axis1.at( "title" ) == "" );
    BOOST_TEST( jHisto.at( "bins" ).get<std::vector<double>>().size() == 43 * 23 );
    ++histo2d[{ -10.0, -20.0 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 1 );
    ++histo2d[{ -10.0, -20.0 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 21 + 1 ) + 1] == 2 );
  }
  {
    Gaudi::Accumulators::WeightedHistogram<1> histo1dw{ &algo, "GaudiH1DW" };
    algo.setProperty( "GaudiH1dw_Title", "A Gaudi 1DW histogram" ).ignore();
    algo.setProperty( "GaudiH1dw_Axis0", "( 21, -10.5, 10.5, \"X\" )" ).ignore();
    histo1dw.createHistogram( algo );
    auto jHisto = toJSON( histo1dw );
    BOOST_TEST( jHisto.at( "title" ) == "A Gaudi 1DW histogram" );
    auto axis0 = jHisto.at( "axis" )[0];
    BOOST_TEST( axis0.at( "nBins" ) == 21 );
    BOOST_TEST( axis0.at( "minValue" ) == -10.5 );
    BOOST_TEST( axis0.at( "maxValue" ) == 10.5 );
    BOOST_TEST( axis0.at( "title" ) == "X" );
    histo1dw[-10.0] += 0.25; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1dw ).at( "bins" )[1] == 0.25 );
    histo1dw[-10.0] += 0.5; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo1dw ).at( "bins" )[1] == 0.75 );
  }
  {
    Gaudi::Accumulators::ProfileHistogram<1u> histo{ &algo, "GaudiP1D" };
    algo.setProperty( "GaudiP1D_Title", "A Gaudi Profile histogram" ).ignore();
    algo.setProperty( "GaudiP1D_Axis0", "( 10, 0, 100 )" ).ignore();
    histo.createHistogram( algo );
    histo[-0.5] += -0.5;
    for ( int i = 0; i < 10; i++ ) { histo[10.0 * double( i ) + 0.5] += double( i ); }
    histo[120.0] += 120.0;
    nlohmann::json j        = toJSON( histo );
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
}

BOOST_AUTO_TEST_CASE( test_custom_root_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo algo;

  {
    Gaudi::Accumulators::RootHistogram<1> histo1d{ &algo, "GaudiH1D" };
    algo.setProperty( "GaudiH1D_Title", "A Gaudi 1D histogram" ).ignore();
    algo.setProperty( "GaudiH1D_Axis0", "( 4, -10, 10, \"X\" )" ).ignore();
    histo1d.createHistogram( algo );
    ++histo1d[1];
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[3] == 1 );
    ++histo1d[2];
    ++histo1d[-100];
    BOOST_TEST( toJSON( histo1d ).at( "bins" )[3] == 2 );
    BOOST_TEST( toJSON( histo1d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo1d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo1d ).at( "sum" ) == 3 );
    BOOST_TEST( toJSON( histo1d ).at( "sum2" ) == 5 );
    BOOST_TEST( toJSON( histo1d ).at( "mean" ) == 1.5 );
    BOOST_TEST( toJSON( histo1d ).at( "standard_deviation" ) == 0.5 );
  }
  {
    Gaudi::Accumulators::RootHistogram<2> histo2d{ &algo, "GaudiH2D" };
    algo.setProperty( "GaudiH2D_Title", "A Gaudi 2D histogram" ).ignore();
    algo.setProperty( "GaudiH2D_Axis0", "( 4, -10, 10, \"X\" )" ).ignore();
    algo.setProperty( "GaudiH2D_Axis1", "( 4, -10, 10, \"Y\" )" ).ignore();
    histo2d.createHistogram( algo );
    ++histo2d[{ 1, 1 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 4 + 1 ) * 3 + 3] == 1 );
    ++histo2d[{ 2, 3 }]; // fill the first (non-overflow) bin
    ++histo2d[{ -100, -100 }];
    BOOST_TEST( toJSON( histo2d ).at( "bins" )[( 1 + 4 + 1 ) * 3 + 3] == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo2d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "sumx" ) == 3 );
    BOOST_TEST( toJSON( histo2d ).at( "sumy" ) == 4 );
    BOOST_TEST( toJSON( histo2d ).at( "sumx2" ) == 5 );
    BOOST_TEST( toJSON( histo2d ).at( "sumy2" ) == 10 );
    BOOST_TEST( toJSON( histo2d ).at( "sumxy" ) == 7 );
    BOOST_TEST( toJSON( histo2d ).at( "meanx" ) == 1.5 );
    BOOST_TEST( toJSON( histo2d ).at( "meany" ) == 2 );
    BOOST_TEST( toJSON( histo2d ).at( "standard_deviationx" ) == .5 );
    BOOST_TEST( toJSON( histo2d ).at( "standard_deviationy" ) == 1 );
  }
  {
    Gaudi::Accumulators::RootHistogram<3> histo3d{ &algo, "GaudiH3D" };
    algo.setProperty( "GaudiH3D_Title", "A Gaudi 3D histogram" ).ignore();
    algo.setProperty( "GaudiH3D_Axis0", "( 4, -10, 10, \"X\" )" ).ignore();
    algo.setProperty( "GaudiH3D_Axis1", "( 4, -10, 10, \"Y\" )" ).ignore();
    algo.setProperty( "GaudiH3D_Axis2", "( 4, -10, 10, \"Z\" )" ).ignore();
    histo3d.createHistogram( algo );
    ++histo3d[{ 1, 1, 1 }]; // fill the first (non-overflow) bin
    BOOST_TEST( toJSON( histo3d ).at( "bins" )[36 * 3 + 6 * 3 + 3] == 1 );
    ++histo3d[{ 2, 3, 2 }]; // fill the first (non-overflow) bin
    ++histo3d[{ -100, -100, -100 }];
    BOOST_TEST( toJSON( histo3d ).at( "bins" )[36 * 3 + 6 * 3 + 3] == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "nEntries" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "nTotEntries" ) == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "sumx" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "sumy" ) == 4 );
    BOOST_TEST( toJSON( histo3d ).at( "sumz" ) == 3 );
    BOOST_TEST( toJSON( histo3d ).at( "sumx2" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumy2" ) == 10 );
    BOOST_TEST( toJSON( histo3d ).at( "sumz2" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumxy" ) == 7 );
    BOOST_TEST( toJSON( histo3d ).at( "sumxz" ) == 5 );
    BOOST_TEST( toJSON( histo3d ).at( "sumyz" ) == 7 );
    BOOST_TEST( toJSON( histo3d ).at( "meanx" ) == 1.5 );
    BOOST_TEST( toJSON( histo3d ).at( "meany" ) == 2 );
    BOOST_TEST( toJSON( histo3d ).at( "meanz" ) == 1.5 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationx" ) == .5 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationy" ) == 1 );
    BOOST_TEST( toJSON( histo3d ).at( "standard_deviationz" ) == .5 );
  }
}

BOOST_AUTO_TEST_CASE( test_custom_integer_histos ) {
  using namespace Gaudi::Accumulators;
  Algo                                        algo;
  Histogram<1, atomicity::none, unsigned int> histo{ &algo, "IntH1D" };
  algo.setProperty( "IntH1D_Title", "A 1D histogram with integer content" ).ignore();
  algo.setProperty( "IntH1D_Axis0", "( 10, 0, 10, \"X\" )" ).ignore();
  histo.createHistogram( algo );
  ++histo[1]; // fill the second (non-overflow) bin
  ++histo[3]; // fill the fourth (non-overflow) bin
  auto j = toJSON( histo );
  BOOST_TEST( j.at( "bins" )[0] == 0 );
  BOOST_TEST( j.at( "bins" )[2] == 1 );
  BOOST_TEST( j.at( "bins" )[4] == 1 );
}

BOOST_AUTO_TEST_CASE( test_custom_custom_axis ) {
  using namespace Gaudi::Accumulators;
  Algo                                                          algo;
  Histogram<1, atomicity::full, TestEnum, std::tuple<EnumAxis>> hist{ &algo, "TestEnumHist" };
  algo.setProperty( "IntH1D_Title", "TestEnum histogram" ).ignore();
  algo.setProperty( "IntH1D_Axis0", "Junk - this is ignored" ).ignore();
  hist.createHistogram( algo );
  hist[TestEnum::A] += 1;
  ++hist[TestEnum::B];
  hist[TestEnum::C] += 2;
  auto j    = toJSON( hist );
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

BOOST_AUTO_TEST_CASE( test_custom_mixed_axis ) {
  using namespace Gaudi::Accumulators;
  Algo                                                                    algo;
  Histogram<2, atomicity::full, float, std::tuple<EnumAxis, Axis<float>>> hist{ &algo, "TestEnumHist" };
  algo.setProperty( "TestEnumHist_Title", "TestEnum 2D histogram" ).ignore();
  algo.setProperty( "TestEnumHist_Axis1", "( 3, 0, 3, \"Value\" )" ).ignore();
  hist.createHistogram( algo );
  hist[{ TestEnum::A, 0.5 }] += 1;
  ++hist[{ TestEnum::B, 1.5 }];
  hist[{ TestEnum::C, 2.5 }] += 2;
  auto j    = toJSON( hist );
  auto bins = j["bins"];
  BOOST_TEST( bins[6 + 1] == 1 );
  BOOST_TEST( bins[6 * 2 + 2] == 1 );
  BOOST_TEST( bins[6 * 3 + 3] == 2 );
  BOOST_TEST( bins[6 * 3 + 3] == 2 );
  BOOST_TEST( j["axis"][0]["nBins"] == 4 );
  BOOST_TEST( j["axis"][0]["title"] == "TestEnum" );
  BOOST_TEST( j["axis"][1]["nBins"] == 3 );
  BOOST_TEST( j["axis"][1]["title"] == "Value" );
  nlohmann::json expected_labels = { "A", "B", "C", "D" };
  BOOST_TEST( j["axis"][0]["labels"] == expected_labels );
}

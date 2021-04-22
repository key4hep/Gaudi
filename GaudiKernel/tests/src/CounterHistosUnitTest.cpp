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

#include <boost/test/unit_test.hpp>

#include <iostream>

// Mock code for the test
struct MonitoringHub : Gaudi::Monitoring::Hub {};
struct ServiceLocator {
  MonitoringHub& monitoringHub() { return m_monitHub; }
  MonitoringHub  m_monitHub{};
};
struct Algo {
  ServiceLocator* serviceLocator() { return new ServiceLocator; }
  std::string     name() { return ""; }
};

BOOST_AUTO_TEST_CASE( test_counter_histos, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo                                      algo;
  Gaudi::Accumulators::ProfileHistogram<1u> histo{&algo, "GaudiP1D", "A Gaudi 1D Profile", {10, 0, 100}};

  histo += {-0.5, -0.5};
  for ( int i = 0; i < 10; i++ ) { histo += {10.0 * double( i ) + 0.5, double( i )}; }
  histo += {120.0, 120.0};

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

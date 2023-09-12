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
#define BOOST_TEST_MODULE test_CounterArray
#include <Gaudi/Accumulators/CounterArray.h>

#include <boost/test/unit_test.hpp>

#include <fmt/format.h>

#include <iostream>
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
  struct CounterSink : public Gaudi::Monitoring::Hub::Sink {
    virtual void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override { m_entities.push_back( ent ); }
    virtual void removeEntity( Gaudi::Monitoring::Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_entities ), end( m_entities ), ent );
      if ( it != m_entities.end() ) m_entities.erase( it );
    }
    std::vector<Gaudi::Monitoring::Hub::Entity> m_entities;
  };

  // Little helper for using automatic nlohmann conversion mechanism
  template <typename T>
  nlohmann::json toJSON( T const& t ) {
    nlohmann::json j = t;
    return t;
  }
} // namespace

BOOST_AUTO_TEST_CASE( test_counter_array ) {
  Algo        algo;
  CounterSink sink;
  algo.serviceLocator()->monitoringHub().addSink( &sink );
  {
    // testing an array of 5 simple counters, with "standard" names
    Gaudi::Accumulators::CounterArray<Gaudi::Accumulators::Counter<>, 5> counters{ &algo, "Counter-{}" };
    // check increment
    for ( unsigned int i = 0; i < 5; i++ ) ++counters[i]; // increment counters
    for ( unsigned int i = 0; i < 5; i++ ) BOOST_TEST( counters[i].nEntries() == 1 );
    // check names
    unsigned int i = 0;
    for ( auto& entity : sink.m_entities ) {
      BOOST_TEST( entity.name == fmt::format( "Counter-{}", i ) );
      i++;
    }
    // check json output
    for ( unsigned int i = 0; i < 5; i++ ) { BOOST_TEST( toJSON( counters[i] ).at( "nEntries" ) == 1 ); }
  }

  {
    // testing an array of averaging counters with "standard" names
    Gaudi::Accumulators::CounterArray<Gaudi::Accumulators::AveragingCounter<>, 5> counters{ &algo, "AvgCounter-{}" };
    // check increment
    for ( unsigned int i = 0; i < 5; i++ ) counters[i] += i; // increment counters
    for ( unsigned int i = 0; i < 5; i++ ) {
      BOOST_TEST( counters[i].nEntries() == 1 );
      BOOST_TEST( counters[i].sum() == i );
    }
    // check names
    unsigned int i = 0;
    for ( auto& entity : sink.m_entities ) {
      BOOST_TEST( entity.name == fmt::format( "AvgCounter-{}", i ) );
      i++;
    }
    // check json output
    for ( unsigned int i = 0; i < 5; i++ ) {
      BOOST_TEST( toJSON( counters[i] ).at( "nEntries" ) == 1 );
      BOOST_TEST( toJSON( counters[i] ).at( "sum" ) == i );
    }
  }

  {
    // testing an array of simple counters, with non standard names
    Gaudi::Accumulators::CounterArray<Gaudi::Accumulators::Counter<>, 5> counters{
        &algo, []( int n ) { return fmt::format( "Counter-{}-{}", n, n ^ 2 ); } };
    // check increment
    for ( unsigned int i = 0; i < 5; i++ ) ++counters[i]; // increment counters
    for ( unsigned int i = 0; i < 5; i++ ) { BOOST_TEST( counters[i].nEntries() == 1 ); }
    // check names
    unsigned int i = 0;
    for ( auto& entity : sink.m_entities ) {
      BOOST_TEST( entity.name == fmt::format( "Counter-{}-{}", i, i ^ 2 ) );
      i++;
    }
    // check json output
    for ( unsigned int i = 0; i < 5; i++ ) { BOOST_TEST( toJSON( counters[i] ).at( "nEntries" ) == 1 ); }
  }
}

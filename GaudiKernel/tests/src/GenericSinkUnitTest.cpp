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
#define BOOST_TEST_MODULE test_GenericSink

#include "Gaudi/MonitoringHub.h"

#include <boost/test/unit_test.hpp>

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

  // A class to be used in Entities and through the Sink/Hub machinery but
  // not deriving from counters
  // Basically accumulate data into a string
  struct Store {
    std::string             m_data;
    Gaudi::Monitoring::Hub* m_monitoringHub{ nullptr };

    template <typename OWNER>
    Store( OWNER* o, std::string const& name, const std::string storeType )
        : m_monitoringHub( &o->serviceLocator()->monitoringHub() ) {
      m_monitoringHub->registerEntity( o->name(), name, storeType, *this );
    }
    ~Store() { m_monitoringHub->removeEntity( *this ); }
    void        storeData( std::string const& data ) { m_data += data; }
    friend void to_json( nlohmann::json& j, Store const& s ) { j = s.m_data; }
  };

  // dummy  Sink for the purpose of testing Hub and Sink with non mergeable items
  struct NonMergeableSink : public Gaudi::Monitoring::Hub::Sink {
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

BOOST_AUTO_TEST_CASE( test_entity_no_merge_reset, *boost::unit_test::tolerance( 1e-14 ) ) {
  Algo             algo;
  NonMergeableSink sink;
  algo.serviceLocator()->monitoringHub().addSink( &sink );

  Store store1( &algo, "TestStore1", "store:test" );
  Store store2( &algo, "TestStore2", "store:test" );

  store1.storeData( "Hel" );
  store1.storeData( "lo " );
  store2.storeData( "World !" );

  auto& entities = sink.m_entities;
  std::sort( begin( entities ), end( entities ), []( const auto& a, const auto& b ) { return a.name < b.name; } );
  std::string output;
  for ( auto& ent : entities ) { output += toJSON( ent ).get<std::string>(); }
  BOOST_TEST( output == "Hello World !" );
}

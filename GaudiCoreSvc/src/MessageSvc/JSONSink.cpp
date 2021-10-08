/***********************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "Gaudi/MonitoringHub.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"

#include <boost/algorithm/string.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <Gaudi/Property.h>
#include <deque>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <string_view>

namespace {
  void writeFileJson( std::string fileName, bool fulloutput,
                      std::map<std::string, std::map<std::string, nlohmann::json>>& sEntities ) {

    // perpareJsonFile
    using json = nlohmann::json;
    json jsonfile;

    // dump all counters
    for ( auto& [algoName, entityMap] : sEntities ) {
      for ( auto& [entityName, entity] : entityMap ) {
        if ( !fulloutput ) {
          if ( !entity.at( "empty" ).template get<bool>() ) {
            jsonfile[algoName][entityName] = entity;
            jsonfile[algoName][entityName].erase( "empty" );
            // jsonfile[algoName][entityName].erase( "type" );
            jsonfile[algoName][entityName].erase( "sum2" );
          }
        } else {
          jsonfile[algoName][entityName] = entity;
        }
      }
    }
    std::ofstream os( fileName, std::ios::out );
    os << jsonfile.dump( 4 );
    os.close();
  }
} // namespace

namespace Gaudi::Monitoring {

  class JSONSink : public Service, public Hub::Sink {

  public:
    using Service::Service;

    /// initialization, registers to Monitoring::Hub
    StatusCode initialize() override {
      return Service::initialize().andThen( [&] {
        // declare ourself as a monitoding sink
        serviceLocator()->monitoringHub().addSink( this );
      } );
    }

    /// stop method, handles the printing
    StatusCode stop() override;

    // Gaudi::Monitoring::Hub::Sink implementation
    void registerEntity( Hub::Entity ent ) override {
      if ( std::string_view( ent.type ).substr( 0, 8 ) == "counter:" || ent.type == "statentity" ||
           ent.type == "histogram" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

    // Gaudi::Monitoring::Hub::Sink implementation
    void removeEntity( Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_monitoringEntities ), end( m_monitoringEntities ), ent );
      if ( it != m_monitoringEntities.end() ) { m_monitoringEntities.erase( it ); }
    }

  private:
    std::deque<Hub::Entity>      m_monitoringEntities;
    Gaudi::Property<std::string> m_jsonOutputFileName{this, "FileName", "counters.json",
                                                      "Name of JSON file for counters dump"};
    Gaudi::Property<bool>        m_jsonOutputFullInfo{this, "DumpFullInfo", false,
                                               "Dump full Counter info in the JSON output file"};
  };

  DECLARE_COMPONENT( JSONSink )
} // namespace Gaudi::Monitoring

StatusCode Gaudi::Monitoring::JSONSink::stop() {
  // We will try to mimic the old monitoring of counters, so we need to split
  // them per Algo. The algo name can be extracted form the id of the entity
  // as its format is "algoName/counterName"
  // This map groups entities per algoName. For each name, the submap gives
  // the counter name of each subentity and the associated json
  std::map<std::string, std::map<std::string, nlohmann::json>> sortedEntities;
  // fill the sorted map
  for ( auto& entity : m_monitoringEntities ) { sortedEntities[entity.component][entity.name] = entity.toJSON(); }

  if ( !m_jsonOutputFileName.empty() ) {
    info() << "Writing counters to JSON file " << m_jsonOutputFileName.value() << endmsg;
    writeFileJson( m_jsonOutputFileName, m_jsonOutputFullInfo, sortedEntities );
  }

  return Service::stop();
}

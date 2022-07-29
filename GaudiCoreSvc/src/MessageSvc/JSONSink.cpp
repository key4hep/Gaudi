/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "GaudiKernel/Service.h"

#include <Gaudi/MonitoringHub.h>

#include <fstream>

#include <deque>
#include <string>

namespace Gaudi::Monitoring {

  class JSONSink : public Service, public Hub::Sink {

  public:
    using Service::Service;

    StatusCode initialize() override {
      return Service::initialize().andThen( [&] { serviceLocator()->monitoringHub().addSink( this ); } );
    }

    StatusCode stop() override {
      auto ok = Service::stop();
      if ( !ok ) return ok;
      if ( m_fileName.empty() ) { return ok; }

      nlohmann::json output;
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ), [&output]( auto& ent ) {
        output.emplace_back( nlohmann::json{
            { "name", ent.name },
            { "component", ent.component },
            { "entity", ent.toJSON() },
        } );
      } );
      info() << "Writing JSON file " << m_fileName.value() << endmsg;
      std::ofstream os( m_fileName, std::ios::out );
      os << output.dump( 4 );
      os.close();
      return ok;
    }

    void registerEntity( Hub::Entity ent ) override {
      if ( wanted( ent.type, m_typesToSave ) && wanted( ent.name, m_namesToSave ) &&
           wanted( ent.component, m_componentsToSave ) ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

    void removeEntity( Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_monitoringEntities ), end( m_monitoringEntities ), ent );
      if ( it != m_monitoringEntities.end() ) { m_monitoringEntities.erase( it ); }
    }

  private:
    bool wanted( std::string name, std::vector<std::string> searchNames ) {
      if ( searchNames.empty() ) { return true; }
      for ( const auto& searchName : searchNames ) {
        const std::regex regex( searchName );
        if ( std::regex_match( name, regex ) ) { return true; }
      }
      return false;
    }

    std::deque<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;
    Gaudi::Property<std::string> m_fileName{ this, "FileName", "json_output.json", "Name of output json file" };
    Gaudi::Property<std::vector<std::string>> m_namesToSave{ this, "NamesToSave", {}, "Names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_componentsToSave{
        this, "ComponentsToSave", {}, "Component names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_typesToSave{
        this, "TypesToSave", {}, "Type names of entities to save" };
  };

  DECLARE_COMPONENT( JSONSink )

} // namespace Gaudi::Monitoring

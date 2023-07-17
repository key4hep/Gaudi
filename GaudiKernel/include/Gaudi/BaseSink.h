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

#include <deque>
#include <string>

namespace Gaudi::Monitoring {

  /**
   * Base class for all Sinks registering to the Monitoring Hub
   *
   * Deals with registration and manages selection of Entities
   * Should be extended by actual Sinks
   */
  class BaseSink : public Service, public Hub::Sink {

  public:
    using Service::Service;

    StatusCode initialize() override {
      // registers itself to the Monitoring Hub
      return Service::initialize().andThen( [&] { serviceLocator()->monitoringHub().addSink( this ); } );
    }

    /// handles registration of a new entity
    void registerEntity( Hub::Entity ent ) override {
      if ( wanted( ent.type, m_typesToSave ) && wanted( ent.name, m_namesToSave ) &&
           wanted( ent.component, m_componentsToSave ) ) {
        m_monitoringEntities.insert( { ent.id(), std::move( ent ) } );
      }
    }

    /// handles removal of an entity
    void removeEntity( Hub::Entity const& ent ) override {
      auto it = m_monitoringEntities.find( ent.id() );
      if ( it != m_monitoringEntities.end() ) { m_monitoringEntities.erase( it ); }
    }

  protected:
    /**
     * applies a callable to all monitoring entities
     *
     * Entities may be first sorted to improve reproducibility
     */
    template <typename Callable>
    void applytoAllEntities( Callable func ) {
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ),
                     [func]( auto& p ) { func( p.second ); } );
    }

    /**
     * returns all entities in JSON format, grouped by component first and then name
     */
    std::map<std::string, std::map<std::string, nlohmann::json>> const sortedEntitiesAsJSON() {
      std::map<std::string, std::map<std::string, nlohmann::json>> sortedEntities;
      applytoAllEntities( [&sortedEntities]( auto& ent ) { sortedEntities[ent.component][ent.name] = ent.toJSON(); } );
      return sortedEntities;
    }

  private:
    /// deciding whether a given name matches the list of regexps given
    /// empty list means everything matches
    bool wanted( std::string name, std::vector<std::string> searchNames ) {
      if ( searchNames.empty() ) { return true; }
      for ( const auto& searchName : searchNames ) {
        const std::regex regex( searchName );
        if ( std::regex_match( name, regex ) ) { return true; }
      }
      return false;
    }

    /// list of entities we are dealing with
    std::map<void*, Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;
    Gaudi::Property<std::vector<std::string>>       m_namesToSave{
        this, "NamesToSave", {}, "List of regexps used to match names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_componentsToSave{
        this, "ComponentsToSave", {}, "List of regexps used to match component names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_typesToSave{
        this, "TypesToSave", {}, "List of regexps used to match type names of entities to save" };
  };

} // namespace Gaudi::Monitoring

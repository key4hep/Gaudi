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

#include <string>
#include <vector>

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
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

    /// handles removal of an entity
    void removeEntity( Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_monitoringEntities ), end( m_monitoringEntities ), ent );
      if ( it != m_monitoringEntities.end() ) { m_monitoringEntities.erase( it ); }
    }

  protected:
    /**
     * applies a callable to all monitoring entities
     */
    template <typename Callable>
    void applyToAllEntities( Callable func ) const {
      // loop over entities
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ), func );
    }

    /**
     * applies a callable to all monitoring entities
     *
     * Entities are first sorted to improve reproducibility
     */
    template <typename Callable>
    void applyToAllEntitiesWithSort( Callable func ) {
      std::sort( begin( m_monitoringEntities ), end( m_monitoringEntities ), []( const auto& a, const auto& b ) {
        return std::tie( a.name, a.component ) > std::tie( b.name, b.component );
      } );
      applyToAllEntities( func );
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
    std::vector<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;
    Gaudi::Property<std::vector<std::string>>   m_namesToSave{
        this, "NamesToSave", {}, "List of regexps used to match names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_componentsToSave{
        this, "ComponentsToSave", {}, "List of regexps used to match component names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_typesToSave{
        this, "TypesToSave", {}, "List of regexps used to match type names of entities to save" };
  };

} // namespace Gaudi::Monitoring

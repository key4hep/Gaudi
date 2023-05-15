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

#include <Gaudi/MonitoringHub.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/Service.h>

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Gaudi::Monitoring {

  /**
   * Base class for all Sinks registering to the Monitoring Hub
   * Should be extended by actual Sinks
   *
   * Deals with registration and manages selection of Entities
   * Provides support for regular flushing of the Sink content.
   * The AutoFlushPeriod property defines the interval. A value
   * of 0 means no auto flush.
   *
   * Actual Sinks extending this base class have to implement
   * the `flush` method. This one has a boolean argument allowing
   * to know whether it was called at a regular interval or a the
   * end
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

    /**
     * pure virtual method to be defined by children and responsible for
     * flushing current data of the Sink. This method is called every
     * m_autoFlushPeriod (if not set to 0) and in the stop method by default
     * @param isStop allows to know which case we are in
     */
    virtual void flush( bool isStop ) = 0;

    StatusCode start() override {
      return Service::start().andThen( [&] {
        // promise needs to be recreated in case of a restart
        m_flushThreadStop = std::promise<void>{};
        // enable periodic output file flush if requested
        if ( m_autoFlushPeriod.value() != 0 ) {
          m_flushThread = std::thread{ [this, flushStop = m_flushThreadStop.get_future()]() {
            using namespace std::chrono_literals;
            while ( flushStop.wait_for( m_autoFlushPeriod.value() * 1s ) == std::future_status::timeout ) {
              flush( false );
            }
          } };
        }
      } );
    }

    StatusCode stop() override {
      m_flushThreadStop.set_value();                        // tell the flush thread we are stopping
      if ( m_flushThread.joinable() ) m_flushThread.join(); // and wait that it exits
      flush( true );
      return Service::stop();
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

    /// Management of entities we will handle or not
    Gaudi::Property<std::vector<std::string>> m_namesToSave{
        this, "NamesToSave", {}, "List of regexps used to match names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_componentsToSave{
        this, "ComponentsToSave", {}, "List of regexps used to match component names of entities to save" };
    Gaudi::Property<std::vector<std::string>> m_typesToSave{
        this, "TypesToSave", {}, "List of regexps used to match type names of entities to save" };

    /// Handling of regular flushes, if requested
    std::thread            m_flushThread;
    std::promise<void>     m_flushThreadStop;
    Gaudi::Property<float> m_autoFlushPeriod{
        this, "AutoFlushPeriod", 0.,
        "if different from 0, indicates every how many seconds to force a write of the FSR data to OutputFile (this "
        "parameter makes sense only if used in conjunction with OutputFile)" };
  };

} // namespace Gaudi::Monitoring

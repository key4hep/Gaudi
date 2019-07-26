#pragma once

#include <functional>
#include <list>
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>

namespace Gaudi {
  /// Central entity in a Gaudi application that manages monitoring objects (i.e. counters, histograms, etc.).
  ///
  /// The Gaudi::MonitoringHub delegates the actual reports to services implementing the Gaudi::MonitoringHub::Sink
  /// interface.
  struct MonitoringHub {
    using json = nlohmann::json;

    /** Wrapper class for arbitrary monitoring objects.
     *
     * What qualifies an object to be a monitoring entity is the existence of a non member function toJSON(cont T&)
     * that returns a JSON object like:
     * ```json
     * {
     *   "type": "some_type_id",
     *   "str": "basic string representation",
     *   "some_value_name": value,
     *   ...
     * }
     * ```
     * The *type* field is used by specific Sink instances to decide if they have to handle the object or not, and
     * it can be used to know which fields to expect and how to treat them.
     *
     * A *str* field is requested for fallback to generic handling, if needed.
     */
    struct Entity {
      template <typename T>
      Entity( std::string id, const T& ent )
          : id{std::move( id )}, ptr{&ent}, type{typeid( T )}, getJSON{[&ent]() { return toJSON( ent ); }} {}
      std::string           id;
      const void*           ptr{nullptr};
      const std::type_info& type;
      std::function<json()> getJSON;
    };

    /// Interface reporting services must implement.
    struct Sink {
      virtual void registerEntity( Entity ent ) = 0;
      virtual ~Sink()                           = default;
    };

    template <typename T>
    void registerEntity( std::string s, const T& ent ) {
      registerEntity( {std::move( s ), ent} );
    }
    void registerEntity( Entity ent ) {
      std::for_each( begin( m_sinks ), end( m_sinks ),
                     [ent]( auto sink ) { sink->registerEntity( std::move( ent ) ); } );
      m_entities.emplace_back( std::move( ent ) );
    }

    void addSink( Sink* sink ) {
      std::for_each( begin( m_entities ), end( m_entities ),
                     [sink]( Entity ent ) { sink->registerEntity( std::move( ent ) ); } );
      m_sinks.push_back( sink );
    }
    void removeSink( Sink* sink ) { m_sinks.remove( sink ); }

  private:
    std::list<Sink*>  m_sinks;
    std::list<Entity> m_entities;
  };
} // namespace Gaudi

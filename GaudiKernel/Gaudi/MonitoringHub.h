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
     * What qualifies an object to be a monitoring entity is the existence of toJSON() method
     * that returns a generic JSON object. It will typically be a dictionnary, but the meaning of each entry
     * is only defined by the Entity producers, for each type of Entity.
     *
     * @param id the name of the entity
     * @param type the type of the entity, as a string. This is used by specific Sink instances to decide if they
     * have to handle a given entity or not. It can be used to know which fields to expect and how to treat them.
     * @param internalType the actual type_info of the internal data in this Entity
     * @param ptr pointer to the actual data inside this Entity
     * @param getJSON a function converting the internal data to json. Due to type erasure, it needs to be a
     * member of this struct
     *
     * A *str* field is requested for fallback to generic handling, if needed.
     */
    struct Entity {
      template <typename T>
      Entity( std::string id, std::string type, const T& ent )
        : id{std::move( id )}, type{std::move( type )}, internalType{typeid( T )}, ptr{&ent},
          getJSON{[&ent]() { return ent.toJSON(); }} {}
      std::string           id;
      std::string           type;
      const std::type_info& internalType;
      const void*           ptr{nullptr};
      std::function<json()> getJSON;
    };

    /// Interface reporting services must implement.
    struct Sink {
      virtual void registerEntity( Entity ent ) = 0;
      virtual ~Sink()                           = default;
    };

    template <typename T>
    void registerEntity( std::string s, std::string t, const T& ent ) {
      registerEntity( {std::move( s ), std::move( t ), ent} );
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

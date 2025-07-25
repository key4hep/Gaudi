/*****************************************************************************\
* (c) Copyright 2020-2025 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include <deque>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <typeindex>
#include <typeinfo>

namespace Gaudi::Monitoring {

  namespace details {

    // type trait checking whether a given type has a friend method reset
    template <typename Arg, typename = void> // this 3rd parameter defaults to void
    struct has_reset_method : std::false_type {};
    template <typename Arg>
    struct has_reset_method<Arg, std::void_t<decltype( reset( std::declval<Arg&>() ) )>> : std::true_type {};
    template <typename Arg>
    constexpr bool has_reset_method_v = has_reset_method<Arg>::value;

    // type trait checking whether a given type has a friend method mergeAndReset
    template <typename Arg, typename = void> // this 3rd parameter defaults to void
    struct has_mergeAndReset_method : std::false_type {};
    template <typename Arg>
    struct has_mergeAndReset_method<
        Arg, std::void_t<decltype( mergeAndReset( std::declval<Arg&>(), std::declval<Arg&>() ) )>> : std::true_type {};
    template <typename Arg>
    constexpr bool has_mergeAndReset_method_v = has_mergeAndReset_method<Arg>::value;

  } // namespace details

  /// Central entity in a Gaudi application that manages monitoring objects (i.e. counters, histograms, etc.).
  ///
  /// The Gaudi::Monitoring::Hub delegates the actual reports to services implementing the Gaudi::Monitoring::Hub::Sink
  /// interface.
  struct Hub {
    /** Wrapper class for arbitrary monitoring objects.
     *
     * Mainly contains a pointer to the actual data with component, name and type metadata
     * Any object can be used as internal data and wrapped into an Entity as long as they can
     * be translated to json format. It is enough to make them aware to the nlohmann library
     * through a dedicated to_json method.
     *
     * another 2 free functions can be overloaded to adapt the behavior of an Entity for a
     * given type T :
     *   - void reset( T& t )
     *     called to reset the entity. The default provided implementation is empty
     *   - void mergeAndReset( T& ent, T&& other )
     *     called to merge other into entity and reset other. The default provided implementation is empty
     */
    class Entity {
    public:
      template <typename T>
      Entity( std::string component, std::string name, std::string type, T& ent )
          : component{ std::move( component ) }
          , name{ std::move( name ) }
          , type{ std::move( type ) }
          , m_ptr{ &ent }
          , m_typeIndex{ typeid( T ) }
          , m_getJSON{ []( void const* ptr ) -> nlohmann::json { return *reinterpret_cast<const T*>( ptr ); } }
          , m_reset{ []( void* ptr ) {
            if constexpr ( details::has_reset_method_v<T> ) { reset( *reinterpret_cast<T*>( ptr ) ); }
          } }
          , m_mergeAndReset{ []( void* e, void* o ) {
            if constexpr ( details::has_mergeAndReset_method_v<T> ) {
              mergeAndReset( *reinterpret_cast<T*>( e ), *reinterpret_cast<T*>( o ) );
            }
          } } {}
      /// name of the component owning the Entity
      std::string component;
      /// name of the entity
      std::string name;
      /// type of the entity, see comment above concerning its format and usage
      std::string type;
      /// function to get internal type
      std::type_index typeIndex() const { return m_typeIndex; }
      /// conversion to json via nlohmann library
      friend void to_json( nlohmann::json& j, Gaudi::Monitoring::Hub::Entity const& e ) {
        j = std::invoke( e.m_getJSON, e.m_ptr );
      }
      /// function resetting internal data
      friend void reset( Entity const& e ) { std::invoke( e.m_reset, e.m_ptr ); }
      /**
       * function calling merge and reset on internal data with the internal data of another entity
       *
       * This function does not protect against usage with entities with different internal types
       * The user should ensure that entities are compatible before calling this function
       */
      friend void mergeAndReset( Entity const& ent, Entity const& other ) {
        if ( ent.typeIndex() != other.typeIndex() ) {
          throw std::runtime_error( std::string( "Entity: mergeAndReset called on different types: " ) +
                                    ent.typeIndex().name() + " and " + other.typeIndex().name() );
        }
        std::invoke( ent.m_mergeAndReset, ent.m_ptr, other.m_ptr );
      }
      /// operator== for comparison with an entity
      bool operator==( Entity const& ent ) const { return id() == ent.id(); }
      /// unique identifier, actually mapped to internal pointer
      void* id() const { return m_ptr; }

    private:
      /// pointer to the actual data inside this Entity
      void* m_ptr{ nullptr };
      // The next 4 members are needed for type erasure
      // indeed, their implementation is internal type dependant
      // (see Constructor above and the usage of T in the reinterpret_cast)
      std::type_index m_typeIndex;
      /// function converting the internal data to json.
      nlohmann::json ( *m_getJSON )( void const* );
      /// function reseting internal data.
      void ( *m_reset )( void* );
      /// function calling merge and reset on internal data with the internal data of another entity
      void ( *m_mergeAndReset )( void*, void* );
    };

    /// Interface reporting services must implement.
    struct Sink {
      virtual void registerEntity( Entity ent )      = 0;
      virtual void removeEntity( Entity const& ent ) = 0;
      virtual ~Sink()                                = default;
    };

    Hub() { m_sinks.reserve( 5 ); }

    template <typename T>
    void registerEntity( std::string c, std::string n, std::string t, T& ent ) {
      registerEntity( { std::move( c ), std::move( n ), std::move( t ), ent } );
    }
    void registerEntity( Entity ent ) {
      std::for_each( begin( m_sinks ), end( m_sinks ), [ent]( auto sink ) { sink->registerEntity( ent ); } );
      m_entities.emplace( ent.id(), std::move( ent ) );
    }
    template <typename T>
    void removeEntity( T& ent ) {
      auto it = m_entities.find( &ent );
      if ( it != m_entities.end() ) {
        std::for_each( begin( m_sinks ), end( m_sinks ), [&it]( auto sink ) { sink->removeEntity( it->second ); } );
        m_entities.erase( it );
      }
    }

    void addSink( Sink* sink ) {
      std::for_each( begin( m_entities ), end( m_entities ),
                     [sink]( auto ent ) { sink->registerEntity( ent.second ); } );
      m_sinks.push_back( sink );
    }
    void removeSink( Sink* sink ) {
      auto it = std::find( begin( m_sinks ), end( m_sinks ), sink );
      if ( it != m_sinks.end() ) m_sinks.erase( it );
    }

  private:
    std::vector<Sink*>      m_sinks;
    std::map<void*, Entity> m_entities;
  };
} // namespace Gaudi::Monitoring

/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
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
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>

namespace Gaudi::Monitoring {

  namespace details {

    struct MergeAndResetBase {
      virtual void operator()( void*, void* ) const = 0;
      virtual ~MergeAndResetBase() {}
    };
    template <typename T, typename = void>
    struct MergeAndReset : MergeAndResetBase {
      void operator()( void*, void* ) const override {}
    };
    template <typename T>
    struct MergeAndReset<T, decltype( std::declval<T>().mergeAndReset( nullptr ) )> : MergeAndResetBase {
      void operator()( void* ptr, void* other ) const override {
        reinterpret_cast<T*>( ptr )->mergeAndReset( std::move( *reinterpret_cast<T*>( other ) ) );
      }
    };

  } // namespace details

  /// Central entity in a Gaudi application that manages monitoring objects (i.e. counters, histograms, etc.).
  ///
  /// The Gaudi::Monitoring::Hub delegates the actual reports to services implementing the Gaudi::Monitoring::Hub::Sink
  /// interface.
  struct Hub {
    using json = nlohmann::json;

    /** Wrapper class for arbitrary monitoring objects.
     *
     * Mainly contains a pointer to the actual data with component, name and type metadata
     * Any object having a toJSON method can be used as internal data and wrapped into an Entity
     *
     * This toJSON method should generate a json dictionnary with a "type" entry of type string
     * and as many others as entries as needed. Entity producers are thus free to add their own entries
     * provided they provide a type one. If the type value contains a ':' character, then the part
     * preceding it will be considered as a namespace. The rest is free text.
     * The type in json should match the type member of the Entity. It is used by Sink instances
     * to decide if they have to handle a given entity or not.
     * It can also be used to know which fields to expect in the json dictionnary
     */
    class Entity {
    public:
      template <typename T>
      Entity( std::string component, std::string name, std::string type, T& ent )
          : component{ std::move( component ) }
          , name{ std::move( name ) }
          , type{ std::move( type ) }
          , m_ptr{ &ent }
          , m_reset{ []( void* ptr ) { reinterpret_cast<T*>( ptr )->reset(); } }
          , m_mergeAndReset{ std::make_shared<details::MergeAndReset<T>>() }
          , m_getJSON{ []( const void* ptr ) { return reinterpret_cast<const T*>( ptr )->toJSON(); } } {}
      /// name of the component owning the Entity
      std::string component;
      /// name of the entity
      std::string name;
      /// type of the entity, see comment above concerning its format and usage
      std::string type;
      /// function giving access to internal data in json format
      json toJSON() const { return ( *m_getJSON )( m_ptr ); }
      /// function resetting internal data
      void reset() { return ( *m_reset )( m_ptr ); }
      // The following function does not protect against usage with entities with different internal types
      // The user should ensure that entities are compatible before calling this function
      /// function calling merge and reset on internal data with the internal data of another entity
      void mergeAndReset( Entity const& ent ) { return ( *m_mergeAndReset )( m_ptr, ent.m_ptr ); }
      /// operator== for comparison with raw pointer
      bool operator==( void* ent ) { return m_ptr == ent; }
      /// operator== for comparison with an entity
      bool operator==( Entity const& ent ) { return m_ptr == ent.m_ptr; }

    private:
      /// pointer to the actual data inside this Entity
      void* m_ptr{ nullptr };
      // The next 3 members are needed for type erasure
      // indeed, their implementation is internal type dependant
      // (see Constructor above and the usage of T in the reinterpret_cast)
      /// function reseting internal data.
      void ( *m_reset )( void* );
      /// function calling merge and reset on internal data with the internal data of another entity
      std::shared_ptr<details::MergeAndResetBase> m_mergeAndReset;
      /// function converting the internal data to json.
      json ( *m_getJSON )( const void* );
    };

    /// Interface reporting services must implement.
    struct Sink {
      virtual void registerEntity( Entity ent )      = 0;
      virtual void removeEntity( Entity const& ent ) = 0;
      virtual ~Sink()                                = default;
    };

    template <typename T>
    void registerEntity( std::string c, std::string n, std::string t, T& ent ) {
      registerEntity( { std::move( c ), std::move( n ), std::move( t ), ent } );
    }
    void registerEntity( Entity ent ) {
      std::for_each( begin( m_sinks ), end( m_sinks ), [ent]( auto sink ) { sink->registerEntity( ent ); } );
      m_entities.emplace_back( std::move( ent ) );
    }
    template <typename T>
    void removeEntity( T& ent ) {
      auto it = std::find( begin( m_entities ), end( m_entities ), &ent );
      if ( it != m_entities.end() ) {
        std::for_each( begin( m_sinks ), end( m_sinks ), [&it]( auto sink ) { sink->removeEntity( *it ); } );
        m_entities.erase( it );
      }
    }

    void addSink( Sink* sink ) {
      std::for_each( begin( m_entities ), end( m_entities ),
                     [sink]( Entity ent ) { sink->registerEntity( std::move( ent ) ); } );
      m_sinks.push_back( sink );
    }
    void removeSink( Sink* sink ) {
      auto it = std::find( begin( m_sinks ), end( m_sinks ), sink );
      if ( it != m_sinks.end() ) m_sinks.erase( it );
    }

  private:
    std::deque<Sink*>  m_sinks;
    std::deque<Entity> m_entities;
  };
} // namespace Gaudi::Monitoring

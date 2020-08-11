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

#include <functional>
#include <list>
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>

namespace Gaudi::Monitoring {
  /// Central entity in a Gaudi application that manages monitoring objects (i.e. counters, histograms, etc.).
  ///
  /// The Gaudi::Monitoring::Hub delegates the actual reports to services implementing the Gaudi::Monitoring::Hub::Sink
  /// interface.
  struct Hub {
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
     * Here is a list of currently used types and their fields. Note that all fields are not
     * independant, that is some values are preprocessed.
     *   - counter : empty(bool), subtype(string)
     *     Depending on the counter subtype, here are the extra fields :
     *     + basic and message : nEntries(integer)
     *     + averaging : basic ones, sum(number), mean(number)
     *         and we have mean = sum / nEntries
     *     + sigma : averaging ones, sum2(number), standard_deviation(number)
     *         and we have standard_deviation = sqrt((sum2 - sum * sum / nEntries)/(nEntries - 1))
     *     + stat : sigma ones, min(number), max(number)
     *     + statentity : DEPRECATED so no fields and empty is always true
     *     + binomial : nEntries(integer), nTrueEntries(integer), nFalseEntries(integer), efficiency(number),
     * efficiencyErr(number) and we have nEntries = nTrueEntries + nFalseEntries efficiency = nTrueEntries/nEntries
     *                     efficiencyErr = (nTrueEntries * nFalseEntries)/(nEntries * nEntries)
     *   - timer : same fields as counter with subtype stat
     *   - histogram : currently empty, implementation to come
     */
    struct Entity {
      template <typename T>
      Entity( std::string component, std::string name, std::string type, const T& ent )
          : component{std::move( component )}
          , name{std::move( name )}
          , type{std::move( type )}
          , internalType{typeid( T )}
          , ptr{&ent}
          , getJSON{[&ent]() { return ent.toJSON(); }} {}
      std::string           component;
      std::string           name;
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
    void registerEntity( std::string c, std::string n, std::string t, const T& ent ) {
      registerEntity( {std::move( c ), std::move( n ), std::move( t ), ent} );
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
} // namespace Gaudi::Monitoring

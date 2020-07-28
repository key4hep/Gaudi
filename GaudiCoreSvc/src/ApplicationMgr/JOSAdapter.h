/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/IJobOptionsSvc.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>
#include <string>
#include <tuple>

namespace Gaudi::Details {

  /// Adapter to provide IOptionsSvc interface over custom implementations of IJobOptionsSvc.
  struct JOSAdapter final : extends<Service, IJobOptionsSvc, Gaudi::Interfaces::IOptionsSvc> {
  public:
    JOSAdapter( SmartIF<IJobOptionsSvc> jos, ISvcLocator* svcLoc )
        : extends{jos.as<INamedInterface>()->name(), svcLoc}, m_jos{std::move( jos )} {}
    using Service::getProperties;

    StatusCode queryInterface( const InterfaceID& ti, void** pp ) override {
      auto sc = m_jos->queryInterface( ti, pp );
      if ( !sc ) sc = extends::queryInterface( ti, pp );
      return sc;
    }

    StatusCode initialize() override {
      auto sc = extends::initialize();
      if ( !sc ) return sc;
      return m_jos.as<IStateful>()->initialize();
    }
    StatusCode start() override {
      auto sc = extends::start();
      if ( !sc ) return sc;
      return m_jos.as<IStateful>()->start();
    }
    StatusCode stop() override {
      auto sc = extends::stop();
      if ( !sc ) return sc;
      return m_jos.as<IStateful>()->stop();
    }
    StatusCode finalize() override {
      auto sc = m_jos.as<IStateful>()->finalize();
      if ( !sc ) return sc;
      return extends::finalize();
    }

    // forward IJobOptionsSvc calls
    StatusCode setMyProperties( const std::string& client, IProperty* me ) override {
      return m_jos->setMyProperties( client, me );
    }
    StatusCode addPropertyToCatalogue( const std::string&                  client,
                                       const Gaudi::Details::PropertyBase& property ) override {
      return m_jos->addPropertyToCatalogue( client, property );
    }
    StatusCode removePropertyFromCatalogue( const std::string& client, const std::string& name ) override {
      return m_jos->removePropertyFromCatalogue( client, name );
    }
    const std::vector<const Gaudi::Details::PropertyBase*>* getProperties( const std::string& client ) const override {
      return m_jos->getProperties( client );
    }
    const Gaudi::Details::PropertyBase* getClientProperty( const std::string& client,
                                                           const std::string& name ) const override {
      return m_jos->getClientProperty( client, name );
    }
    std::vector<std::string> getClients() const override { return m_jos->getClients(); }
    StatusCode               readOptions( const std::string& file, const std::string& path = "" ) override {
      return m_jos->readOptions( file, path );
    }

    // adapt IOptionsSvc interface
    void set( const std::string& key, const std::string& value ) override {
      auto [client, name] = i_splitKey( key );
      Gaudi::Property<std::string> p;
      p.setName( name );
      p.fromString( value ).ignore( /* cannot fail */ );
      if ( auto sc = addPropertyToCatalogue( client, p ); !sc ) {
        throw GaudiException{"failed to set property " + key + " to " + value, name, sc};
      }
    }
    std::string get( const std::string& key, const std::string& default_ ) const override {
      auto [client, name] = i_splitKey( key );
      auto p              = getClientProperty( client, name );
      if ( !p ) return default_;
      return p->toString();
    }
    std::string pop( const std::string& key, const std::string& default_ ) override {
      auto [client, name] = i_splitKey( key );
      auto value          = get( key, default_ );
      removePropertyFromCatalogue( client, name ).ignore();
      return value;
    }
    bool has( const std::string& key ) const override {
      auto [client, name] = i_splitKey( key );
      return getClientProperty( client, name );
    }
    bool isSet( const std::string& key ) const override {
      return has( key ); // in IJobOptionsSvc a property is set only if registered
    }

    std::vector<std::tuple<std::string, std::string>> items() const override {
      std::vector<std::tuple<std::string, std::string>> data;
      for ( auto& client : getClients() ) {
        for ( auto p : *getProperties( client ) ) { data.emplace_back( client + p->name(), p->toString() ); }
      }
      return data;
    }

    void bind( const std::string& prefix, Gaudi::Details::PropertyBase* property ) override {
      // in IJobOptionsSvc bind just set client property
      auto p = getClientProperty( prefix, property->name() );
      if ( p ) property->assign( *p );
    }

    void broadcast( const std::regex&, const std::string&, OnlyDefaults ) override {
      // no-op: cannot be implemented without actual property binding
    }

  private:
    inline static std::tuple<std::string, std::string> i_splitKey( const std::string& key ) {
      auto sep = key.rfind( '.' );
      if ( sep == key.npos ) { throw std::invalid_argument{"cannot split key without at least one '.'"}; }
      return {key.substr( 0, sep ), key.substr( sep + 1 )};
    }

    SmartIF<IJobOptionsSvc> m_jos;
  };
} // namespace Gaudi::Details

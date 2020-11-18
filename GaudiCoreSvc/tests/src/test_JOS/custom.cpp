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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_JOS
#include <boost/test/unit_test.hpp>

// here we have to include the IJobOptionsSvc deprecated header, so we silence the warning
#define GAUDI_INTERNAL_NO_IJOBOPTIONSSVC_H_DEPRECATION 1

#include "fixture.h"

#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/IJobOptionsSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Service.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <vector>

/*
Test that we can use custom implementations of IJobOptionsSvc that do not implement IOptionsSvc.
*/

/// Minimal IJobOptionsSvc implementation for testing.
struct CustomJOS : extends<Service, IJobOptionsSvc> {
  using extends::extends;
  using Service::getProperties;

  ~CustomJOS() {
    for ( auto& c : m_catalog ) {
      for ( auto p : *c.second ) { delete p; }
      delete c.second;
    }
  }

  StatusCode setMyProperties( const std::string& client, IProperty* me ) override {
    debug() << "setMyProperties(" << client << ", " << me << ')' << endmsg;
    auto props = getProperties( client );
    if ( props )
      for ( auto p : *props )
        me->setProperty( *p )
            .orElse( [&]() { warning() << "failed to set " << client << '.' << p->name() << endmsg; } )
            .ignore();
    return StatusCode::SUCCESS;
  }
  StatusCode addPropertyToCatalogue( const std::string&                  client,
                                     const Gaudi::Details::PropertyBase& property ) override {
    debug() << "addPropertyToCatalogue(" << client << ", " << property << ')' << endmsg;
    auto p = std::make_unique<Gaudi::Property<std::string>>( property.name(), property.toString() );
    debug() << "  " << *p << endmsg;
    if ( !getProperties( client ) ) m_catalog[client] = new std::vector<const Gaudi::Details::PropertyBase*>{};
    m_catalog[client]->push_back( p.release() );
    return StatusCode::SUCCESS;
  }
  StatusCode removePropertyFromCatalogue( const std::string& client, const std::string& name ) override {
    debug() << "removePropertyFromCatalogue(" << client << ", " << name << ')' << endmsg;
    auto it = m_catalog.find( client );
    if ( it != end( m_catalog ) ) {
      auto props = it->second;
      props->erase( remove_if( props->begin(), props->end(), [&name]( auto p ) { return p->name() == name; } ),
                    props->end() );
    }
    return StatusCode::SUCCESS;
  }
  const std::vector<const Gaudi::Details::PropertyBase*>* getProperties( const std::string& client ) const override {
    debug() << "getProperties(" << client << ')' << endmsg;
    auto it = m_catalog.find( client );
    return ( it != end( m_catalog ) ) ? it->second : nullptr;
  }
  const Gaudi::Details::PropertyBase* getClientProperty( const std::string& client,
                                                         const std::string& name ) const override {
    debug() << "getClientProperty(" << client << ", " << name << ')' << endmsg;
    const Gaudi::Details::PropertyBase* prop = nullptr;

    auto props = getProperties( client );
    if ( props ) {
      debug() << props << ' ' << props->size() << endmsg;
      for ( auto tmp : *props ) {
        debug() << "  -> " << *tmp << endmsg;
        if ( tmp->name() == name ) prop = tmp;
      }
    }
    debug() << "   --> " << prop << endmsg;
    return prop;
  }
  std::vector<std::string> getClients() const override {
    debug() << "getClients()" << endmsg;
    std::vector<std::string> data;
    data.reserve( m_catalog.size() );
    for_each( begin( m_catalog ), end( m_catalog ), [&data]( auto item ) { data.emplace_back( item.first ); } );
    return data;
  }
  StatusCode readOptions( const std::string&, const std::string& ) override { return StatusCode::SUCCESS; }

  Gaudi::Property<std::string> m_type{this, "TYPE"};
  Gaudi::Property<std::string> m_path{this, "PATH"};

  std::map<std::string, std::vector<const Gaudi::Details::PropertyBase*>*> m_catalog;
};

DECLARE_COMPONENT( CustomJOS )

BOOST_AUTO_TEST_CASE( JOS_adapter ) {
  Fixture f{"CustomJOS"};

  auto& jos = Gaudi::svcLocator()->getOptsSvc();

  TestPropertyHolder           ph;
  Gaudi::Property<std::string> p1{&ph, "p1", ""};
  Gaudi::Property<int>         p2{&ph, "p2", 0};

  jos.set( "test.p1", "'v2'" );
  jos.set( "test.p2", "10" );

  ph.bindPropertiesTo( jos );

  BOOST_CHECK( jos.has( "test.p1" ) );
  BOOST_CHECK_EQUAL( jos.get( "test.p1" ), "v2" );
  BOOST_CHECK( jos.has( "test.p2" ) );
  BOOST_CHECK_EQUAL( jos.get( "test.p2" ), "10" );
  BOOST_CHECK( !jos.has( "test.no" ) );
}

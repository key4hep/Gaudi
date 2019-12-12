/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// ===========================================================================
// STD & STL:
// ===========================================================================
#include <iostream>
#include <sstream>
// ===========================================================================
// Boost:
// ===========================================================================
#include "boost/algorithm/string.hpp"
// ===========================================================================
// Local
// ===========================================================================
#include "SvcCatalog.h"
// ===========================================================================
namespace {
  constexpr struct select1st_t {
    template <typename S, typename T>
    const S& operator()( const std::pair<S, T>& p ) const {
      return p.first;
    }
    template <typename S, typename T>
    S& operator()( std::pair<S, T>& p ) const {
      return p.first;
    }
  } select1st{};
} // namespace
// ===========================================================================
SvcCatalog::~SvcCatalog() {
  for ( const auto& cur : m_catalog ) {
    for ( auto& prop : cur.second ) delete prop;
  }
}
// ============================================================================
StatusCode SvcCatalog::addProperty( const std::string& client, const Gaudi::Details::PropertyBase* property ) {
  auto props = findProperties( client );
  if ( props ) {
    removeProperty( client, property->name() ).ignore();
    props->push_back( property );
  } else {
    m_catalog.emplace( client, PropertiesT{property} );
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode SvcCatalog::removeProperty( const std::string& client, const std::string& name ) {
  auto props = findProperties( client );
  if ( props ) {
    auto res = findProperty( *props, name );
    if ( res.first ) {
      delete *res.second;
      props->erase( res.second );
    }
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
const SvcCatalog::PropertiesT* SvcCatalog::getProperties( const std::string& client ) const {
  return findProperties( client );
}
// ============================================================================
std::vector<std::string> SvcCatalog::getClients() const {
  std::vector<std::string> result;
  result.reserve( m_catalog.size() );
  std::transform( std::begin( m_catalog ), std::end( m_catalog ), std::back_inserter( result ), select1st );
  return result;
}
// ============================================================================
const SvcCatalog::PropertiesT* SvcCatalog::findProperties( const std::string& client ) const {
  auto result = m_catalog.find( client );
  return ( result != m_catalog.end() ) ? &result->second : nullptr;
}
// ============================================================================
SvcCatalog::PropertiesT* SvcCatalog::findProperties( const std::string& client ) {
  auto result = m_catalog.find( client );
  return ( result != m_catalog.end() ) ? &result->second : nullptr;
}
// ============================================================================
std::pair<bool, SvcCatalog::PropertiesT::const_iterator> SvcCatalog::findProperty( const SvcCatalog::PropertiesT& props,
                                                                                   const std::string& name ) const {
  auto p = std::find_if( std::begin( props ), std::end( props ), [&]( const Gaudi::Details::PropertyBase* prop ) {
    return boost::iequals( name, prop->name() );
  } );
  return {p != std::end( props ), p};
}
// ============================================================================
std::pair<bool, SvcCatalog::PropertiesT::iterator> SvcCatalog::findProperty( SvcCatalog::PropertiesT& props,
                                                                             const std::string&       name ) {
  auto p = std::find_if( std::begin( props ), std::end( props ), [&]( const Gaudi::Details::PropertyBase* prop ) {
    return boost::iequals( name, prop->name() );
  } );
  return {p != std::end( props ), p};
}
// ============================================================================
std::ostream& SvcCatalog::fillStream( std::ostream& o ) const {
  // loop over the clients:
  for ( const auto& iclient : m_catalog ) {
    o << "Client '" << iclient.first << "'" << std::endl;
    for ( const auto& p : iclient.second ) {
      if ( p ) o << "\t" << ( *p ) << std::endl;
    }
  }
  //
  return o; // RETURN
}
const Gaudi::Details::PropertyBase* SvcCatalog::getProperty( const std::string& client,
                                                             const std::string& name ) const {
  auto props = findProperties( client );
  if ( props ) {
    const auto res = findProperty( *props, name );
    if ( res.first ) { return *res.second; }
  }
  return nullptr;
}
// ============================================================================
// printoput operator
// ============================================================================
std::ostream& operator<<( std::ostream& o, const SvcCatalog& c ) { return c.fillStream( o ); }
// ============================================================================
// The END
// ============================================================================

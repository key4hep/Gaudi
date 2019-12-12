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
#include "Catalog.h"
// ============================================================================
// Boost:
// ============================================================================
#include <boost/format.hpp>
// ============================================================================
// Namesapce aliases:
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
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
// ============================================================================
std::vector<std::string> gp::Catalog::ClientNames() const {
  std::vector<std::string> result;
  std::transform( std::begin( catalog_ ), std::end( catalog_ ), std::back_inserter( result ), select1st );
  return result;
}
// ============================================================================
bool gp::Catalog::Add( Property* property ) {
  assert( property );
  auto it = catalog_.find( property->ClientName() );
  if ( it == catalog_.end() ) {
    CatalogSet::mapped_type properties;
    properties.insert( property );
    catalog_.emplace( property->ClientName(), properties );
    return true;
  }
  it->second.erase( *property );
  it->second.insert( property );
  // TODO: check return value
  return true;
}
// ============================================================================
gp::Property* gp::Catalog::Find( const std::string& client, const std::string& name ) {
  auto it = catalog_.find( client );
  if ( it == catalog_.end() ) return nullptr;

  auto pit = std::find_if( it->second.begin(), it->second.end(), Property::Equal( name ) );
  return ( pit != it->second.end() ) ? &*pit : nullptr;
}
// ============================================================================
std::string gp::Catalog::ToString() const {
  std::string result;
  for ( const auto& client : catalog_ ) {
    for ( const auto& current : client.second ) { result += current.ToString() + "\n"; }
  }
  return result;
}
// ============================================================================
// print the content of the catalogue to std::ostream
// ============================================================================
std::ostream& Gaudi::Parsers::Catalog::fillStream( std::ostream& o ) const {
  o << "// " << std::string( 82, '=' ) << std::endl
    << "//       Parser catalog " << std::endl
    << "// " << std::string( 82, '=' ) << std::endl;

  size_t nComponents = 0;
  size_t nProperties = 0;

  for ( const auto& client : catalog_ ) {
    o << boost::format( "// Properties of '%1%' %|43t|# = %2%" ) % client.first % client.second.size() << std::endl;
    ++nComponents;
    nProperties += client.second.size();
    for ( const auto& current : client.second ) {
      o << boost::format( "%1%   %|44t| = %2% ; " ) % current.FullName() % current.ValueAsString() << '\n';
    }
  }
  o << "// " << std::string( 82, '=' ) << '\n'
    << boost::format( "// End parser catalog #Components=%1% #Properties=%2%" ) % nComponents % nProperties << '\n'
    << "// " << std::string( 82, '=' ) << std::endl;
  return o;
}

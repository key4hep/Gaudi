/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Catalog.h"
#include <fmt/format.h>

namespace gp = Gaudi::Parsers;
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
gp::Property* gp::Catalog::Find( std::string_view client, std::string_view name ) {
  auto it = catalog_.find( client );
  if ( it == catalog_.end() ) return nullptr;

  auto pit = std::find_if( it->second.begin(), it->second.end(),
                           [&]( const Property& property ) { return name == property.NameInClient(); } );

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
  o << R"(// ==================================================================================
//       Parser catalog
// ==================================================================================
)";

  size_t nComponents = 0;
  size_t nProperties = 0;

  for ( const auto& client : catalog_ ) {
    o << fmt::format( "// Properties of {:<25} # = {}", fmt::format( "'{}'", client.first ), client.second.size() );
    ++nComponents;
    nProperties += client.second.size();
    for ( const auto& current : client.second ) {
      o << fmt::format( "{:<44} = {} ;", current.FullName(), current.ValueAsString() );
    }
  }
  o << fmt::format( R"(// ==================================================================================
// End parser catalog #Components={} #Properties={}
// ==================================================================================
)",
                    nComponents, nProperties );
  return o;
}

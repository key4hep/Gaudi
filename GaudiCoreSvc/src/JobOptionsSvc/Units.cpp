/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Units.h"

namespace gp = Gaudi::Parsers;

bool gp::Units::Add( std::string name, double value ) { return Add( std::move( name ), value, Position() ); }

bool gp::Units::Add( std::string name, double value, const Position& pos ) {
  return units_.emplace( std::move( name ), ValueWithPosition( value, pos ) ).second;
}

bool gp::Units::Find( std::string_view name, double& result ) const {
  ValueWithPosition r;
  if ( !Find( name, r ) ) return false;
  result = r.first;
  return true;
}

bool gp::Units::Find( std::string_view name, ValueWithPosition& result ) const {
  auto it = units_.find( name );
  if ( it == units_.end() ) return false;
  result = it->second;
  return true;
}

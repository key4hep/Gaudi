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
#include "PropertyValue.h"
#include <GaudiKernel/SerializeSTL.h>
#include <GaudiKernel/compose.h>
#include <algorithm>
#include <sstream>

namespace gp = Gaudi::Parsers;

bool               gp::PropertyValue::IsSimple() const { return std::holds_alternative<std::string>( value_ ); }
bool               gp::PropertyValue::IsVector() const { return std::holds_alternative<VectorOfStrings>( value_ ); }
bool               gp::PropertyValue::IsMap() const { return std::holds_alternative<MapOfStrings>( value_ ); }
gp::PropertyValue& gp::PropertyValue::operator+=( const PropertyValue& right ) {

  if ( IsReference() ) { throw PropertyValueException::WrongLValue(); }
  std::visit(
      Gaudi::overload(
          []( std::string&, const auto& ) { throw PropertyValueException::WrongLValue(); },
          []( VectorOfStrings& lhs, const std::string& rhs ) { lhs.push_back( rhs ); },
          []( VectorOfStrings& lhs, const VectorOfStrings& rhs ) { lhs.insert( lhs.end(), rhs.begin(), rhs.end() ); },
          []( MapOfStrings& lhs, const MapOfStrings& rhs ) { lhs.insert( rhs.begin(), rhs.end() ); },
          []( auto&, const auto& ) { throw PropertyValueException::WrongRValue(); } ),
      value_, right.value_ );
  return *this;
}

const gp::PropertyValue gp::PropertyValue::operator+( const PropertyValue& right ) {
  return PropertyValue{ *this } += right;
}

gp::PropertyValue& gp::PropertyValue::operator-=( const PropertyValue& right ) {
  if ( IsReference() ) { throw PropertyValueException::WrongLValue(); }
  std::visit( Gaudi::overload( []( VectorOfStrings&   lhs,
                                   const std::string& rhs ) { lhs.erase( std::find( lhs.begin(), lhs.end(), rhs ) ); },
                               []( VectorOfStrings& lhs, const VectorOfStrings& rhs ) {
                                 for ( const auto& item : rhs ) lhs.erase( std::find( lhs.begin(), lhs.end(), item ) );
                               },
                               []( MapOfStrings& lhs, const std::string& rhs ) { lhs.erase( rhs ); },
                               []( MapOfStrings& lhs, const VectorOfStrings& rhs ) {
                                 for ( const auto& item : rhs ) lhs.erase( item );
                               },
                               []( std::string&, const auto& ) { throw PropertyValueException::WrongLValue(); },
                               []( auto&, const auto& ) { throw PropertyValueException::WrongRValue(); } ),
              value_, right.value_ );
  return *this;
}

const gp::PropertyValue gp::PropertyValue::operator-( const PropertyValue& right ) {
  return PropertyValue{ *this } -= right;
}

std::string gp::PropertyValue::ToString() const {
  if ( IsReference() ) {
    const auto& value = std::get<VectorOfStrings>( value_ );
    if ( value.at( 0 ) != "" ) {
      return "@" + value.at( 0 ) + "." + value.at( 1 );
    } else {
      return "@" + value.at( 0 );
    }
  }
  return std::visit(
      []( const auto& v ) {
        // make sure to be consistent by explicitly re-using `GaudiUtils::operator<<`
        using GaudiUtils::operator<<; // cppcheck-suppress constStatement
        std::ostringstream s{};
        s << v;
        return s.str();
      },
      value_ );
}

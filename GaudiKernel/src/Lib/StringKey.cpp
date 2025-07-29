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
#include <Gaudi/Parsers/CommonParsers.h>
#include <GaudiKernel/StringKey.h>
#include <GaudiKernel/ToStream.h>

/** @file
 *  Implementation file for class Gaudi::StringKey
 *  @see Gaudi::StringKey
 *  @date 2009-04-08
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 */

std::string        Gaudi::StringKey::toString() const { return Gaudi::Utils::toString( m_str ); }
const std::string& Gaudi::StringKey::__str__() const { return m_str; }
std::string        Gaudi::StringKey::__repr__() const { return toString(); }
bool               Gaudi::StringKey::__eq__( const Gaudi::StringKey& right ) const { return *this == right; }
bool               Gaudi::StringKey::__eq__( std::string_view right ) const { return *this == right; }
bool               Gaudi::StringKey::__neq__( const Gaudi::StringKey& right ) const { return *this != right; }
bool               Gaudi::StringKey::__neq__( std::string_view right ) const { return *this != right; }
std::ostream&      Gaudi::Utils::toStream( const Gaudi::StringKey& key, std::ostream& s ) {
  return Gaudi::Utils::toStream( key.str(), s );
}
StatusCode Gaudi::Parsers::parse( Gaudi::StringKey& result, std::string_view input ) {
  std::string _result;
  StatusCode  sc = parse( _result, input );
  result         = Gaudi::StringKey( _result );
  return sc;
}
StatusCode Gaudi::Parsers::parse( std::vector<Gaudi::StringKey>& result, std::string_view input ) {
  result.clear();
  typedef std::vector<std::string> Strings;
  Strings                          _result;
  StatusCode                       sc = parse( _result, input );
  if ( sc.isFailure() ) { return sc; }
  result.reserve( _result.size() );

  std::copy( _result.begin(), _result.end(), std::back_inserter( result ) );

  return StatusCode::SUCCESS;
}

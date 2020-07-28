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
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/ToStream.h"
#include <Gaudi/Parsers/CommonParsers.h>
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::StringKey
 *  @see Gaudi::StringKey
 *  @date 2009-04-08
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 */
// ============================================================================
// constructor from the string: perform hashing
// ============================================================================
Gaudi::StringKey::StringKey( std::string key )
    : m_str( std::move( key ) )
    , m_hash( std::hash<std::string>()( m_str ) ) // NB! hashing here!!!
{}
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey::toString() const { return Gaudi::Utils::toString( m_str ); }
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey::__str__() const { return m_str; }
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey::__repr__() const { return toString(); }
// ============================================================================
// equality operator for python
bool Gaudi::StringKey::__eq__( const Gaudi::StringKey& right ) const { return *this == right; }
// ============================================================================
// equality operators for python
// ============================================================================
bool Gaudi::StringKey::__eq__( std::string_view right ) const { return *this == right; }
// ============================================================================
// non-equality operator for python
// ============================================================================
bool Gaudi::StringKey::__neq__( const Gaudi::StringKey& right ) const { return *this != right; }
// ============================================================================
// non-equality operator for python
// ============================================================================
bool Gaudi::StringKey::__neq__( std::string_view right ) const { return *this != right; }
// ============================================================================
/*  send the object to stream (needed to use it as property)
 *  @see Gaudi::Utils::toString
 *  @see Gaudi::Utils::toStream
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const Gaudi::StringKey& key, std::ostream& s ) {
  return Gaudi::Utils::toStream( key.str(), s );
}
// ============================================================================
/*  parse the key from the string
 *  @attention: this function is needed to use it as property
 *  @param result (OUTPUT) the parsing result
 *  @param input the input string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( Gaudi::StringKey& result, const std::string& input ) {
  std::string _result;
  StatusCode  sc = parse( _result, input );
  result         = Gaudi::StringKey( _result );
  return sc;
}
// ============================================================================
/*  parse the vector of keys from the string
 *  @see Gaudi::Parsers
 *  @see Gaudi::Parsers::parse
 *  @see Gaudi::StringKey
 *  @attention: this function is needed to use it as property
 *  @param result (OUTPUT) the parsing result
 *  @param input the input string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( std::vector<Gaudi::StringKey>& result, const std::string& input ) {
  result.clear();
  typedef std::vector<std::string> Strings;
  Strings                          _result;
  StatusCode                       sc = parse( _result, input );
  if ( sc.isFailure() ) { return sc; } // RETURN
  result.reserve( _result.size() );
  //
  std::copy( _result.begin(), _result.end(), std::back_inserter( result ) );
  //
  return StatusCode::SUCCESS;
}

// ============================================================================
// The END
// ============================================================================

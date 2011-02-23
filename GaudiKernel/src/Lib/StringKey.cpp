// $Id: $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/ToStream.h"
#include "GaudiKernel/StringKey.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/functional/hash.hpp"
#include "boost/bind.hpp"
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
Gaudi::StringKey::StringKey ( const std::string& key ) 
  : m_str  ( key )
  , m_hash ( boost::hash_value( key ) )  // NB! hashing here!!!
{}
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey::toString() const 
{ return Gaudi::Utils::toString ( m_str ) ; }
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey:: __str__ () const { return toString() ; }
// ============================================================================
// the representation of the object
// ============================================================================
std::string Gaudi::StringKey:: __repr__() const { return toString() ; }
// ============================================================================
// equality operator for python 
bool Gaudi::StringKey::__eq__    ( const Gaudi::StringKey&   right ) const 
{ return this == &right || *this == right ; }
// ============================================================================
// equality operators for python 
// ============================================================================
bool Gaudi::StringKey::__eq__    ( const std::string&        right ) const 
{ return *this == right ; }
// ============================================================================
// non-equality operator for python 
// ============================================================================
bool Gaudi::StringKey::__neq__   ( const Gaudi::StringKey&   right ) const 
{ return this != &right && *this != right ; }
// ============================================================================
// non-equality operator for python 
// ============================================================================
bool Gaudi::StringKey::__neq__   ( const std::string&        right ) const 
{ return *this != right ; }
// ============================================================================
/*  send the object to stream (needed to use it as property)
 *  @see Gaudi::Utils::toString
 *  @see Gaudi::Utils::toStream
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream 
( const Gaudi::StringKey& key , std::ostream& s ) 
{ return Gaudi::Utils::toStream ( key.str() , s ) ; }
// ============================================================================
/*  parse the key from the string
 *  @attention: this function is needed to use it as property
 *  @param result (OUTPUT) the parsing result
 *  @param input the input string
 *  @return status code 
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse 
( Gaudi::StringKey&   result , 
  const std::string&  input  ) 
{
  std::string _result ;
  StatusCode sc = parse ( _result , input ) ;
  result = Gaudi::StringKey ( _result ) ;
  return sc ;
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
StatusCode Gaudi::Parsers::parse 
( std::vector<Gaudi::StringKey>&  result , 
  const std::string&              input  ) 
{
  result.clear() ;
  typedef std::vector<std::string> Strings ;
  Strings _result ;
  StatusCode sc = parse ( _result , input ) ;
  if ( sc.isFailure() ) { return sc ; }                              // RETURN 
  result.reserve ( _result.size() ) ;
  //
  std::copy ( _result.begin() , 
              _result.end  () , std::back_inserter ( result ) ) ;
  //
  return StatusCode::SUCCESS ;
}

  


// ============================================================================
// The END 
// ============================================================================


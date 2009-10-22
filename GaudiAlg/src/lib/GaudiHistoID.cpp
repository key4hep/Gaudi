// $Id:$ 
// ============================================================================
/*  @file gaudiHistoID.cpp
 *
 *  Implementation file for class GaudiAlg::ID
 *
 *  $Id: GaudiHistoID.cpp,v 1.1 2006/03/09 14:50:22 hmd Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   03/02/2006
 */
// ============================================================================
// Include Files
// ============================================================================
// STD & STL 
// ============================================================================
#include <sstream>
// ============================================================================
// local
// ============================================================================
#include "GaudiAlg/GaudiHistoID.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/functional/hash.hpp"
// ============================================================================
// Implicit constructor from a numeric ID
// ============================================================================
GaudiAlg::ID::ID ( const GaudiAlg::ID::NumericID id ) 
  : m_nID  ( id ) 
  , m_aID  ( "" )
  , m_hash ( boost::hash_value ( id ) ) 
{}
// ============================================================================
// Implicit constructor from a literal ID
// ============================================================================
GaudiAlg::ID::ID ( const GaudiAlg::ID::LiteralID& id ) 
  : m_nID ( -1 ) 
  , m_aID ( id ) 
  , m_hash ( boost::hash_value ( id ) ) 
{}
// ============================================================================
// Implicit constructor from a literal ID
// ============================================================================
GaudiAlg::ID::ID ( const char* id  ) 
  : m_nID  ( -1 ) 
  , m_aID  ( id ) 
  , m_hash ( 0 ) 
{ m_hash = boost::hash_value ( m_aID ) ; }
// ============================================================================
GaudiAlg::ID::LiteralID GaudiAlg::ID::idAsString() const
{
  std::ostringstream s ;
  fillStream ( s ) ;
  return s.str() ;
}
// ============================================================================
// Operator overloading for ostream
// ============================================================================
std::ostream& GaudiAlg::ID::fillStream( std::ostream& s ) const
{
  if      ( numeric() ) { s << numericID() ; }
  else if ( literal() ) { s << literalID() ; }
  else                  { s << "UNDEFINED" ; }
  return s ;
}
// ============================================================================
// The END 
// ============================================================================

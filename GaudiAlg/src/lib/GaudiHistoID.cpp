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
/*  @file gaudiHistoID.cpp
 *
 *  Implementation file for class GaudiAlg::ID
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
GaudiAlg::ID::LiteralID GaudiAlg::ID::idAsString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}
// ============================================================================
// Operator overloading for ostream
// ============================================================================
std::ostream& GaudiAlg::ID::fillStream( std::ostream& s ) const {
  if ( numeric() ) {
    s << numericID();
  } else if ( literal() ) {
    s << literalID();
  } else {
    s << "UNDEFINED";
  }
  return s;
}
// ============================================================================
// The END
// ============================================================================

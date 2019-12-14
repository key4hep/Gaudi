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
#include "Position.h"
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/format.hpp>
std::string Gaudi::Parsers::Position::ToString() const {
  return str( boost::format( "%1%: (%2%, %3%)" ) % filename_ % line_ % column_ );
}

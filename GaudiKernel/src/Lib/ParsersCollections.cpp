/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <GaudiKernel/VectorMap.h>
// ============================================================================
#include <Gaudi/Parsers/Factory.h>
// ============================================================================

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<std::string, double>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<Gaudi::StringKey, double>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}
// ============================================================================

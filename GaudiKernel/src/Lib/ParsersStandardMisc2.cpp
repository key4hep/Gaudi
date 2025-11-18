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
#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse( std::vector<std::vector<double>>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::vector<std::vector<int>>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<int, int>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<int, double>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

PARSERS_DEF_FOR_STRMAP( std::string )

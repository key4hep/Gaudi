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
#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse( std::map<unsigned int, std::string>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::string& name, std::string& value, std::string_view input ) {
  Skipper                                      skipper;
  KeyValueGrammar<IteratorT, Skipper>          g;
  KeyValueGrammar<IteratorT, Skipper>::ResultT result;
  auto                                         iter = input.begin();
  bool parse_result = qi::phrase_parse( iter, input.end(), g, skipper, result ) && ( iter == input.end() );
  if ( parse_result ) {
    name  = result.first;
    value = result.second;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, std::pair<double, double>>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, std::pair<int, int>>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

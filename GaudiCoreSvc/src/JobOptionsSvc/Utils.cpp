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
#include "Utils.h"
#include <GaudiKernel/System.h>
#include <regex>

namespace gpu = Gaudi::Parsers::Utils;

std::string gpu::replaceEnvironments( std::string_view input ) {
  static const std::regex expression( "\\$(([A-Za-z0-9_]+)|\\(([A-Za-z0-9_]+)\\))" );

  std::string                                          result = std::string{ input };
  auto                                                 start  = input.begin();
  auto                                                 end    = input.end();
  std::match_results<std::string_view::const_iterator> what;
  auto                                                 flags = std::regex_constants::match_default;
  std::size_t                                          from  = 0;
  while ( std::regex_search( start, end, what, expression, flags ) ) {
    if ( std::string env; System::getEnv( what[what[2].matched ? 2 : 3].str(), env ) ) {
      auto pos = result.find( what[0].first, from, what[0].length() );
      result.replace( pos, what[0].length(), env );
      from = pos + env.size();
    }
    start = what[0].second;
    // update flags:
    flags |= std::regex_constants::match_prev_avail;
  }
  return result;
}

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
// ===========================================================================
// Include files
// ===========================================================================
#include "Utils.h"
// ===========================================================================
// Boost
// ===========================================================================
#include <boost/algorithm/string.hpp>
#include <regex>
// ===========================================================================
// Gaudi
// ===========================================================================
#include <GaudiKernel/System.h>
// ===========================================================================

namespace gpu = Gaudi::Parsers::Utils;

std::string gpu::replaceEnvironments( std::string_view input ) {
  static const std::regex expression( "\\$(([A-Za-z0-9_]+)|\\(([A-Za-z0-9_]+)\\))" );

  std::string                                          result = std::string{ input };
  auto                                                 start  = input.begin();
  auto                                                 end    = input.end();
  std::match_results<std::string_view::const_iterator> what;
  auto                                                 flags = std::regex_constants::match_default;
  while ( std::regex_search( start, end, what, expression, flags ) ) {
    std::string var{ what[2].first, what[2].second };
    if ( var.empty() ) var = std::string{ what[3].first, what[3].second };
    std::string env;
    if ( System::getEnv( var, env ) ) {
      boost::algorithm::replace_first( result, std::string{ what[0].first, what[0].second }, env );
    }
    start = what[0].second;
    // update flags:
    flags |= std::regex_constants::match_prev_avail;
  }
  return result;
}
// ===========================================================================

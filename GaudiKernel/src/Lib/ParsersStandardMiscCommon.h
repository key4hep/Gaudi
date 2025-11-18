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
#pragma once

#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/StringKey.h>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#define PARSERS_DEF_FOR_STRMAP( ValueType )                                                                            \
  StatusCode Gaudi::Parsers::parse( std::map<std::string, ValueType>& result, std::string_view input ) {               \
    return Gaudi::Parsers::parse_( result, input );                                                                    \
  }                                                                                                                    \
  StatusCode Gaudi::Parsers::parse( std::map<std::string, ValueType, std::less<>>& result, std::string_view input ) {  \
    return Gaudi::Parsers::parse_( result, input );                                                                    \
  }

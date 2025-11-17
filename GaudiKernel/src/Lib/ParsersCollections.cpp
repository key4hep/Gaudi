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
/// @fixme workaround for a warning in a Boost spirit header
/// ```
/// include/boost/spirit/.../real_impl.hpp:337:45: warning: 'n' may be used uninitialized [-Wmaybe-uninitialized]
///   337 |             traits::assign_to(traits::negate(neg, n), attr);
///       |                               ~~~~~~~~~~~~~~^~~~~~~~
/// ```
#ifndef __clang__
#  pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/VectorMap.h>

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<std::string, double>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<Gaudi::StringKey, double>& result, std::string_view input ) {
  return Gaudi::Parsers::parse_( result, input );
}

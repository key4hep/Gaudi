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

#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/Factory.h>
#include <string>
#include <string_view>

#define PARSERS_DEF_FOR_SINGLE( Type )                                                                                 \
  StatusCode Gaudi::Parsers::parse( Type& result, std::string_view input ) {                                           \
    return Gaudi::Parsers::parse_( result, input );                                                                    \
  }

PARSERS_DEF_FOR_SINGLE( bool )
PARSERS_DEF_FOR_SINGLE( char )
PARSERS_DEF_FOR_SINGLE( unsigned char )
PARSERS_DEF_FOR_SINGLE( signed char )
PARSERS_DEF_FOR_SINGLE( int )
PARSERS_DEF_FOR_SINGLE( short )
PARSERS_DEF_FOR_SINGLE( unsigned short )
PARSERS_DEF_FOR_SINGLE( unsigned int )
PARSERS_DEF_FOR_SINGLE( long )
PARSERS_DEF_FOR_SINGLE( unsigned long )
PARSERS_DEF_FOR_SINGLE( long long )
PARSERS_DEF_FOR_SINGLE( unsigned long long )
PARSERS_DEF_FOR_SINGLE( double )
PARSERS_DEF_FOR_SINGLE( float )
PARSERS_DEF_FOR_SINGLE( long double )
PARSERS_DEF_FOR_SINGLE( std::string )

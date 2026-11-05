/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <GaudiUtils/QuasiRandom.h>

#include <cstdint>
#include <string>

namespace Gaudi::Utils::QuasiRandom {
  std::uint32_t mix( std::uint32_t state ) {
    state += ( state << 16 );
    state ^= ( state >> 13 );
    state += ( state << 4 );
    state ^= ( state >> 7 );
    state += ( state << 10 );
    state ^= ( state >> 5 );
    state += ( state << 8 );
    state ^= ( state >> 16 );
    return state;
  }

  std::uint32_t mix32( std::uint32_t state, std::uint32_t extra ) { return mix( state + extra ); }

  std::uint32_t mix64( std::uint32_t state, uint64_t extra ) {
    state = mix32( state, std::uint32_t( extra ) );
    return mix32( state, std::uint32_t( ( extra >> 32 ) ) );
  }

  std::uint32_t mixString( std::uint32_t state, const std::string& extra ) {
    // prefix name with ' ' until the size is a multiple of 4.
    std::string s = std::string( ( 4 - extra.size() % 4 ) % 4, ' ' ) + extra;
    for ( std::size_t i = 0; i < s.size() / 4; ++i ) {
      // FIXME: this might do something different on big endian vs. small endian machines...
      std::uint32_t x = std::uint32_t( s[i * 4] ) | std::uint32_t( s[i * 4 + 1] ) << 8 |
                        std::uint32_t( s[i * 4 + 2] ) << 16 | std::uint32_t( s[i * 4 + 3] ) << 24;
      state = mix32( state, x );
    }
    return state;
  }
} // namespace Gaudi::Utils::QuasiRandom

// Include files
#include <string>

// local
#include "GaudiUtils/QuasiRandom.h"

namespace Gaudi
{
  namespace Utils
  {
    namespace QuasiRandom
    {

      uint32_t mix( uint32_t state )
      {
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

      uint32_t mix32( uint32_t state, uint32_t extra ) { return mix( state + extra ); }

      uint32_t mix64( uint32_t state, uint64_t extra )
      {
        typedef boost::low_bits_mask_t<32> mask_t;
        state = mix32( state, uint32_t( extra & mask_t::sig_bits_fast ) );
        return mix32( state, uint32_t( ( extra >> 32 ) & mask_t::sig_bits_fast ) );
      }

      uint32_t mixString( uint32_t state, const std::string& extra )
      {
        // prefix name with ' ' until the size is a multiple of 4.
        std::string s = std::string( ( 4 - extra.size() % 4 ) % 4, ' ' ) + extra;
        for ( size_t i = 0; i < s.size() / 4; ++i ) {
          // FIXME: this might do something different on big endian vs. small endian machines...
          uint32_t x = uint32_t( s[i * 4] ) | uint32_t( s[i * 4 + 1] ) << 8 | uint32_t( s[i * 4 + 2] ) << 16 |
                       uint32_t( s[i * 4 + 3] ) << 24;
          state = mix32( state, x );
        }
        return state;
      }
    }
  }
}

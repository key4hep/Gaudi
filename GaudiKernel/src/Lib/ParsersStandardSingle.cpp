// ============================================================================
// Include files
// ============================================================================
#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/Factory.h>
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <set>
#include <string>
#include <vector>
// ============================================================================
// ============================================================================
#define PARSERS_DEF_FOR_SINGLE( Type )                                                                                 \
  StatusCode Gaudi::Parsers::parse( Type& result, const std::string& input ) {                                         \
    return Gaudi::Parsers::parse_( result, input );                                                                    \
  }
// ============================================================================
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
#if BOOST_VERSION <= 105500
PARSERS_DEF_FOR_SINGLE( float )
#else
// See GAUDI-1121.
StatusCode Gaudi::Parsers::parse( float& result, const std::string& input ) {
  double     tmp{0};
  StatusCode sc = Gaudi::Parsers::parse_( tmp, input );
  result        = static_cast<float>( tmp );
  return sc;
}
#endif
PARSERS_DEF_FOR_SINGLE( long double )
PARSERS_DEF_FOR_SINGLE( std::string )

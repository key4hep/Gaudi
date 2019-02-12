// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/VectorMap.h"
// ============================================================================
#include <Gaudi/Parsers/Factory.h>
// ============================================================================

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<std::string, double>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<GaudiUtils::VectorMap<std::string, double>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( GaudiUtils::VectorMap<Gaudi::StringKey, double>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<GaudiUtils::VectorMap<Gaudi::StringKey, double>>::parse_( result, input );
}
// ============================================================================

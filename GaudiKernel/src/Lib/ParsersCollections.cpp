// $Id:$
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/VectorMap.h"
// ============================================================================
#include "GaudiKernel/ParsersFactory.h"
// ============================================================================

StatusCode
Gaudi::Parsers::parse(GaudiUtils::VectorMap<std::string, double>& result,
		const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode
Gaudi::Parsers::parse(GaudiUtils::VectorMap<Gaudi::StringKey, double>& result,
		const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}
// ============================================================================

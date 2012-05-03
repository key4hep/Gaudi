// $Id:$
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Parsers.h"
// ============================================================================
#include "GaudiKernel/ParsersFactory.h"
// ============================================================================
StatusCode Gaudi::Parsers::parse(Gaudi::Histo1DDef& result,
		const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}
// ============================================================================
StatusCode Gaudi::Parsers::parse(std::map<std::string,
		Gaudi::Histo1DDef>& result,
		const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}
// ============================================================================

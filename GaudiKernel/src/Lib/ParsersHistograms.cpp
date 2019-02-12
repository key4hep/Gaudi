// ============================================================================
// Include files
// ============================================================================
#include <Gaudi/Parsers/CommonParsers.h>
// ============================================================================
#include <Gaudi/Parsers/Factory.h>
// ============================================================================
StatusCode Gaudi::Parsers::parse( Gaudi::Histo1DDef& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<Gaudi::Histo1DDef>::parse_( result, input );
}
// ============================================================================
StatusCode Gaudi::Parsers::parse( std::map<std::string, Gaudi::Histo1DDef>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, Gaudi::Histo1DDef>>::parse_( result, input );
}
// ============================================================================

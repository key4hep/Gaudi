#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse( std::map<std::string, int>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, int>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, unsigned int>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, unsigned int>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, double>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, double>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, std::vector<std::string>>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, std::vector<std::string>>>::parse_( result, input );
}

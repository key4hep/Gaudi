#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse( std::vector<std::vector<double>>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::vector<std::vector<double>>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<int, int>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<int, int>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<int, double>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<int, double>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::map<std::string, std::string>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::map<std::string, std::string>>::parse_( result, input );
}

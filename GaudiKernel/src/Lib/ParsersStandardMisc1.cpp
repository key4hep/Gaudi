#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse( std::pair<double, double>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::pair<double, double>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::pair<int, int>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::pair<int, int>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::vector<std::pair<double, double>>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::vector<std::pair<double, double>>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::vector<std::pair<int, int>>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::vector<std::pair<int, int>>>::parse_( result, input );
}

StatusCode Gaudi::Parsers::parse( std::vector<std::vector<std::string>>& result, const std::string& input ) {
  return Gaudi::Parsers::sparse<std::vector<std::vector<std::string>>>::parse_( result, input );
}

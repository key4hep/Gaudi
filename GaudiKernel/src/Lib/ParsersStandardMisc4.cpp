#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::vector<int> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::vector<double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<int, std::string>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

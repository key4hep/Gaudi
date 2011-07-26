#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse(std::vector<std::vector<double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<int, int>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<int, double>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::string>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

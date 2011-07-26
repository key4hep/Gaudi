#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse(std::map<std::string, int>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, unsigned int>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, double>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::vector<std::string> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

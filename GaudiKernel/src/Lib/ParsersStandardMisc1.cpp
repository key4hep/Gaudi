#include "ParsersStandardMiscCommon.h"

StatusCode Gaudi::Parsers::parse(std::pair<double,double>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::pair<int, int>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::vector<std::pair<double, double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::vector<std::pair<int, int> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::vector<std::vector<std::string> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

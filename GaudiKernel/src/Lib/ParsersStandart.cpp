// $Id$
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/ParsersFactory.h"
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
#include <set>
#include <map>
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
// ============================================================================
#define PARSERS_DEF_FOR_SINGLE(Type)\
  StatusCode Gaudi::Parsers::parse(Type& result, const std::string& input) {\
    return Gaudi::Parsers::parse_(result, input);\
  }
#define PARSERS_DEF_FOR_LIST(InnerType)\
    StatusCode Gaudi::Parsers::parse(std::vector<InnerType>& result,\
                          const std::string& input) {\
    return Gaudi::Parsers::parse_(result, input);\
    }\
    StatusCode Gaudi::Parsers::parse(std::set<InnerType>& result,\
                          const std::string& input) {\
    return Gaudi::Parsers::parse_(result, input);\
    }\
    StatusCode Gaudi::Parsers::parse(std::list<InnerType>& result,\
                          const std::string& input) {\
    return Gaudi::Parsers::parse_(result, input);\
    }
// ============================================================================
PARSERS_DEF_FOR_SINGLE(bool)
PARSERS_DEF_FOR_SINGLE(char)
PARSERS_DEF_FOR_SINGLE(unsigned char)
PARSERS_DEF_FOR_SINGLE(signed char)
PARSERS_DEF_FOR_SINGLE(int)
PARSERS_DEF_FOR_SINGLE(short)
PARSERS_DEF_FOR_SINGLE(unsigned short)
PARSERS_DEF_FOR_SINGLE(unsigned int)
PARSERS_DEF_FOR_SINGLE(long)
PARSERS_DEF_FOR_SINGLE(unsigned long)
PARSERS_DEF_FOR_SINGLE(long long)
PARSERS_DEF_FOR_SINGLE(unsigned long long)
PARSERS_DEF_FOR_SINGLE(double)
PARSERS_DEF_FOR_SINGLE(float)
PARSERS_DEF_FOR_SINGLE(long double)
PARSERS_DEF_FOR_SINGLE(std::string)

PARSERS_DEF_FOR_LIST(bool)
PARSERS_DEF_FOR_LIST(char)
PARSERS_DEF_FOR_LIST(unsigned char)
PARSERS_DEF_FOR_LIST(signed char)
PARSERS_DEF_FOR_LIST(int)
PARSERS_DEF_FOR_LIST(short)
PARSERS_DEF_FOR_LIST(unsigned short)
PARSERS_DEF_FOR_LIST(unsigned int)
PARSERS_DEF_FOR_LIST(long)
PARSERS_DEF_FOR_LIST(unsigned long)
PARSERS_DEF_FOR_LIST(long long)
PARSERS_DEF_FOR_LIST(unsigned long long)
PARSERS_DEF_FOR_LIST(double)
PARSERS_DEF_FOR_LIST(float)
PARSERS_DEF_FOR_LIST(long double)
PARSERS_DEF_FOR_LIST(std::string)

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


StatusCode Gaudi::Parsers::parse(std::vector<std::vector<double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

/*StatusCode Gaudi::Parsers::parse(std::set<double>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}*/

StatusCode Gaudi::Parsers::parse(std::map<int, int>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<int, double>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::string>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

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

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::vector<int> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<std::string, std::vector<double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<int, std::string>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}

StatusCode Gaudi::Parsers::parse(std::map<unsigned int, std::string>& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}


StatusCode Gaudi::Parsers::parse(std::string& name, std::string& value ,
    const std::string& input ) {
  Skipper skipper;
  KeyValueGrammar<IteratorT, Skipper> g;
  KeyValueGrammar<IteratorT, Skipper>::ResultT result;
  std::string::const_iterator iter = input.begin();
  bool parse_result = qi::phrase_parse(iter, input.end(), g, skipper,
      result) && (iter==input.end());
  if (parse_result) {
    name = result.first;
    value = result.second;
  }
  return parse_result;
}



StatusCode Gaudi::Parsers::parse(std::map<std::string, std::pair<double, double> >& result, const std::string& input) {
    return Gaudi::Parsers::parse_(result, input);
}
// ============================================================================
// The END
// ============================================================================
// ============================================================================

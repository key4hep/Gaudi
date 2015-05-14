#include "GaudiKernel/AttribStringParser.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <boost/test/minimal.hpp>
#include <boost/regex.hpp>

int test_main(int /*argc*/, char** /*argv*/)
{
  using Gaudi::Utils::AttribStringParser;
  using Attrib = Gaudi::Utils::AttribStringParser::Attrib;
  using std::vector;
  using std::map;
  using std::string;
  using std::cout;
  using std::endl;
  typedef map<string, string> toks_t;
  typedef vector<toks_t> res_t;
  {
    res_t results;
    for(string s: {"abc='123'",
      " abc='123'",
      "abc='123' ",
      "abc = '123'",
      "  k1='v1' k2='v2'   k3='${HOME}' " }) {
      cout << "processing \"" << s << "\"" << endl;
      toks_t m;
      for(auto attrib: AttribStringParser(s)) {
        cout << "   \"" << attrib.tag << "\" -> \"" << attrib.value << "\"" << endl;
        m[attrib.tag] = attrib.value;
      }
      results.push_back(std::move(m));
    }
    res_t expected{{{"abc", "123"}},
      {{"abc", "123"}},
      {{"abc", "123"}},
      {{"abc", "123"}}, // <-- this one is a bit special
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", System::getEnv("HOME")}},
    };

    BOOST_CHECK(results.size() == expected.size());
    for(decltype(results.size()) i = 0; i < results.size(); ++i ) {
      BOOST_CHECK(results[i] == expected[i]);
    }
  }
  {
    // check without variable expansion
    Attrib result = *begin(AttribStringParser("var='${HOME}'", false));
    Attrib expected{"var", "${HOME}"};
    BOOST_CHECK(result.tag == expected.tag && result.value == expected.value);
  }


  return 0;
}

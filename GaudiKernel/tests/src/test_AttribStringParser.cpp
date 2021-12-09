/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/AttribStringParser.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/regex.hpp>

#define BOOST_TEST_MODULE AttribStringParser_test
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_case ) {
  using Gaudi::Utils::AttribStringParser;
  using Attrib = Gaudi::Utils::AttribStringParser::Attrib;
  using std::cout;
  using std::endl;
  using std::map;
  using std::string;
  using std::vector;
  typedef map<string, string> toks_t;
  typedef vector<toks_t>      res_t;
  {
    res_t results;
    for ( string s : { "abc='123'", " abc='123'", "abc='123' ", "abc = '123'", "  k1='v1' k2='v2'   k3='${HOME}' " } ) {
      cout << "processing \"" << s << "\"" << endl;
      toks_t m;
      for ( auto attrib : AttribStringParser( s ) ) {
        cout << "   \"" << attrib.tag << "\" -> \"" << attrib.value << "\"" << endl;
        m[attrib.tag] = attrib.value;
      }
      results.push_back( std::move( m ) );
    }
    res_t expected{
        { { "abc", "123" } },
        { { "abc", "123" } },
        { { "abc", "123" } },
        { { "abc", "123" } }, // <-- this one is a bit special
        { { "k1", "v1" }, { "k2", "v2" }, { "k3", System::getEnv( "HOME" ) } },
    };

    BOOST_CHECK( results.size() == expected.size() );
    for ( decltype( results.size() ) i = 0; i < results.size(); ++i ) { BOOST_CHECK( results[i] == expected[i] ); }
  }
  {
    // check without variable expansion
    Attrib result = *begin( AttribStringParser( "var='${HOME}'", false ) );
    Attrib expected{ "var", "${HOME}" };
    BOOST_CHECK( result.tag == expected.tag && result.value == expected.value );
  }
}

// $Id:$
// ===========================================================================
// Include files
// ===========================================================================
#include "Utils.h"
// ===========================================================================
// STD & STL
// ===========================================================================
// ===========================================================================
// Boost
// ===========================================================================
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
// ===========================================================================
// Gaudi
// ===========================================================================
#include "GaudiKernel/System.h"
// ===========================================================================
// Local
// ===========================================================================
// ===========================================================================

namespace gpu=Gaudi::Parsers::Utils;

std::string gpu::replaceEnvironments(const std::string& input) {
  std::string result=input;// result

    const char* re = "\\$(([A-Za-z0-9_]+)|\\(([A-Za-z0-9_]+)\\))";
    std::string::const_iterator start, end;
    boost::regex expression(re);
    start = input.begin();
    end = input.end();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    while ( boost::regex_search(start, end, what, expression, flags ) )
    {
      std::string var,env;
      std::string matched(what[0].first,what[0].second);
      std::string v1(what[2].first,what[2].second);
      std::string v2(what[3].first,what[3].second);

      if ( v1.length()>0){ var = v1; }
      else { var = v2; }

      System::getEnv(var, env);
      if(var != "UNKNOWN") {
        boost::algorithm::replace_first(result,matched, env);
      }
      start = what[0].second;
      // update flags:
      flags |= boost::match_prev_avail;
      flags |= boost::match_not_bob;
    }
    return result;
}


// ===========================================================================


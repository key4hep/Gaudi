// ===========================================================================
// Include files
// ===========================================================================
#include "Utils.h"
// ===========================================================================
// Boost
// ===========================================================================
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
// ===========================================================================
// Gaudi
// ===========================================================================
#include "GaudiKernel/System.h"
// ===========================================================================

namespace gpu = Gaudi::Parsers::Utils;

std::string gpu::replaceEnvironments( const std::string& input )
{
  static const boost::regex expression( "\\$(([A-Za-z0-9_]+)|\\(([A-Za-z0-9_]+)\\))" );

  std::string result = input;
  auto start         = input.begin();
  auto end           = input.end();
  boost::match_results<std::string::const_iterator> what;
  boost::match_flag_type flags = boost::match_default;
  while ( boost::regex_search( start, end, what, expression, flags ) ) {
    std::string var{what[2].first, what[2].second};
    if ( var.empty() ) var = std::string{what[3].first, what[3].second};
    std::string env;
    if ( System::getEnv( var, env ) ) {
      boost::algorithm::replace_first( result, std::string{what[0].first, what[0].second}, env );
    }
    start = what[0].second;
    // update flags:
    flags |= boost::match_prev_avail;
    flags |= boost::match_not_bob;
  }
  return result;
}
// ===========================================================================

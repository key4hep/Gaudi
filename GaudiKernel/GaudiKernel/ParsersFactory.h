// ============================================================================
#ifndef GAUDIPROPERTYPARSERS_PARSERSGENERATOR_H
#define GAUDIPROPERTYPARSERS_PARSERSGENERATOR_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
#include <map>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/GrammarsV2.h"
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ========================================================================
    typedef std::string::const_iterator IteratorT;
    //typedef boost::spirit::ascii::space_type Skipper;
    typedef SkipperGrammar<IteratorT> Skipper;
    // ========================================================================
    template<typename ResultT> inline StatusCode
    parse_(ResultT& result, const std::string& input){
      Skipper skipper;
      typename Grammar_<IteratorT, ResultT, Skipper>::Grammar g;
      IteratorT iter = input.begin(), end = input.end();
      return qi::phrase_parse( iter, end, g, skipper , result) && (iter==end);
    }
    //=========================================================================
    template<> inline StatusCode
    parse_(std::string& result, const std::string& input){
      Skipper skipper;
      Grammar_<IteratorT, std::string, Skipper>::Grammar g;
      IteratorT iter = input.begin(), end = input.end();
      if (!(qi::phrase_parse( iter, end, g, skipper, result) && (iter==end))){
        result = input;
      }
      //@attention always
      return true;
    }
    //=========================================================================
  }/* Parsers */
} /*  Gaudi */
//=============================================================================
#endif // GAUDIPROPERTYPARSERS_PARSERSGENERATOR_H


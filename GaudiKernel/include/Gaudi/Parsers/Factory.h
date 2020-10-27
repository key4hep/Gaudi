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
#pragma once
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <string>
#include <vector>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
// ============================================================================
// Gaudi
// ============================================================================
#include <Gaudi/Parsers/Grammars.h>
#include <GaudiKernel/StatusCode.h>
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ========================================================================
    typedef std::string::const_iterator IteratorT;
    // typedef boost::spirit::ascii::space_type Skipper;
    typedef SkipperGrammar<IteratorT> Skipper;
    // ========================================================================
    template <typename ResultT>
    inline StatusCode parse_( ResultT& result, const std::string& input ) {
      Skipper                                                 skipper;
      typename Grammar_<IteratorT, ResultT, Skipper>::Grammar g;
      IteratorT                                               iter = input.begin(), end = input.end();
      return ( qi::phrase_parse( iter, end, g, skipper, result ) && ( iter == end ) ? StatusCode::SUCCESS
                                                                                    : StatusCode::FAILURE );
    }
    //=========================================================================
    template <>
    inline StatusCode parse_( std::string& result, const std::string& input ) {
      Skipper                                            skipper;
      Grammar_<IteratorT, std::string, Skipper>::Grammar g;
      IteratorT                                          iter = input.begin(), end = input.end();
      if ( !( qi::phrase_parse( iter, end, g, skipper, result ) && ( iter == end ) ) ) { result = input; }
      //@attention always
      return StatusCode::SUCCESS;
    }
    //=========================================================================
    template <typename ResultT>
    inline StatusCode parse( ResultT& result, const std::string& input ) {
      return parse_( result, input );
    }
    //=========================================================================
  } // namespace Parsers
} // namespace Gaudi

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
// Include files

#include "GaudiKernel/HashMap.h"
#include <Gaudi/Parsers/CommonParsers.h>
// Local implementation of parsers from HashMap
namespace Gaudi {
  namespace Parsers {
    /// Basic parser for the types of HashMap used in DODBasicMapper.
    /// Delegates to the parser of map<string,string>.
    template <typename K, typename V>
    StatusCode parse( GaudiUtils::HashMap<K, V>& result, std::string_view input ) {
      std::map<std::string, std::string> tmp;
      StatusCode                         sc = parse( tmp, input );
      if ( sc.isSuccess() ) {
        result.clear();
        result.insert( tmp.begin(), tmp.end() );
      }
      return sc;
    }
  } // namespace Parsers
} // namespace Gaudi

// local
#include "DODBasicMapper.h"

// ----------------------------------------------------------------------------
// Implementation file for class: DODBasicMapper
//
// 16/01/2012: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT( DODBasicMapper )

Gaudi::Utils::TypeNameString DODBasicMapper::algorithmForPath( const std::string& path ) {
  auto alg = m_algMap.find( path );
  return ( alg != m_algMap.end() ) ? alg->second : "";
}

std::string DODBasicMapper::nodeTypeForPath( const std::string& path ) {
  auto node = m_nodeMap.find( path );
  return ( node != m_nodeMap.end() ) ? node->second : "";
}

// ============================================================================

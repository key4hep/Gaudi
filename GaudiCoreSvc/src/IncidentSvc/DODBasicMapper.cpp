/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <Gaudi/Parsers/CommonParsers.h>
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/IDODAlgMapper.h>
#include <GaudiKernel/IDODNodeMapper.h>

/** @class DODBasicMapper DODBasicMapper.h IncidentSvc/DODBasicMapper.h
 *
 * Default implementation of a DataOnDemand mapper.
 *
 * @author Marco Clemencic
 * @date 16/01/2012
 */
class DODBasicMapper : public extends<AlgTool, IDODAlgMapper, IDODNodeMapper> {
public:
  /// inherit contructor
  using extends::extends;

  /// @see IDODAlgMapper
  Gaudi::Utils::TypeNameString algorithmForPath( const std::string& path ) override;

  /// @see IDODNodeMapper
  std::string nodeTypeForPath( const std::string& path ) override;

private:
  template <class T>
  using MapProp = Gaudi::Property<GaudiUtils::HashMap<std::string, T>>;

  MapProp<std::string> m_nodeMap{
      this, "Nodes", {}, "map of the type of nodes to be associated to paths (path -> data_type)." };
  MapProp<Gaudi::Utils::TypeNameString> m_algMap{
      this, "Algorithms", {}, "map of algorithms to be used to produce entries (path -> alg_name)." };
};

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

DECLARE_COMPONENT( DODBasicMapper )

Gaudi::Utils::TypeNameString DODBasicMapper::algorithmForPath( const std::string& path ) {
  auto alg = m_algMap.find( path );
  return ( alg != m_algMap.end() ) ? alg->second : "";
}

std::string DODBasicMapper::nodeTypeForPath( const std::string& path ) {
  auto node = m_nodeMap.find( path );
  return ( node != m_nodeMap.end() ) ? node->second : "";
}

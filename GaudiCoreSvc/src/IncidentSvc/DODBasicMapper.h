/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

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

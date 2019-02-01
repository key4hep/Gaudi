#ifndef INCIDENTSVC_DODBASICMAPPER_H
#define INCIDENTSVC_DODBASICMAPPER_H 1
// Include files
// from Gaudi
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/HashMap.h"

// Implemented interfaces
#include "GaudiKernel/IDODAlgMapper.h"
#include "GaudiKernel/IDODNodeMapper.h"

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
      this, "Nodes", {}, "map of the type of nodes to be associated to paths (path -> data_type)."};
  MapProp<Gaudi::Utils::TypeNameString> m_algMap{
      this, "Algorithms", {}, "map of algorithms to be used to produce entries (path -> alg_name)."};
};

#endif // INCIDENTSVC_DODBASICMAPPER_H

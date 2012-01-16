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
class DODBasicMapper: public extends2<AlgTool, IDODAlgMapper, IDODNodeMapper> {
public:
  /// Standard constructor
  DODBasicMapper(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~DODBasicMapper(); ///< Destructor

  /// @see IDODAlgMapper
  virtual Gaudi::Utils::TypeNameString algorithmForPath(const std::string &path);

  /// @see IDODNodeMapper
  virtual std::string nodeTypeForPath(const std::string &path);

protected:
private:
  typedef GaudiUtils::HashMap<std::string, Gaudi::Utils::TypeNameString>  AlgMap;
  typedef GaudiUtils::HashMap<std::string, std::string>  NodeMap;

  AlgMap m_algMap;
  NodeMap m_nodeMap;
};

#endif // INCIDENTSVC_DODBASICMAPPER_H

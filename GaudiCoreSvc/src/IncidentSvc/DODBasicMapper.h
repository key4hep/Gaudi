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
class DODBasicMapper: public extends<AlgTool,
                                     IDODAlgMapper,
                                     IDODNodeMapper> {
public:
  /// Standard constructor
  DODBasicMapper(const std::string& type, const std::string& name, const IInterface* parent);
  ~DODBasicMapper() override = default; ///< Destructor

  /// @see IDODAlgMapper
  Gaudi::Utils::TypeNameString algorithmForPath(const std::string &path) override;

  /// @see IDODNodeMapper
  std::string nodeTypeForPath(const std::string &path) override;

private:
  GaudiUtils::HashMap<std::string, Gaudi::Utils::TypeNameString>  m_algMap;
  GaudiUtils::HashMap<std::string, std::string>                   m_nodeMap;
};

#endif // INCIDENTSVC_DODBASICMAPPER_H

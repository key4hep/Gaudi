#ifndef GAUDI_DATAINCIDENT_H
#define GAUDI_DATAINCIDENT_H

// Include files
#include "GaudiKernel/Incident.h"

/**
 * @class DataIncident DataIncident.h GaudiKernel/DataIncident.h
 *
 * Data service incident class.
 *
 * @author   Markus Frank
 * @version  1.0
 */
template<class T>
class GAUDI_API ContextIncident: public Incident {
public:
  /// Initializing Constructor
  /** @param source   [IN]  Incident source (service or algorithm name)
    * @param type     [IN]  Incident type
    * @param tag      [IN]  Incident tag
    *
    * @return Reference to ContextIncident object.
    */
  ContextIncident(const std::string& src,const std::string& typ,T tag)
    : Incident(src,typ), m_tag(tag)  {  }

  /// Standard destructor
  virtual ~ContextIncident() { }

  /// Accesssor to the tag value (CONST)
  const T& tag()  const { return m_tag; }

private:
  T m_tag;
};

/// Specific incident type used by the data-on-demand-service
typedef ContextIncident<std::string> DataIncident;

#endif //GAUDI_DATAINCIDENT_H

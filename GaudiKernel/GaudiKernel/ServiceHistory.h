// $Id: ServiceHistory.h,v 1.2 2008/06/04 12:35:15 marcocle Exp $

#ifndef GAUDIKERNEL_SERVICEHISTORY_H
#define GAUDIKERNEL_SERVICEHISTORY_H

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include <string>
#include <vector>

class IService;
class Property;
class JobHistory;

/** @class ServiceHistory ServiceHistory.h
 *
 *  ServiceHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API ServiceHistory: public HistoryObj, public IVersHistoryObj {

public:
  ServiceHistory();
  ServiceHistory(const IService*, const JobHistory* ) ;
  ServiceHistory(const IService&, const JobHistory* ) ;

  virtual ~ServiceHistory();

  // Class IDs
  virtual const CLID& clID() const { return classID(); }
  static const CLID& classID();

  // Return the service properties.
  const PropertyList& properties() const { return m_properties; }

  const IService* service() const { return m_pService; }

  const JobHistory* jobHistory() const { return m_jobHistory; }

  void dump(std::ostream &, const bool isXML=false, int indent=0) const;

  const std::string& name() const { return m_name; }
  const std::string& type() const { return m_type; }
  const std::string& version() const { return m_version; }


private:

  const IService* m_pService;
  const JobHistory* m_jobHistory;
  std::string m_name,m_type,m_version;
  PropertyList m_properties;

};

// Output stream.
GAUDI_API std::ostream& operator<<(std::ostream& lhs, const ServiceHistory& rhs);

#endif

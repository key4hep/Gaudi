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
#ifndef GAUDIKERNEL_SERVICEHISTORY_H
#define GAUDIKERNEL_SERVICEHISTORY_H

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include <string>
#include <vector>

class IService;
class JobHistory;

/** @class ServiceHistory ServiceHistory.h
 *
 *  ServiceHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API ServiceHistory : public HistoryObj, public IVersHistoryObj {

public:
  ServiceHistory();
  ServiceHistory( const IService*, const JobHistory* );
  ServiceHistory( const IService&, const JobHistory* );

  virtual ~ServiceHistory() = default;

  // Class IDs
  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();

  // Return the service properties.
  const PropertyList& properties() const override { return m_properties; }

  const IService* service() const { return m_pService; }

  const JobHistory* jobHistory() const { return m_jobHistory; }

  void dump( std::ostream&, const bool isXML = false, int indent = 0 ) const override;

  const std::string& name() const override { return m_name; }
  const std::string& type() const override { return m_type; }
  const std::string& version() const override { return m_version; }

private:
  const IService*   m_pService;
  const JobHistory* m_jobHistory{nullptr};
  std::string       m_name, m_type, m_version;
  PropertyList      m_properties;
};

// Output stream.
GAUDI_API std::ostream& operator<<( std::ostream& lhs, const ServiceHistory& rhs );

#endif

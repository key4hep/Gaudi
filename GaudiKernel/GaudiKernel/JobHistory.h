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

#ifndef GAUDIKERNEL_JOBHISTORY_H
#define GAUDIKERNEL_JOBHISTORY_H

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"
#include "GaudiKernel/PropertyFwd.h"

#include <ctime>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

/** @class JobHistory JobHistory.h
 *
 *  JobHistory class definition
 *
 *  @author: Charles Leggett
 *
 */
class GAUDI_API JobHistory : public HistoryObj, virtual public IVersHistoryObj {

public:
  typedef std::vector<std::pair<std::string, const Gaudi::Details::PropertyBase*>> PropertyPairList;

private: // data
  std::string m_release_version;
  std::string m_dir;
  std::string m_cmtconfig;

  std::string m_osname;
  std::string m_hostname;
  std::string m_os_version;
  std::string m_machine;

  std::vector<std::string> m_environ;

  PropertyList     m_props;
  PropertyPairList m_ppl;

  std::vector<std::string> m_CVSid;
  time_t                   m_start_time;

public: // functions
  // Constructor.
  JobHistory();
  JobHistory( const std::string& rel, const std::string& os, const std::string& host, const std::string& dir,
              const std::string& osver, const std::string& mach, const std::string& cmtconfig, const time_t& time );

  // Destructor.
  virtual ~JobHistory();

  // Class IDs
  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();

  // add a global property
  void addProperty( const std::string&, const Gaudi::Details::PropertyBase* );

  // Return the job history data.
  std::string              release_version() const { return m_release_version; }
  std::string              os() const { return m_osname; }
  std::string              hostname() const { return m_hostname; }
  std::string              os_version() const { return m_os_version; }
  std::string              machine() const { return m_machine; }
  std::string              dir() const { return m_dir; }
  std::string              cmtconfig() const { return m_cmtconfig; }
  std::vector<std::string> environment() const { return m_environ; }
  const PropertyList&      properties() const override { return m_props; }
  const PropertyPairList&  propertyPairs() const { return m_ppl; }
  time_t                   start_time() const { return m_start_time; }

  void dump( std::ostream&, const bool isXML = false, int indent = 0 ) const override;

  const std::string& name() const override { return m_machine; }
  const std::string& version() const override { return m_release_version; }
  const std::string& type() const override { return m_osname; }

private:
};

// Output stream.
GAUDI_API std::ostream& operator<<( std::ostream& lhs, const JobHistory& rhs );

#endif

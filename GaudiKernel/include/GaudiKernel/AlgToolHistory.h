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
#ifndef GAUDIKERNEL_ALGTOOLHISTORY_H
#define GAUDIKERNEL_ALGTOOLHISTORY_H

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

class AlgTool;
class JobHistory;

#include <memory>
#include <string>
#include <vector>

/** @class AlgToolHistory AlgToolHistory.h
 *
 *  AlgToolHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API AlgToolHistory : public HistoryObj, public IVersHistoryObj {

private: // data
  // Algtool full type.
  std::string m_type;

  // Algtool version.
  std::string m_version;

  // Algtool name.
  std::string m_name;

  // Pointer to the algtool
  const AlgTool* m_tool;

  // Properties.
  PropertyList m_properties;

  // Link to jobHistory
  const JobHistory* m_jobHistory;

public:
  AlgToolHistory( const AlgTool& alg, const JobHistory* job );

  AlgToolHistory( std::string algVersion, std::string algName, std::string algType, const AlgTool* tool,
                  const PropertyList& props, const JobHistory* job );

  // Class IDs
  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();

  // Return the algtool type.
  const std::string& algtool_type() const { return m_type; }

  // Return the algtool version.
  const std::string& algtool_version() const { return m_version; }

  // Return the algtool name.
  const std::string& algtool_name() const { return m_name; }

  // Pointer to the algtool
  const AlgTool* algtool_instance() const { return m_tool; }

  // Return the algorithm properties.
  const PropertyList& properties() const override { return m_properties; }

  // Return the jobHistory
  const JobHistory* jobHistory() const { return m_jobHistory; }

  std::ostream& dump( std::ostream&, bool isXML, int indent ) const override;

  const std::string& name() const override { return algtool_name(); }
  const std::string& type() const override { return algtool_type(); }
  const std::string& version() const override { return algtool_version(); }

  friend std::ostream& operator<<( std::ostream& lhs, const AlgToolHistory& rhs ) { return rhs.dump( lhs, false, 0 ); }
};

#endif

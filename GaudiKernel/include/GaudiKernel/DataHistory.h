/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_DATAHISTORY_H
#define GAUDIKERNEL_DATAHISTORY_H

// History for an event data object.

#include <GaudiKernel/HistoryObj.h>

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/StatusCode.h>
#include <iostream>
#include <string>
#include <vector>

class AlgorithmHistory;

/** @class DataHistory DataHistory.h
 *
 *  DataHistory class definition
 *
 *  @author: Charles Leggett
 *
 */
class GAUDI_API DataHistory : public HistoryObj {

public:
  class DataHistoryOrder final {
  public:
    bool operator()( const DataHistory& lhs, const DataHistory& rhs ) const {
      return std::tie( lhs.m_dataClassID, lhs.m_dataKey, lhs.m_algHist ) <
             std::tie( rhs.m_dataClassID, rhs.m_dataKey, rhs.m_algHist );
    }
    bool operator()( const DataHistory* lhs, const DataHistory* rhs ) const { return ( *this )( *lhs, *rhs ); }
  };

  DataHistory( const CLID& id, std::string key, AlgorithmHistory* alg );

  const CLID&        clID() const override { return DataHistory::classID(); }
  static const CLID& classID();

  std::string dataKey() const { return m_dataKey; }
  const CLID& dataClassID() const { return m_dataClassID; }

  AlgorithmHistory* algorithmHistory() const { return m_algHist; }

  std::ostream&        dump( std::ostream&, const bool isXML, int indent ) const override;
  friend std::ostream& operator<<( std::ostream& lhs, const DataHistory& rhs ) { return rhs.dump( lhs, false, 0 ); }

private:
  CLID              m_dataClassID;
  std::string       m_dataKey;
  AlgorithmHistory* m_algHist;
  std::string       m_dummy = "none";
};

#endif

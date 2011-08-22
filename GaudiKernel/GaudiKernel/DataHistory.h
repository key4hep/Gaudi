// $Id: DataHistory.h,v 1.1 2006/11/09 10:24:05 mato Exp $

#ifndef GAUDIKERNEL_DATAHISTORY_H
#define GAUDIKERNEL_DATAHISTORY_H

// History for an event data object.

#include "GaudiKernel/HistoryObj.h"

#include <vector>
#include <string>
#include <iostream>
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ClassID.h"

class AlgorithmHistory;

/** @class DataHistory DataHistory.h
 *
 *  DataHistory class definition
 *
 *  @author: Charles Leggett
 *
 */
class GAUDI_API DataHistory: public HistoryObj {

public:

  class DataHistoryOrder {
  public:
    bool operator() ( const DataHistory* lhs, const DataHistory* rhs ) const {
      if (lhs->m_dataClassID == rhs->m_dataClassID) {
	if (lhs->m_dataKey == rhs->m_dataKey) {
	  return ( lhs->m_algHist < rhs->m_algHist );
	} else {
	  return ( lhs->m_dataKey < rhs->m_dataKey );
	}
      } else {
	return (lhs->m_dataClassID < rhs->m_dataClassID);
      }

    }
    bool operator() ( const DataHistory& lhs, const DataHistory& rhs ) const {
      if (lhs.m_dataClassID == rhs.m_dataClassID) {
	if (lhs.m_dataKey == rhs.m_dataKey) {
	  return ( lhs.m_algHist < rhs.m_algHist );
	} else {
	  return ( lhs.m_dataKey < rhs.m_dataKey );
	}
      } else {
	return (lhs.m_dataClassID < rhs.m_dataClassID);
      }
    }
  };

  DataHistory(const CLID& id, const std::string& key, AlgorithmHistory* alg);

  ~DataHistory(){};

  virtual const CLID& clID() const { return DataHistory::classID(); }
  static const CLID& classID();


  std::string dataKey() const { return m_dataKey; }
  const CLID& dataClassID() const { return m_dataClassID; }

  AlgorithmHistory* algorithmHistory() const { return m_algHist; }

  void dump(std::ostream &, const bool isXML=false, int indent=0) const;

private:

  CLID m_dataClassID;
  std::string m_dataKey;
  AlgorithmHistory* m_algHist;
  std::string m_dummy;


};

GAUDI_API std::ostream& operator<<(std::ostream& lhs, const DataHistory& rhs);

#endif

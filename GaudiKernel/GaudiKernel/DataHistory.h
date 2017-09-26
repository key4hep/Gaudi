#ifndef GAUDIKERNEL_DATAHISTORY_H
#define GAUDIKERNEL_DATAHISTORY_H

// History for an event data object.

#include "GaudiKernel/HistoryObj.h"

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"
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
class GAUDI_API DataHistory : public HistoryObj
{

public:
  class DataHistoryOrder final
  {
  public:
    bool operator()( const DataHistory* lhs, const DataHistory* rhs ) const
    {
      if ( lhs->m_dataClassID == rhs->m_dataClassID ) {
        if ( lhs->m_dataKey == rhs->m_dataKey ) {
          return ( lhs->m_algHist < rhs->m_algHist );
        } else {
          return ( lhs->m_dataKey < rhs->m_dataKey );
        }
      } else {
        return ( lhs->m_dataClassID < rhs->m_dataClassID );
      }
    }
    bool operator()( const DataHistory& lhs, const DataHistory& rhs ) const
    {
      if ( lhs.m_dataClassID == rhs.m_dataClassID ) {
        if ( lhs.m_dataKey == rhs.m_dataKey ) {
          return ( lhs.m_algHist < rhs.m_algHist );
        } else {
          return ( lhs.m_dataKey < rhs.m_dataKey );
        }
      } else {
        return ( lhs.m_dataClassID < rhs.m_dataClassID );
      }
    }
  };

  DataHistory( const CLID& id, std::string key, AlgorithmHistory* alg );

  ~DataHistory() override = default;

  const CLID& clID() const override { return DataHistory::classID(); }
  static const CLID& classID();

  std::string dataKey() const { return m_dataKey; }
  const CLID& dataClassID() const { return m_dataClassID; }

  AlgorithmHistory* algorithmHistory() const { return m_algHist; }

  void dump( std::ostream&, const bool isXML = false, int indent = 0 ) const override;

private:
  CLID m_dataClassID;
  std::string m_dataKey;
  AlgorithmHistory* m_algHist;
  std::string m_dummy = "none";
};

GAUDI_API std::ostream& operator<<( std::ostream& lhs, const DataHistory& rhs );

#endif

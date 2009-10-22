///////////////////////////////////////////////////////////////////////////
//
// $Id: DataHistory.cpp,v 1.1 2006/11/09 10:24:05 mato Exp $
//
// GaudiHistory/DataHistory.cpp
//
// Contains history information for a Data Object
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/DataHistory.h"
#include "GaudiKernel/AlgorithmHistory.h"
#include <iostream>

using std::ostream;
using std::endl;

//**********************************************************************
// Member functions.
//**********************************************************************

// Default constructor.

DataHistory::DataHistory(const CLID& id, const std::string& key,
			 AlgorithmHistory* alg):
  m_dataClassID(id), m_dataKey(key), m_algHist(alg)
{


}

//**********************************************************************

const CLID&
DataHistory::classID() {
  static CLID CLID_DataHistory = 83814411;   // from `clid DataHistory`
  
  return CLID_DataHistory;
}
  

  
ostream& operator<<(ostream& lhs, const DataHistory& rhs) {

  lhs << "ClassID: " << rhs.dataClassID() << endl
      << "Key: " << rhs.dataKey() << endl
      << "AlgorithHistory: " << (void*) rhs.algorithmHistory() << endl;

  return lhs;

}

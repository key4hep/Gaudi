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
///////////////////////////////////////////////////////////////////////////
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

using std::endl;
using std::ostream;

//**********************************************************************
// Member functions.
//**********************************************************************

// Default constructor.

DataHistory::DataHistory( const CLID& id, std::string key, AlgorithmHistory* alg )
    : m_dataClassID( id ), m_dataKey( std::move( key ) ), m_algHist( alg ) {}

//**********************************************************************

const CLID& DataHistory::classID() {
  static const CLID CLID_DataHistory = 83814411; // from `clid DataHistory`

  return CLID_DataHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DataHistory::dump( std::ostream& ost, const bool /*isXML*/, int /*ind*/ ) const {
  ost << "ClassID: " << dataClassID() << endl
      << "Key: " << dataKey() << endl
      << "AlgorithmHistory: " << (void*)algorithmHistory() << endl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ostream& operator<<( ostream& lhs, const DataHistory& rhs ) {

  rhs.dump( lhs, false );
  return lhs;
}

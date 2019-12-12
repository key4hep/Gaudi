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
// Include files
#include <iostream>

// local
#include "CallgrindProfile.h"
#include "local_callgrind.h"

//-----------------------------------------------------------------------------
// Implementation file for class : CallgrindProfile
//
// 2014-08-22 : Ben Couturier
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( CallgrindProfile )

//=============================================================================
// Main execution
//=============================================================================
StatusCode CallgrindProfile::execute() {

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  // Increase event number
  ++m_eventNumber;

  if ( m_eventNumber == m_nStartFromEvent ) {
    m_profiling = true;
    warning() << "Starting Callgrind profile at event " << m_eventNumber << endmsg;
    CALLGRIND_START_INSTRUMENTATION;
  }

  if ( m_eventNumber == m_nZeroAtEvent ) {
    warning() << "Setting Callgrind counters to zero at event " << m_eventNumber << endmsg;
    CALLGRIND_ZERO_STATS;
  }

  if ( m_eventNumber == m_nStopAtEvent ) {
    m_profiling = false;
    warning() << "Stopping Callgrind profile at event " << m_eventNumber << endmsg;
    CALLGRIND_STOP_INSTRUMENTATION;
  }

  if ( m_eventNumber == m_nDumpAtEvent ) {
    warning() << "Dumping Callgrind counters to zero at event " << m_eventNumber << endmsg;

    if ( m_dumpName == "" ) {
      CALLGRIND_DUMP_STATS;
    } else {
      CALLGRIND_DUMP_STATS_AT( m_dumpName.value().c_str() );
    }
    m_dumpDone = true;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode CallgrindProfile::finalize() {

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  if ( !m_dumpDone ) {
    if ( m_dumpName == "" ) {
      CALLGRIND_DUMP_STATS;
    } else {
      CALLGRIND_DUMP_STATS_AT( m_dumpName.value().c_str() );
    }
  }

  return Algorithm::finalize(); // must be called after all other actions
}

//=============================================================================

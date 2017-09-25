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
DECLARE_ALGORITHM_FACTORY( CallgrindProfile )

//=============================================================================
// Initialization
//=============================================================================
StatusCode CallgrindProfile::initialize()
{
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode CallgrindProfile::execute()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  // Increase event number
  m_eventNumber += 1;

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
StatusCode CallgrindProfile::finalize()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  if ( !m_dumpDone ) {
    if ( m_dumpName == "" ) {
      CALLGRIND_DUMP_STATS;
    } else {
      CALLGRIND_DUMP_STATS_AT( m_dumpName.value().c_str() );
    }
  }

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

//=============================================================================

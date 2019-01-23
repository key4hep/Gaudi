// Include files

// local
#include "JemallocProfile.h"

// including jemmalloc.h is difficult as the malloc signature is not exactly identical
// to the system one (issue with throw).
// We therefore declare mallctl here.

extern "C" {
int mallctl( const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen );
}

//-----------------------------------------------------------------------------
// Implementation file for class : JemallocProfile
//
// 2015-06-09 : Ben Couturier
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( JemallocProfile )

//=============================================================================
// Initialization
//=============================================================================
StatusCode JemallocProfile::initialize()
{
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;
  bool active = true;
  int  res    = mallctl( "prof.active", NULL, NULL, &active, sizeof( active ) );
  if ( res != 0 ) {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode JemallocProfile::execute()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  // Increase event number
  m_eventNumber += 1;

  if ( m_eventNumber == m_nStartFromEvent ) {
    m_profiling = true;
    info() << "Starting Jemalloc profile at event " << m_eventNumber << endmsg;
  }

  if ( m_profiling && ( ( m_eventNumber - m_nStartFromEvent ) % m_dumpPeriod == 0 ) ) {
    info() << "Jemalloc Dumping heap at event " << m_eventNumber << endmsg;
    mallctl( "prof.dump", NULL, NULL, NULL, 0 );
  }

  if ( m_eventNumber == m_nStopAtEvent ) {
    m_profiling = false;
    info() << "Stopping Jemalloc profile at event " << m_eventNumber << endmsg;
    mallctl( "prof.dump", NULL, NULL, NULL, 0 );
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode JemallocProfile::finalize()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;
  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

//=============================================================================

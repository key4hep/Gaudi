// Include files
#include <iostream>

// local
#include "IntelProfile.h"

//-----------------------------------------------------------------------------
// Implementation file for class : IntelProfile
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( IntelProfile )

//=============================================================================
// Main execution
//=============================================================================
StatusCode IntelProfile::execute()
{

  // Increase event number
  m_eventNumber += 1;

  if ( m_eventNumber == m_nStartFromEvent ) {
    warning() << "Starting Intel profile at event " << m_eventNumber << endmsg;
    __itt_resume();
  }

  if ( m_eventNumber == m_nStopAtEvent ) {
    warning() << "Stopping Intel profile at event " << m_eventNumber << endmsg;
    __itt_pause();
  }

  return StatusCode::SUCCESS;
}

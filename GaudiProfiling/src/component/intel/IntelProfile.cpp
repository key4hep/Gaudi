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
#include "IntelProfile.h"

//-----------------------------------------------------------------------------
// Implementation file for class : IntelProfile
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( IntelProfile )

//=============================================================================
// Main execution
//=============================================================================
StatusCode IntelProfile::execute() {

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

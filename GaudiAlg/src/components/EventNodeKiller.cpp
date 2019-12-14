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
// local
#include "EventNodeKiller.h"

//-----------------------------------------------------------------------------
// Implementation file for class : EventNodeKiller
//
// 2005-07-14 : Marco Cattaneo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( EventNodeKiller )

//=============================================================================
// Main execution
//=============================================================================
StatusCode EventNodeKiller::execute() {

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  for ( auto& node : m_nodes ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Killing node " << node << endmsg;
    eventSvc()->unlinkObject( node ).ignore();
  }

  return StatusCode::SUCCESS;
}
//=============================================================================

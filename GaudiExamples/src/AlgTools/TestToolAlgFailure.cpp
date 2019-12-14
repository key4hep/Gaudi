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

// from Gaudi
#include "GaudiKernel/MsgStream.h"

// local
#include "TestToolAlgFailure.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( TestToolAlgFailure )

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlgFailure::initialize() {

  debug() << "Initialize" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    info() << "Loading tool " << i << endmsg;
    try {
      /* mytool = */ tool<IAlgTool>( i );
      info() << "Tool loaded successfully" << endmsg;
    } catch ( GaudiException& e ) {
      if ( m_ignoreFailure ) {
        warning() << "Got exception '" << e.what() << "'" << endmsg;
      } else {
        error() << "Got exception '" << e.what() << "'" << endmsg;
        if ( m_throwException ) { throw; }
        return StatusCode::FAILURE;
      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlgFailure::execute() {

  info() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlgFailure::finalize() {

  info() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

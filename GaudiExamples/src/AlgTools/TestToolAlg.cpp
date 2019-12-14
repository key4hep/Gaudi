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
#include "TestToolAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( TestToolAlg )

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlg::initialize() {

  debug() << "Initialise" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    std::string name  = i;
    std::string type  = i;
    const int   slash = i.find_first_of( "/" );
    if ( slash > 0 ) {
      type = i.substr( 0, slash );
      name = i.substr( slash + 1 );
    }
    debug() << "Loading tool " << name << " of type " << type << endmsg;
    /*mytool = */ tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlg::execute() {

  debug() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlg::finalize() {

  debug() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

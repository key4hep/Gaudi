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
#include "TestTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TestTool
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_COMPONENT( TestTool )

//=============================================================================

StatusCode TestTool::initialize() {
  debug() << "Initialize" << endmsg;

  if ( !GaudiTool::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    std::string name  = i;
    std::string type  = i;
    auto        slash = i.find_first_of( "/" );
    if ( slash != std::string::npos ) {
      type = i.substr( 0, slash );
      name = i.substr( slash + 1 );
    }
    debug() << "Loading tool " << name << " of type " << type << endmsg;
    /* mytool = */ tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

StatusCode TestTool::finalize() {
  debug() << "Finalize" << endmsg;
  return GaudiTool::finalize();
}

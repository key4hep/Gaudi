/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include "SubAlg.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/MsgStream.h>

// Static Factory declaration

DECLARE_COMPONENT( SubAlg )

// Constructor
//------------------------------------------------------------------------------
SubAlg::SubAlg( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode SubAlg::initialize() {
  //------------------------------------------------------------------------------
  info() << "initializing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode SubAlg::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode SubAlg::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

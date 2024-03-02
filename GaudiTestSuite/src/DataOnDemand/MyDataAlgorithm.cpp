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
#include "MyDataAlgorithm.h"
#include "GaudiKernel/DataObject.h"

// Static Factory declaration

DECLARE_COMPONENT( MyDataAlgorithm )

// Constructor
//------------------------------------------------------------------------------
MyDataAlgorithm::MyDataAlgorithm( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::initialize() {
  //------------------------------------------------------------------------------

  info() << "initializing...." << endmsg;
  info() << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  DataObject* tmp = nullptr;
  eventSvc()->retrieveObject( "Rec/Muon/Digits", tmp ).ignore();
  eventSvc()->retrieveObject( "Rec/Muon/Foos", tmp ).ignore();
  info() << "....execution done" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

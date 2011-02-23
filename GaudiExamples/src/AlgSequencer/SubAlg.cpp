// $Id: SubAlg.cpp,v 1.3 2006/11/30 10:35:26 mato Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "SubAlg.h"

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(SubAlg)

// Constructor
//------------------------------------------------------------------------------
SubAlg::SubAlg(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode SubAlg::initialize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "initializing...." << endmsg;
//  return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode SubAlg::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode SubAlg::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

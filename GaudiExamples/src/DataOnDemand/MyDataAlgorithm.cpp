// Include files
#include "GaudiKernel/DataObject.h"
#include "MyDataAlgorithm.h"

// Static Factory declaration

DECLARE_COMPONENT(MyDataAlgorithm)

// Constructor
//------------------------------------------------------------------------------
MyDataAlgorithm::MyDataAlgorithm(const std::string& name, ISvcLocator* ploc)
           : GaudiAlgorithm(name, ploc) {
//------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::initialize() {
//------------------------------------------------------------------------------

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "initializing...." << endmsg;



  log << MSG::INFO << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;
  get<DataObject>("Rec/Muon/Digits");
  get<DataObject>("Rec/Muon/Foos");
  log << MSG::INFO << "....execution done" << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;


  return StatusCode::SUCCESS;
}

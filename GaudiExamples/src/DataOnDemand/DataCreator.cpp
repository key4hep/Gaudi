#include "GaudiKernel/DataObject.h"
#include "DataCreator.h"

// Static Factory declaration
DECLARE_COMPONENT(DataCreator)

// Constructor
//------------------------------------------------------------------------------
DataCreator::DataCreator(const std::string& name, ISvcLocator* ploc)
           : GaudiAlgorithm(name, ploc) {
//------------------------------------------------------------------------------
  declareProperty("Data",  m_data = "/Event/Unknown" );
}

//------------------------------------------------------------------------------
StatusCode DataCreator::initialize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "initializing...." << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode DataCreator::execute() {
//------------------------------------------------------------------------------
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;
  put(new DataObject(), m_data);
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode DataCreator::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}


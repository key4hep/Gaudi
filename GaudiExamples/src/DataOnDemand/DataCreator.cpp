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
  info() << "initializing...." << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode DataCreator::execute() {
//------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  put(new DataObject(), m_data);
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode DataCreator::finalize() {
//------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}


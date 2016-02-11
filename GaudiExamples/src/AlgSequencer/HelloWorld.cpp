// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "HelloWorld.h"

// Static Factory declaration

DECLARE_COMPONENT(HelloWorld)

// Constructor
//------------------------------------------------------------------------------
HelloWorld::HelloWorld(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
  m_initialized = false;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::initialize() {
//------------------------------------------------------------------------------
  // avoid calling initialize more than once
  if( m_initialized ) return StatusCode::SUCCESS;

  info() << "initializing...." << endmsg;

  IntegerProperty m_int;
  DoubleProperty  m_double;
  StringProperty  m_string;

  declareInfo("int_value",t_int, "description");
  declareInfo("double_value",t_double, "description");
  declareInfo("string_value",t_string, "description");

  m_initialized = true;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode HelloWorld::execute() {
//------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode HelloWorld::finalize() {
//------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  m_initialized = false;
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode HelloWorld::beginRun() {
//------------------------------------------------------------------------------
  info() << "beginning new run...." << endmsg;

  m_initialized = true;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode HelloWorld::endRun() {
//------------------------------------------------------------------------------
  info() << "ending new run...." << endmsg;

  m_initialized = true;
  return StatusCode::SUCCESS;
}

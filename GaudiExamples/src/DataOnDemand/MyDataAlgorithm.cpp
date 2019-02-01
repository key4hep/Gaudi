// Include files
#include "MyDataAlgorithm.h"
#include "GaudiKernel/DataObject.h"

// Static Factory declaration

DECLARE_COMPONENT( MyDataAlgorithm )

// Constructor
//------------------------------------------------------------------------------
MyDataAlgorithm::MyDataAlgorithm( const std::string& name, ISvcLocator* ploc ) : GaudiAlgorithm( name, ploc ) {
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
  get<DataObject>( "Rec/Muon/Digits" );
  get<DataObject>( "Rec/Muon/Foos" );
  info() << "....execution done" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyDataAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

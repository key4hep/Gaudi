// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "ParentAlg.h"

// Static Factory declaration

DECLARE_COMPONENT(ParentAlg)

// Constructor
//------------------------------------------------------------------------------
ParentAlg::ParentAlg(const std::string& name, ISvcLocator* ploc)
          : GaudiAlgorithm(name, ploc) {
//------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode ParentAlg::initialize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  StatusCode sc;

  log << MSG::INFO << "creating sub-algorithms...." << endmsg;

  sc =  createSubAlgorithm( "SubAlg", "SubAlg1", m_subalg1);
  if( sc.isFailure() ) return Error("Error creating Sub-Algorithm SubAlg1",sc);

  sc =  createSubAlgorithm( "SubAlg", "SubAlg2", m_subalg2);
  if( sc.isFailure() ) return Error("Error creating Sub-Algorithm SubAlg2",sc);

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ParentAlg::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  StatusCode sc;
  log << MSG::INFO << "executing...." << endmsg;

  std::vector<Algorithm*>::const_iterator it  = subAlgorithms()->begin();
  std::vector<Algorithm*>::const_iterator end = subAlgorithms()->end();
  for ( ; it != end; it++) {
    sc = (*it)->execute();
    if( sc.isFailure() ) {
      log << "Error executing Sub-Algorithm" << (*it)->name() << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode ParentAlg::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

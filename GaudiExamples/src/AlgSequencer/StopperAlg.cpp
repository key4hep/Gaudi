// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"

#include "StopperAlg.h"

// Static Factory declaration

DECLARE_COMPONENT(StopperAlg)

// Constructor
//------------------------------------------------------------------------------
StopperAlg::StopperAlg(const std::string& name, ISvcLocator* ploc)
          : GaudiAlgorithm(name, ploc) {
//------------------------------------------------------------------------------
  declareProperty("StopCount", m_stopcount);
}

//------------------------------------------------------------------------------
StatusCode StopperAlg::initialize() {
//------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode StopperAlg::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  static int count = 0;

  if ( ++count >= m_stopcount ) {
    log << MSG::INFO << "scheduling a event processing stop...." << endmsg;
    auto evt = service<IEventProcessor>("ApplicationMgr");
    if (evt->stopRun().isFailure()) {
      log << MSG::ERROR << "unable to schedule a stopRun" << endmsg;
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode StopperAlg::finalize() {
//------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

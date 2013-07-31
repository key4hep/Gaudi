// $Id: StopperAlg.cpp,v 1.4 2006/11/30 10:35:26 mato Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"

#include "StopperAlg.h"

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(StopperAlg)

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
    IEventProcessor* evt = svc<IEventProcessor>("ApplicationMgr");
    if (evt->stopRun().isFailure()) {
      log << MSG::ERROR << "unable to issue a stopRun to the EventProcessor"
	  << endmsg;
      return StatusCode::FAILURE;
    }
    evt->release();
  }

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode StopperAlg::finalize() {
//------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

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
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/MsgStream.h"

#include "StopperAlg.h"

// Static Factory declaration

DECLARE_COMPONENT( StopperAlg )

//------------------------------------------------------------------------------
StatusCode StopperAlg::initialize() {
  //------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode StopperAlg::execute() {
  //------------------------------------------------------------------------------
  static int count = 0;

  if ( ++count >= m_stopcount ) {
    info() << "scheduling a event processing stop...." << endmsg;
    auto evt = service<IEventProcessor>( "ApplicationMgr" );
    if ( evt->stopRun().isFailure() ) {
      error() << "unable to schedule a stopRun" << endmsg;
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

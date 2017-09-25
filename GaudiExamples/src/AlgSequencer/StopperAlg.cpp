// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/MsgStream.h"

#include "StopperAlg.h"

// Static Factory declaration

DECLARE_COMPONENT( StopperAlg )

//------------------------------------------------------------------------------
StatusCode StopperAlg::initialize()
{
  //------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode StopperAlg::execute()
{
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
StatusCode StopperAlg::finalize()
{
  //------------------------------------------------------------------------------
  return StatusCode::SUCCESS;
}

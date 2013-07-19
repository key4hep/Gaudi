// Include files
#include "LoopAlg.h"

#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT(LoopAlg)

//------------------------------------------------------------------------------
LoopAlg::LoopAlg(const std::string& name,
				 ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator)
//------------------------------------------------------------------------------
{
}


//------------------------------------------------------------------------------
StatusCode LoopAlg::initialize()
//------------------------------------------------------------------------------
{
  IService *pService(0);
  const bool CREATENOW(true);
  return service("ServiceB", pService, CREATENOW);
}


//------------------------------------------------------------------------------
StatusCode LoopAlg::execute()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Execute..." << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode LoopAlg::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}

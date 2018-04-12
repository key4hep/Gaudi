// Include files
#include "LoopAlg.h"

#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT( LoopAlg )

//------------------------------------------------------------------------------
LoopAlg::LoopAlg( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator )
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
StatusCode LoopAlg::initialize()
//------------------------------------------------------------------------------
{
  IService*  pService = nullptr;
  const bool CREATENOW( true );
  return service( "ServiceB", pService, CREATENOW );
}

//------------------------------------------------------------------------------
StatusCode LoopAlg::execute()
//------------------------------------------------------------------------------
{
  info() << "Execute..." << endmsg;
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode LoopAlg::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}

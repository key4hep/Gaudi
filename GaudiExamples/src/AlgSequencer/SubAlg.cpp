// Include files
#include "SubAlg.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/MsgStream.h"

// Static Factory declaration

DECLARE_COMPONENT( SubAlg )

// Constructor
//------------------------------------------------------------------------------
SubAlg::SubAlg( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc )
{
  //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode SubAlg::initialize()
{
  //------------------------------------------------------------------------------
  info() << "initializing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode SubAlg::execute()
{
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode SubAlg::finalize()
{
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

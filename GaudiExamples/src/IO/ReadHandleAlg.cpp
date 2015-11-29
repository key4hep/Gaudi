// Framework include files
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/MsgStream.h"

// Example related include files
#include "ReadHandleAlg.h"

using namespace Gaudi::Examples;

DECLARE_ALGORITHM_FACTORY(ReadHandleAlg)

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadHandleAlg::execute() {
  
  Collision* c = m_inputHandle.get();

  const int evtNum = ( getContext() ? getContext()->evt() : -1 );

  info() << "Event " << evtNum << " Collision number " <<  c->collision() << endmsg;
  
  return StatusCode::SUCCESS;
}

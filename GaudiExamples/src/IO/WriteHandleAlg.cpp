// Framework include files
#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "WriteHandleAlg.h"

DECLARE_ALGORITHM_FACTORY( WriteHandleAlg )

StatusCode WriteHandleAlg::execute() // the execution of the algorithm
{
  info() << "Hello, I am executing" << endmsg;

  // Set collision to the current event number from the context;
  // if the context doesn't exist, set it to some dummy value
  // this fallback allows to stay compatible with non-hive infrastructure
  Collision* c = new Collision( Gaudi::Hive::currentContext().evt() );

  if ( m_useHandle )
    m_output_handle.put( c );
  else
    eventSvc()->registerObject( "/Event", "MyCollision", c );

  return StatusCode::SUCCESS;
}

// Include files
#include "ParentAlg.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( ParentAlg )

StatusCode ParentAlg::initialize() {
  info() << "creating sub-algorithms...." << endmsg;

  auto sc = createSubAlgorithm( "SubAlg", "SubAlg1", m_subalg1 );
  if ( !sc ) {
    error() << "Error creating Sub-Algorithm SubAlg1" << endmsg;
    return sc;
  }

  sc = createSubAlgorithm( "SubAlg", "SubAlg2", m_subalg2 );
  if ( !sc ) {
    error() << "Error creating Sub-Algorithm SubAlg2" << endmsg;
    return sc;
  }

  // the base class must be invoked _after_ the subalgorithms are created
  return Sequence::initialize();
}

StatusCode ParentAlg::execute( const EventContext& ctx ) const {
  info() << "executing...." << endmsg;

  for ( auto alg : ( *subAlgorithms() ) ) {
    if ( !alg->sysExecute( ctx ) ) { error() << "Error executing Sub-Algorithm" << alg->name() << endmsg; }
  }
  return StatusCode::SUCCESS;
}

StatusCode ParentAlg::finalize() {
  info() << "finalizing...." << endmsg;
  return Sequence::finalize();
}

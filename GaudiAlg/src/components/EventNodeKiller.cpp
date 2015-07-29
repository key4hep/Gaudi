// Include files
// local
#include "EventNodeKiller.h"

//-----------------------------------------------------------------------------
// Implementation file for class : EventNodeKiller
//
// 2005-07-14 : Marco Cattaneo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( EventNodeKiller )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
EventNodeKiller::EventNodeKiller( const std::string& name,
                                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  declareProperty( "Nodes", m_nodes );
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode EventNodeKiller::execute() {

  if (msgLevel() <= MSG::DEBUG) debug() << "==> Execute" << endmsg;

  for( auto& node : m_nodes ) {
    if (msgLevel() <= MSG::DEBUG) debug() << "Killing node " << node << endmsg;
    eventSvc()->unlinkObject( node ).ignore();
  }

  return StatusCode::SUCCESS;
}
//=============================================================================

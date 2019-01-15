// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"

// local
#include "TestToolAlgFailure.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( TestToolAlgFailure )

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlgFailure::initialize()
{

  debug() << "Initialize" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    info() << "Loading tool " << i << endmsg;
    try {
      /* mytool = */ tool<IAlgTool>( i );
      info() << "Tool loaded successfully" << endmsg;
    } catch ( GaudiException& e ) {
      if ( m_ignoreFailure ) {
        warning() << "Got exception '" << e.what() << "'" << endmsg;
      } else {
        error() << "Got exception '" << e.what() << "'" << endmsg;
        if ( m_throwException ) {
          throw;
        }
        return StatusCode::FAILURE;
      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlgFailure::execute()
{

  info() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlgFailure::finalize()
{

  info() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

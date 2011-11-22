// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"

// local
#include "TestToolAlgFailure.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY(TestToolAlgFailure)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TestToolAlgFailure::TestToolAlgFailure( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ) {

  declareProperty( "Tools", m_tools );
  declareProperty( "IgnoreFailure", m_ignoreFailure = false);

}

//=============================================================================
// Destructor
//=============================================================================
TestToolAlgFailure::~TestToolAlgFailure() {}

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlgFailure::initialize() {

  debug() << "Initialize" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  //IAlgTool * mytool;
  for ( ToolList::iterator it = m_tools.begin();
        it != m_tools.end(); ++it ) {
    info() << "Loading tool " << *it << endmsg;
    try {
      /* mytool = */ tool<IAlgTool>( *it );
      info() << "Tool loaded successfully" << endmsg;
    } catch (GaudiException &e) {
      if ( m_ignoreFailure ) {
        warning() << "Got exception '" << e.what() << "'" << endmsg;
      } else {
        error() << "Got exception '" << e.what() << "'" << endmsg;
        return StatusCode::FAILURE;
      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlgFailure::execute() {

  info() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlgFailure::finalize() {

  info() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

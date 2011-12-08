// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"

// local
#include "TestToolAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY(TestToolAlg)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TestToolAlg::TestToolAlg( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ) {

  declareProperty( "Tools", m_tools );

}

//=============================================================================
// Destructor
//=============================================================================
TestToolAlg::~TestToolAlg() {}

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlg::initialize() {

  debug() << "Initialise" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  //IAlgTool * mytool;
  for ( ToolList::iterator it = m_tools.begin();
        it != m_tools.end(); ++it ) {
    std::string name = (*it);
    std::string type = (*it);
    const int slash = (*it).find_first_of( "/" );
    if ( slash > 0 ) {
      type = (*it).substr( 0, slash );
      name = (*it).substr( slash+1 );
    }
    debug() << "Loading tool " << name << " of type " << type << endmsg;
    /*mytool = */tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlg::execute() {

  debug() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlg::finalize() {

  debug() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

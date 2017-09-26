// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"

// local
#include "TestToolAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichToolTest
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( TestToolAlg )

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode TestToolAlg::initialize()
{

  debug() << "Initialise" << endmsg;

  if ( !GaudiAlgorithm::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    std::string name = i;
    std::string type = i;
    const int slash  = i.find_first_of( "/" );
    if ( slash > 0 ) {
      type = i.substr( 0, slash );
      name = i.substr( slash + 1 );
    }
    debug() << "Loading tool " << name << " of type " << type << endmsg;
    /*mytool = */ tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TestToolAlg::execute()
{

  debug() << "Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TestToolAlg::finalize()
{

  debug() << "Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}

//=============================================================================

// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"

// local
#include "TestTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TestTool
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_COMPONENT( TestTool )

//=============================================================================

StatusCode TestTool::initialize()
{
  debug() << "Initialize" << endmsg;

  if ( !GaudiTool::initialize() ) return StatusCode::FAILURE;

  // setup tool registry
  // IAlgTool * mytool;
  for ( const auto& i : m_tools ) {
    std::string name  = i;
    std::string type  = i;
    auto        slash = i.find_first_of( "/" );
    if ( slash != std::string::npos ) {
      type = i.substr( 0, slash );
      name = i.substr( slash + 1 );
    }
    debug() << "Loading tool " << name << " of type " << type << endmsg;
    /* mytool = */ tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

StatusCode TestTool::finalize()
{
  debug() << "Finalize" << endmsg;
  return GaudiTool::finalize();
}

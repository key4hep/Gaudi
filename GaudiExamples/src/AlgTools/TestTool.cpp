// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"

// local
#include "TestTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TestTool
//
// 2004-03-08 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_COMPONENT(TestTool)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TestTool::TestTool( const std::string& type,
                    const std::string& name,
                    const IInterface* parent )
  : base_class ( type, name , parent ) {

  declareProperty( "Tools", m_tools );

}

//=============================================================================

StatusCode TestTool::initialize()
{
  debug() << "Initialize" << endmsg;

  if ( !GaudiTool::initialize() ) return StatusCode::FAILURE;

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
    /* mytool = */ tool<IAlgTool>( type, name );
  }

  return StatusCode::SUCCESS;
}

StatusCode TestTool::finalize()
{
  debug() << "Finalize" << endmsg;
  return GaudiTool::finalize();
}

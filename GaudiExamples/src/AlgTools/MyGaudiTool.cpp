// Framework include files
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyGaudiTool.h"

// Declaration of the AlgTool Factory
DECLARE_COMPONENT( MyGaudiTool )

//------------------------------------------------------------------------------
const std::string& MyGaudiTool::message() const
//------------------------------------------------------------------------------
{
  static const std::string msg( "It works!!!" );
  return msg;
}

//------------------------------------------------------------------------------
void MyGaudiTool::doIt() const
//------------------------------------------------------------------------------
{
  info() << "doIt() has been called" << endmsg;
  debug() << "doIt() [DEBUG] has been called" << endmsg;
  // show the feature introduced with GAUDI-1078
  if ( contextSvc() && name().substr( 0, 7 ) == "ToolSvc" ) Info( "public tool called by" );
}

//------------------------------------------------------------------------------
void MyGaudiTool::doItAgain()
//------------------------------------------------------------------------------
{
  info() << "doItAgain() has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode MyGaudiTool::initialize()
//------------------------------------------------------------------------------
{
  StatusCode sc = base_class::initialize();

  info() << "intialize() has been called" << endmsg;

  // Make use of tool<>

  info() << "Int    = " << m_int.value() << endmsg;
  info() << "Double = " << m_double.value() << endmsg;
  info() << "String = " << m_string.value() << endmsg;
  info() << "Bool   = " << m_bool.value() << endmsg;

  return sc;
}
//------------------------------------------------------------------------------
StatusCode MyGaudiTool::finalize()
//------------------------------------------------------------------------------
{
  info() << "finalize() has been called" << endmsg;
  return base_class::finalize();
}

//------------------------------------------------------------------------------
MyGaudiTool::~MyGaudiTool()
//------------------------------------------------------------------------------
{
  // do not print messages if we are created in genconf
  const std::string cmd = System::cmdLineArgs()[0];
  if ( cmd.find( "genconf" ) != std::string::npos ) return;

  info() << "destructor has been called" << endmsg;
}

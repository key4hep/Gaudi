// Framework include files
#include "GaudiKernel/GaudiException.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyTool.h"

// Declaration of the AlgTool Factory
DECLARE_COMPONENT( MyTool )

//------------------------------------------------------------------------------
const std::string& MyTool::message() const
//------------------------------------------------------------------------------
{
  static std::string msg( "It works!!!" );
  return msg;
}

//------------------------------------------------------------------------------
void MyTool::doIt() const
//------------------------------------------------------------------------------
{
  info() << "doIt() has been called" << endmsg;
  debug() << "doIt() [DEBUG] has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode MyTool::initialize()
//------------------------------------------------------------------------------
{
  info() << "intialize() has been called" << endmsg;

  info() << "Int    = " << m_int.value() << endmsg;
  info() << "Double = " << m_double.value() << endmsg;
  info() << "String = " << m_string.value() << endmsg;
  info() << "Bool   = " << m_bool.value() << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode MyTool::finalize()
//------------------------------------------------------------------------------
{
  info() << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyTool::~MyTool()
//------------------------------------------------------------------------------
{
  // to not print messages if we are created in genconf
  const std::string cmd = System::cmdLineArgs()[0];
  if ( cmd.find( "genconf" ) != std::string::npos ) return;

  info() << "destructor has been called" << endmsg;
}

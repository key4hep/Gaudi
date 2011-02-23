// $Id: MyGaudiTool.cpp,v 1.4 2008/10/10 15:18:56 marcocle Exp $
// Framework include files
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyGaudiTool.h"

// Declaration of the AlgTool Factory
DECLARE_TOOL_FACTORY(MyGaudiTool)

//------------------------------------------------------------------------------
MyGaudiTool::MyGaudiTool( const std::string& type,
                          const std::string& name,
                          const IInterface* parent )
  //------------------------------------------------------------------------------
  : base_class( type, name, parent ) {

  // declare properties
  declareProperty( "Int",    m_int    = 100);
  declareProperty( "Double", m_double = 100.);
  declareProperty( "String", m_string = "hundred");
  declareProperty( "Bool",   m_bool   = true);
}

//------------------------------------------------------------------------------
const std::string&  MyGaudiTool::message() const
  //------------------------------------------------------------------------------
{
  static std::string msg("It works!!!");
  return msg;
}

//------------------------------------------------------------------------------
void  MyGaudiTool::doIt()
  //------------------------------------------------------------------------------
{
  info()  << "doIt() has been called" << endmsg;
  debug() << "doIt() [DEBUG] has been called" << endmsg;
}

//------------------------------------------------------------------------------
void MyGaudiTool::doItAgain()
//------------------------------------------------------------------------------
{
  info()  << "doItAgain() has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  MyGaudiTool::initialize()
  //------------------------------------------------------------------------------
{
  info() << "intialize() has been called" << endmsg;

  // Make use of tool<>

  info() << "Int    = " << m_int    << endmsg;
  info() << "Double = " << m_double << endmsg;
  info() << "String = " << m_string << endmsg;
  info() << "Bool   = " << m_bool   << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  MyGaudiTool::finalize()
  //------------------------------------------------------------------------------
{
  info() << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyGaudiTool::~MyGaudiTool( )
  //------------------------------------------------------------------------------
{
  info() << "destructor has been called" << endmsg;
}



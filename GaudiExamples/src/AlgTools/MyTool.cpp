// Framework include files
#include "GaudiKernel/GaudiException.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyTool.h"

// Declaration of the AlgTool Factory
DECLARE_COMPONENT(MyTool)

//------------------------------------------------------------------------------
MyTool::MyTool( const std::string& type,
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
const std::string&  MyTool::message() const
//------------------------------------------------------------------------------
{
  static std::string msg("It works!!!");
  return msg;
}

//------------------------------------------------------------------------------
void  MyTool::doIt()
//------------------------------------------------------------------------------
{
  info() << "doIt() has been called" << endmsg;
  debug() << "doIt() [DEBUG] has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  MyTool::initialize()
//------------------------------------------------------------------------------
{
  info() << "intialize() has been called" << endmsg;

  info() << "Int    = " << m_int    << endmsg;
  info() << "Double = " << m_double << endmsg;
  info() << "String = " << m_string << endmsg;
  info() << "Bool   = " << m_bool   << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  MyTool::finalize()
//------------------------------------------------------------------------------
{
  info() << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyTool::~MyTool( )
//------------------------------------------------------------------------------
{
  info() << "destructor has been called" << endmsg;
}

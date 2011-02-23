// $Id: MyTool.cpp,v 1.4 2006/05/04 15:17:33 hmd Exp $
// Framework include files
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyTool.h"

// Declaration of the AlgTool Factory
DECLARE_TOOL_FACTORY(MyTool)

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
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "doIt() has been called" << endmsg;
  log << MSG::DEBUG << "doIt() [DEBUG] has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  MyTool::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "intialize() has been called" << endmsg;

  log << MSG::INFO << "Int    = " << m_int    << endmsg;
  log << MSG::INFO << "Double = " << m_double << endmsg;
  log << MSG::INFO << "String = " << m_string << endmsg;
  log << MSG::INFO << "Bool   = " << m_bool   << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  MyTool::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyTool::~MyTool( )
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "destructor has been called" << endmsg;
}



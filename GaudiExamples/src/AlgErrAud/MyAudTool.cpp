// $Id: MyAudTool.cpp,v 1.1 2007/01/22 16:06:14 hmd Exp $
// Framework include files
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyAudTool.h"

// Declaration of the AlgTool Factory
DECLARE_TOOL_FACTORY(MyAudTool)

//------------------------------------------------------------------------------
MyAudTool::MyAudTool( const std::string& type,
                const std::string& name,
                const IInterface* parent )
//------------------------------------------------------------------------------
: base_class( type, name, parent ) {
}

//------------------------------------------------------------------------------
const std::string&  MyAudTool::message() const
//------------------------------------------------------------------------------
{
  static std::string msg("It works!!!");
  return msg;
}

//------------------------------------------------------------------------------
void  MyAudTool::doErr()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "doErr() has been called" << endmsg;
}
void  MyAudTool::doFatal()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::FATAL << "doFatal() has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  MyAudTool::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "intialize() has been called" << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  MyAudTool::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyAudTool::~MyAudTool( )
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "destructor has been called" << endmsg;
}



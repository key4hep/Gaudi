// Framework include files
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

// Tool example
#include "MyAudTool.h"

// Declaration of the AlgTool Factory
DECLARE_COMPONENT( MyAudTool )

//------------------------------------------------------------------------------
MyAudTool::MyAudTool( const std::string& type, const std::string& name, const IInterface* parent )
    //------------------------------------------------------------------------------
    : base_class( type, name, parent )
{
}

//------------------------------------------------------------------------------
const std::string& MyAudTool::message() const
//------------------------------------------------------------------------------
{
  static std::string msg( "It works!!!" );
  return msg;
}

//------------------------------------------------------------------------------
void MyAudTool::doErr()
//------------------------------------------------------------------------------
{
  error() << "doErr() has been called" << endmsg;
}
void MyAudTool::doFatal()
//------------------------------------------------------------------------------
{
  fatal() << "doFatal() has been called" << endmsg;
}

//------------------------------------------------------------------------------
StatusCode MyAudTool::initialize()
//------------------------------------------------------------------------------
{
  info() << "intialize() has been called" << endmsg;

  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode MyAudTool::finalize()
//------------------------------------------------------------------------------
{
  info() << "finalize() has been called" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
MyAudTool::~MyAudTool()
//------------------------------------------------------------------------------
{
  info() << "destructor has been called" << endmsg;
}

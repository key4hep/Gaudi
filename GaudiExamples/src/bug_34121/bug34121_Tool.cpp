// $Id: bug34121_Tool.cpp,v 1.1 2008/03/03 21:04:28 marcocle Exp $
// Framework include files
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"

#include "GaudiKernel/AlgTool.h"
#include "../AlgTools/IMyTool.h"

// Accessing data:
#include "GaudiKernel/PhysicalConstants.h"

namespace bug_34121 {

// Tool example
class Tool : public AlgTool,
                      virtual public IMyTool {
public:

  /// Standard Constructor
  Tool(const std::string& type,
                const std::string& name,
                const IInterface* parent);

  /// IMyTool interface
  virtual const std::string&  message() const;
  virtual void  doIt();
  /// Overriding initialize and finalize
  virtual StatusCode initialize();
  virtual StatusCode finalize();

protected:
  /// Standard destructor
  virtual ~Tool( );

private:
  /// Properties
  double       m_double;
};

//------------------------------------------------------------------------------
Tool::Tool( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
//------------------------------------------------------------------------------
: AlgTool( type, name, parent ) {

  // declare my special interface
  declareInterface<IMyTool>(this);

  // declare properties
  declareProperty( "Double", m_double = 100.);
}

//------------------------------------------------------------------------------
const std::string&  Tool::message() const
//------------------------------------------------------------------------------
{
  static std::string msg("It works!!!");
  return msg;
}

//------------------------------------------------------------------------------
void  Tool::doIt()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Double = " << m_double << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  Tool::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Initialize" << endmsg;
  log << MSG::INFO << "Double = " << m_double << endmsg;
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  Tool::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalize" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
Tool::~Tool( ) {}
//------------------------------------------------------------------------------
}

// Declaration of the AlgTool Factory
DECLARE_NAMESPACE_TOOL_FACTORY(bug_34121,Tool)

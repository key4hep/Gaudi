// Framework include files
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
  const std::string&  message() const override;
  void  doIt() override;
  /// Overriding initialize and finalize
  StatusCode initialize() override;
  StatusCode finalize() override;

protected:
  /// Standard destructor
  ~Tool( ) override;

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
  static const std::string msg("It works!!!");
  return msg;
}

//------------------------------------------------------------------------------
void  Tool::doIt()
//------------------------------------------------------------------------------
{
  info() << "Double = " << m_double << endmsg;
}

//------------------------------------------------------------------------------
StatusCode  Tool::initialize()
//------------------------------------------------------------------------------
{
  info() << "Initialize" << endmsg;
  info() << "Double = " << m_double << endmsg;
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
StatusCode  Tool::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalize" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
Tool::~Tool( ) {}
//------------------------------------------------------------------------------

// Declaration of the AlgTool Factory
DECLARE_COMPONENT(Tool)
} // namespace bug_34121

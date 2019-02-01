// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "../AlgTools/IMyTool.h"

namespace bug_34121 {

  class MyAlgorithm : public Algorithm {
  public:
    /// Constructor of this form must be provided
    using Algorithm::Algorithm;

    /// Three mandatory member functions of any algorithm
    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

  private:
    Gaudi::Property<std::string> m_toolName{this, "Tool", "bug_34121::Tool", "Type of the tool to use"};
    IMyTool*                     m_tool = nullptr;
  };

  //------------------------------------------------------------------------------
  StatusCode MyAlgorithm::initialize() {
    //------------------------------------------------------------------------------

    StatusCode sc;
    info() << "initializing...." << endmsg;

    sc = toolSvc()->retrieveTool( m_toolName, m_tool, this );
    if ( sc.isFailure() ) {
      error() << "Error retrieving the tool" << endmsg;
      return sc;
    }
    info() << "....initialization done" << endmsg;

    return StatusCode::SUCCESS;
  }

  //------------------------------------------------------------------------------
  StatusCode MyAlgorithm::execute() {
    //------------------------------------------------------------------------------
    info() << "executing...." << endmsg;

    m_tool->doIt();

    return StatusCode::SUCCESS;
  }

  //------------------------------------------------------------------------------
  StatusCode MyAlgorithm::finalize() {
    //------------------------------------------------------------------------------
    info() << "finalizing...." << endmsg;

    toolSvc()->releaseTool( m_tool ).ignore();

    return StatusCode::SUCCESS;
  }

  // Static Factory declaration
  DECLARE_COMPONENT( MyAlgorithm )

} // namespace bug_34121

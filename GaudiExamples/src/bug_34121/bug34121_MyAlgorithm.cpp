// $Id: bug34121_MyAlgorithm.cpp,v 1.1 2008/03/03 21:04:28 marcocle Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/Algorithm.h"

#include "../AlgTools/IMyTool.h"

namespace bug_34121 {

class MyAlgorithm : public Algorithm {
public:
  /// Constructor of this form must be provided
  MyAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  virtual ~MyAlgorithm(){}

private:

  std::string   m_toolName;
  IMyTool*      m_tool;

};

// Constructor
//------------------------------------------------------------------------------
MyAlgorithm::MyAlgorithm(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
  declareProperty("Tool", m_toolName = "bug_34121::Tool",
                  "Type of the tool to use");
}

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::initialize() {
//------------------------------------------------------------------------------

  MsgStream log(msgSvc(), name());
  StatusCode sc;
  log << MSG::INFO << "initializing...." << endmsg;

  sc = toolSvc()->retrieveTool(m_toolName, m_tool, this );
  if( sc.isFailure() ) {
    log << MSG::ERROR<< "Error retrieving the tool" << endmsg;
    return sc;
  }
  log << MSG::INFO << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode MyAlgorithm::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;

  m_tool->doIt();

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode MyAlgorithm::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;

  toolSvc()->releaseTool( m_tool ).ignore();

  return StatusCode::SUCCESS;
}

}

// Static Factory declaration
DECLARE_NAMESPACE_ALGORITHM_FACTORY(bug_34121,MyAlgorithm)


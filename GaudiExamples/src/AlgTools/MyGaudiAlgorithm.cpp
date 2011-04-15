// $Id: MyGaudiAlgorithm.cpp,v 1.1 2008/10/10 15:18:56 marcocle Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"

#include "IMyTool.h"
#include "MyGaudiAlgorithm.h"

// Static Factory declaration
DECLARE_ALGORITHM_FACTORY(MyGaudiAlgorithm)

// Constructor
//------------------------------------------------------------------------------
MyGaudiAlgorithm::MyGaudiAlgorithm(const std::string& name, ISvcLocator* ploc)
  : GaudiAlgorithm(name, ploc)
  , m_myPrivToolHandle("MyTool/PrivToolHandle", this)
  , m_myPubToolHandle("MyTool/PubToolHandle") {
  //------------------------------------------------------------------------------
  declareProperty("ToolWithName", m_privateToolType = "MyTool",
                  "Type of the tool to use (internal name is ToolWithName)");
  declareProperty("PrivToolHandle", m_myPrivToolHandle);
  declareProperty("PubToolHandle", m_myPubToolHandle);
}

//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::initialize() {
  //------------------------------------------------------------------------------

  StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  info() << "initializing...." << endmsg;

  m_publicTool   = tool<IMyTool>("MyTool");
  m_privateTool  = tool<IMyTool>("MyTool",this);
  m_publicGTool  = tool<IMyTool>("MyGaudiTool");
  m_privateGTool = tool<IMyTool>("MyGaudiTool",this);
  m_privateToolWithName = tool<IMyTool>(m_privateToolType, "ToolWithName", this);
  m_privateOtherInterface = tool<IMyOtherTool>("MyGaudiTool", this);
  // force initialization of tool handles
  if ( m_myPrivToolHandle.retrieve().isFailure() ||
       m_myPubToolHandle.retrieve().isFailure() ) {
    return StatusCode::FAILURE;
  }

  info() << "....initialization done" << endmsg;

  return sc;
}


//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  m_publicTool->doIt();
  m_privateTool->doIt();
  m_publicGTool->doIt();
  m_privateGTool->doIt();
  m_privateToolWithName->doIt();
  m_privateOtherInterface->doItAgain();

  m_myPrivToolHandle->doIt();
  m_myPubToolHandle->doIt();

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  return GaudiAlgorithm::finalize();
}

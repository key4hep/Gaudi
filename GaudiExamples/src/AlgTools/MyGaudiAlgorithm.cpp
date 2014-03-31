// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"

#include "IMyTool.h"
#include "MyGaudiAlgorithm.h"

// Static Factory declaration
DECLARE_COMPONENT(MyGaudiAlgorithm)

// Constructor
//------------------------------------------------------------------------------
MyGaudiAlgorithm::MyGaudiAlgorithm(const std::string& name, ISvcLocator* ploc)
  : GaudiAlgorithm(name, ploc) {
  //------------------------------------------------------------------------------
  declareProperty("ToolWithName", m_privateToolType = "MyTool",
                  "Type of the tool to use (internal name is ToolWithName)");
  declareProperty("PrivToolHandle", m_myPrivToolHandle);
  declareProperty("PubToolHandle", m_myPubToolHandle);

  declareInput("tracks", m_tracks, "/Event/Rec/Tracks");
  declareInput("hits", m_hits, "/Event/Rec/Hits");

  declareInput("raw", m_raw, std::vector<std::string>({"/Rec/RAW", "/DAQ/RAW"}));

  declareOutput("trackSelection", m_selectedTracks, "/Event/MyAnalysis/Tracks");

  std::cout << "handle " << m_tracks.dataProductName() << " is " << (m_tracks.isValid() ? "" : "NOT") << " valid" << std::endl;
  std::cout << "handle " << m_hits.dataProductName() << " is " << (m_hits.isValid() ? "" : "NOT") << " valid" << std::endl;
  std::cout << "handle " << m_raw.dataProductName() << " is " << (m_raw.isValid() ? "" : "NOT") << " valid" << std::endl;
  std::cout << "handle " << m_selectedTracks.dataProductName() << " is " << (m_selectedTracks.isValid() ? "" : "NOT") << " valid" << std::endl;

  declarePrivateTool(m_myPrivToolHandle, "MyTool/PrivToolHandle");
  declarePublicTool(m_myPubToolHandle, "MyTool/PubToolHandle");
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

  info() << m_tracks.dataProductName() << endmsg;
  info() << m_hits.dataProductName() << endmsg;
  info() << m_raw.dataProductName() << endmsg;

  info() << m_selectedTracks.dataProductName() << endmsg;

  info() << "....initialization done" << endmsg;

  return sc;
}


//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  info() << "tools created with tool<T>..." << endmsg;

  m_publicTool->doIt();
  m_privateTool->doIt();
  m_publicGTool->doIt();
  m_privateGTool->doIt();
  m_privateToolWithName->doIt();
  m_privateOtherInterface->doItAgain();

  info() << "tools created via ToolHandle<T>...." << endmsg;

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

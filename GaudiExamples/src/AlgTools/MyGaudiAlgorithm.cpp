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
  : GaudiAlgorithm(name, ploc),
  m_myPrivToolHandle("MyTool/PrivToolHandle",this),
  m_myPubToolHandle("MyTool/PubToolHandle"),
  m_myGenericToolHandle("MyTool/GenericToolHandle"),
  m_tracks("/Event/Rec/Tracks",Gaudi::DataHandle::Reader, this),
  m_hits("/Event/Rec/Hits",Gaudi::DataHandle::Reader,this),
  m_raw("/Rec/RAW",Gaudi::DataHandle::Reader,this),
  m_selectedTracks("/Event/MyAnalysis/Tracks",Gaudi::DataHandle::Writer,this)
 {
  //------------------------------------------------------------------------------
  declareProperty("ToolWithName", m_privateToolType = "MyTool",
                  "Type of the tool to use (internal name is ToolWithName)");
  declareProperty("PrivToolHandle", m_myPrivToolHandle);
  declareProperty("PubToolHandle", m_myPubToolHandle);
  declareProperty("GenericToolHandle", m_myGenericToolHandle);

  declareProperty("tracks", m_tracks, "the tracks");
  declareProperty("hits", m_hits, "the hits");
  declareProperty("raw", m_raw, "the raw stuff");

  declareProperty("trackSelection", m_selectedTracks, "the selected tracks");

  // declarePrivateTool(m_myPrivToolHandle, "MyTool/PrivToolHandle");
  // declarePublicTool(m_myPubToolHandle, "MyTool/PubToolHandle");

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
  if ( ! (m_myPrivToolHandle.retrieve() &&
          m_myPubToolHandle.retrieve()  &&
          m_myGenericToolHandle.retrieve() ) ) {
    error() << "Unable to retrive one of the ToolHandles" << endmsg;
    return StatusCode::FAILURE;
  }

  info() << m_tracks.objKey() << endmsg;
  info() << m_hits.objKey() << endmsg;
  info() << m_raw.objKey() << endmsg;

  info() << m_selectedTracks.objKey() << endmsg;

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

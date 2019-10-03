// Include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "IMyTool.h"
#include "MyGaudiAlgorithm.h"

// Static Factory declaration
DECLARE_COMPONENT( MyGaudiAlgorithm )

// Constructor
//------------------------------------------------------------------------------
MyGaudiAlgorithm::MyGaudiAlgorithm( const std::string& name, ISvcLocator* ploc ) : GaudiAlgorithm( name, ploc ) {
  // Keep at least one old-style ToolHandle property to test compilation
  declareProperty( "LegacyToolHandle", m_legacyToolHandle );
  declareProperty( "UndefinedToolHandle", m_undefinedToolHandle );

  //------------------------------------------------------------------------------
  m_myCopiedConstToolHandle  = m_myPubToolHandle;
  m_myCopiedToolHandle       = m_myPubToolHandle;
  m_myCopiedConstToolHandle2 = m_myConstToolHandle;
}

//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::initialize() {
  //------------------------------------------------------------------------------

  StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  info() << "initializing...." << endmsg;

  m_publicTool            = tool<IMyTool>( "MyTool" );
  m_privateTool           = tool<IMyTool>( "MyTool", this );
  m_publicGTool           = tool<IMyTool>( "MyGaudiTool" );
  m_privateGTool          = tool<IMyTool>( "MyGaudiTool", this );
  m_privateToolWithName   = tool<IMyTool>( m_privateToolType, "ToolWithName", this );
  m_privateOtherInterface = tool<IMyOtherTool>( "MyGaudiTool", this );

  // disable ToolHandle
  m_myUnusedToolHandle.disable();

  info() << m_tracks.objKey() << endmsg;
  info() << m_hits.objKey() << endmsg;
  info() << m_raw.objKey() << endmsg;

  info() << m_selectedTracks.objKey() << endmsg;

  // m_wrongIfaceTool is being retrieved via the wrong interface.
  // we expect the retrieve() to throw an exception.
  try {
    if ( m_wrongIfaceTool.retrieve().isFailure() ) {
      error() << "unable to retrieve " << m_wrongIfaceTool.typeAndName() << " (unexpected)" << endmsg;
      m_wrongIfaceTool.disable();
    }
  } catch ( GaudiException& ex ) {
    info() << "unable to retrieve " << m_wrongIfaceTool.typeAndName() << " (expected) with exception: " << ex.what()
           << endmsg;
    m_wrongIfaceTool.disable();
  }

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
  m_myConstToolHandle->doIt();

  info() << "tools copied assigned via ToolHandle<T>...." << endmsg;

  m_myCopiedConstToolHandle->doIt();
  m_myCopiedToolHandle->doIt();
  m_myCopiedConstToolHandle2->doIt();

  info() << "tools copied constructed via ToolHandle<T>...." << endmsg;

  // copy construct some handles
  ToolHandle<const IMyTool> h1( m_myPubToolHandle );
  ToolHandle<IMyTool>       h2( m_myPrivToolHandle );
  ToolHandle<const IMyTool> h3( m_myConstToolHandle );
  h1->doIt();
  h2->doIt();
  h3->doIt();

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyGaudiAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  return GaudiAlgorithm::finalize();
}

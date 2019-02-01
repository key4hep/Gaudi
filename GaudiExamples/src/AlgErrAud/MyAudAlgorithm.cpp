// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "IMyAudTool.h"
#include "MyAudAlgorithm.h"

// Static Factory declaration

DECLARE_COMPONENT( MyAudAlgorithm )

// Constructor
//------------------------------------------------------------------------------
MyAudAlgorithm::MyAudAlgorithm( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ), m_i( 0 ) {
  //------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------
StatusCode MyAudAlgorithm::initialize() {
  //------------------------------------------------------------------------------

  StatusCode sc;
  info() << "initializing...." << endmsg;

  sc = toolSvc()->retrieveTool( "MyAudTool", m_tool );
  if ( sc.isFailure() ) { error() << "Error retrieving the tool" << endmsg; }

  info() << "....initialization done" << endmsg;

  info() << "test " << std::setw( 4 ) << std::setfill( '0' ) << 10 << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAudAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  if ( m_i % 4 == 1 ) m_tool->doErr();
  if ( m_i % 4 == 2 ) m_tool->doFatal();
  if ( m_i % 5 == 0 ) error() << "testing AlgErrorAud" << endmsg;

  always() << "This message should never be suppressed" << endmsg;

  m_i++;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAudAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  always() << "none of the following 10 messages should be suppressed" << endmsg;

  for ( int i = 0; i < 10; ++i ) { m_tool->doErr(); }

  toolSvc()->releaseTool( m_tool ).ignore();

  return StatusCode::SUCCESS;
}

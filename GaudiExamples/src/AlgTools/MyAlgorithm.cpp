/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"

#include "IMyTool.h"
#include "MyAlgorithm.h"

// Static Factory declaration

DECLARE_COMPONENT( MyAlgorithm )

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::initialize() {
  //------------------------------------------------------------------------------

  StatusCode sc;
  info() << "initializing...." << endmsg;

  if ( !m_privateToolsOnly ) {
    sc = toolSvc()->retrieveTool( "MyTool", m_publicTool );
    if ( sc.isFailure() ) {
      error() << "Error retrieving the public tool" << endmsg;
      return sc;
    }
    sc = toolSvc()->retrieveTool( "MyGaudiTool", m_publicGTool );
    if ( sc.isFailure() ) {
      error() << "Error retrieving the Gaudi public tool" << endmsg;
      return sc;
    }
  }

  sc = toolSvc()->retrieveTool( "MyTool", m_privateTool, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool" << endmsg;
    return sc;
  }

  sc = toolSvc()->retrieveTool( "MyGaudiTool", m_privateGTool, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the Gaudi private tool" << endmsg;
    return sc;
  }

  sc = toolSvc()->retrieveTool( m_privateToolType, "ToolWithName", m_privateToolWithName, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool with name" << endmsg;
    return sc;
  }

  sc = toolSvc()->retrieveTool( "MyGaudiTool", m_privateOtherInterface, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the Gaudi private tool with second interface" << endmsg;
    return sc;
  }

  info() << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  if ( !m_privateToolsOnly ) {
    m_publicTool->doIt();
    m_publicGTool->doIt();
  }
  m_privateTool->doIt();
  m_privateGTool->doIt();
  m_privateToolWithName->doIt();
  m_privateOtherInterface->doItAgain();

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  if ( !m_privateToolsOnly ) {
    toolSvc()->releaseTool( m_publicTool ).ignore();
    toolSvc()->releaseTool( m_publicGTool ).ignore();
  }
  toolSvc()->releaseTool( m_privateTool ).ignore();
  toolSvc()->releaseTool( m_privateGTool ).ignore();
  toolSvc()->releaseTool( m_privateToolWithName ).ignore();
  toolSvc()->releaseTool( m_privateOtherInterface ).ignore();

  return StatusCode::SUCCESS;
}

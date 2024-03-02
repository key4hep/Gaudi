/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "MyAlgorithm.h"
#include "IMyTool.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IToolSvc.h>

DECLARE_COMPONENT( MyAlgorithm )

using Gaudi::TestSuite::TestAlg;

MyAlgorithm::MyAlgorithm( const std::string& name, ISvcLocator* ploc ) : TestAlg( name, ploc ) {
  // Keep at least one old-style ToolHandle property to test compilation
  declareProperty( "LegacyToolHandle", m_legacyToolHandle );
  declareProperty( "UndefinedToolHandle", m_undefinedToolHandle );

  m_myCopiedConstToolHandle  = m_myPubToolHandle;
  m_myCopiedToolHandle       = m_myPubToolHandle;
  m_myCopiedConstToolHandle2 = m_myConstToolHandle;
}

StatusCode MyAlgorithm::initialize() {
  StatusCode sc = TestAlg::initialize();
  if ( !sc ) return sc;
  info() << "initializing...." << endmsg;

  if ( !m_privateToolsOnly ) {
    sc = toolSvc()->retrieveTool( "MyTool", m_publicTool );
    if ( sc.isFailure() ) {
      error() << "Error retrieving the public tool" << endmsg;
      return sc;
    }
  }

  sc = toolSvc()->retrieveTool( "MyTool", m_privateTool, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool" << endmsg;
    return sc;
  }

  sc = toolSvc()->retrieveTool( m_privateToolType, "ToolWithName", m_privateToolWithName, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool with name" << endmsg;
    return sc;
  }

  sc = toolSvc()->retrieveTool( "MyTool", m_privateOtherInterface, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool with second interface" << endmsg;
    return sc;
  }

  // disable ToolHandle
  m_myUnusedToolHandle.disable();

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

  if ( m_privateToolsOnly ) {
    // disable all public tools
    m_myPubToolHandle.disable();
    m_myConstToolHandle.disable();
    m_myCopiedConstToolHandle.disable();
    m_myCopiedToolHandle.disable();
    m_myCopiedConstToolHandle2.disable();
  }

  info() << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  if ( !m_privateToolsOnly ) { m_publicTool->doIt(); }
  m_privateTool->doIt();
  m_privateToolWithName->doIt();
  m_privateOtherInterface->doItAgain();

  info() << "tools created via ToolHandle<T>...." << endmsg;

  m_myPrivToolHandle->doIt();
  if ( !m_privateToolsOnly ) {
    m_myPubToolHandle->doIt();
    m_myConstToolHandle->doIt();
  }

  if ( !m_privateToolsOnly ) {
    info() << "tools copied assigned via ToolHandle<T>...." << endmsg;

    m_myCopiedConstToolHandle->doIt();
    m_myCopiedToolHandle->doIt();
    m_myCopiedConstToolHandle2->doIt();
  }

  info() << "tools copied constructed via ToolHandle<T>...." << endmsg;

  // copy construct some handles
  ToolHandle<IMyTool> h1( m_myPrivToolHandle );
  h1->doIt();

  if ( !m_privateToolsOnly ) {
    ToolHandle<const IMyTool> h2( m_myPubToolHandle );
    ToolHandle<const IMyTool> h3( m_myConstToolHandle );
    h2->doIt();
    h3->doIt();
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode MyAlgorithm::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  if ( !m_privateToolsOnly ) { toolSvc()->releaseTool( m_publicTool ).ignore(); }
  toolSvc()->releaseTool( m_privateTool ).ignore();
  toolSvc()->releaseTool( m_privateToolWithName ).ignore();
  toolSvc()->releaseTool( m_privateOtherInterface ).ignore();

  return TestAlg::finalize();
}

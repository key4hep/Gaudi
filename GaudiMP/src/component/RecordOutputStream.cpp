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

// local
#include "RecordOutputStream.h"

// ----------------------------------------------------------------------------
// Implementation file for class: RecordOutputStream
//
// 30/08/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT( RecordOutputStream )

// ============================================================================
// Initialization
// ============================================================================
StatusCode RecordOutputStream::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;              // error printed already by GaudiAlgorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;

  if ( m_streamName.empty() ) {
    m_streamName = "Deferred:" + name();
    debug() << "Using default OutputStreamName: '" << m_streamName << "'" << endmsg;
  }

  m_flagLocation = locationRoot() + "/" + m_streamName;
  return StatusCode::SUCCESS;
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode RecordOutputStream::execute() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  getOrCreate<DataObject, DataObject>( m_flagLocation, false );
  /*
  if (!exist(m_flagLocation, false)) {
    DataObject *obj = new DataObject();
    put(obj, m_flagLocation, false);
  }
  */
  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode RecordOutputStream::finalize() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

// ============================================================================

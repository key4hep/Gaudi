/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// FrameWork includes
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObject.h>

// STL includes
#include <string>

// Local includes
#include "OutputStreamShared.h"

/** @class RecordOutputStream
 *
 * Simple class that adds an entry to the Transient Event Store to record that
 * the processing flow triggered its execution.
 *
 * The main use case is to replace OutputStream instances appearing in the
 * normal processing flow of workers in GaudiMP jobs. The information is then
 * passed to the writer process where an instance of @c ReplayOutputStream will
 * trigger the actual writing.
 *
 * @author Marco Clemencic
 * @date 30/08/2013
 */
class RecordOutputStream : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;

private:
  Gaudi::Property<std::string> m_streamName{
      this, "OutputStreamName", {}, "Name of the OutputStream instance should be triggered." };

  /// location of the DataObject flag used to record that this algorithm was
  /// called
  std::string m_flagLocation;
};

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
  StatusCode sc = Algorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;         // error printed already by Algorithm

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

  DataObject* tmp = nullptr;
  if ( !eventSvc()->retrieveObject( m_flagLocation, tmp ) ) {
    tmp = new DataObject();
    return eventSvc()->registerObject( m_flagLocation, tmp );
  }
  return StatusCode::SUCCESS;
}

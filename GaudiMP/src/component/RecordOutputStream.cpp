// Include files

// From Gaudi
#include "GaudiKernel/AlgFactory.h"
// local
#include "RecordOutputStream.h"

// ----------------------------------------------------------------------------
// Implementation file for class: RecordOutputStream
//
// 30/08/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_ALGORITHM_FACTORY(RecordOutputStream)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
RecordOutputStream::RecordOutputStream(const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator)
{
  declareProperty("OutputStreamName", m_streamName,
                  "Name of the OutputStream instance should be triggered.");
}

// ============================================================================
// Destructor
// ============================================================================
RecordOutputStream::~RecordOutputStream() {}

// ============================================================================
// Initialization
// ============================================================================
StatusCode RecordOutputStream::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  if (m_streamName.empty()) {
    m_streamName = "Deferred:" + name();
    debug() << "Using default OutputStreamName: '"
            << m_streamName << "'" << endmsg;
  }

  m_flagLocation = locationRoot() + "/" + m_streamName;
  return StatusCode::SUCCESS;
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode RecordOutputStream::execute() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  getOrCreate<DataObject, DataObject>(m_flagLocation, false);
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
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

// ============================================================================

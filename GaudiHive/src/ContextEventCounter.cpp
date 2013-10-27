// Include files

// From Gaudi
#include "GaudiKernel/AlgFactory.h"
// local
#include "ContextEventCounter.h"

// ----------------------------------------------------------------------------
// Implementation file for class: ContextEventCounter
// 
// 27/10/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_ALGORITHM_FACTORY(ContextEventCounter)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
ContextEventCounter::ContextEventCounter(const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator)
{
  
}

// ============================================================================
// Destructor
// ============================================================================
ContextEventCounter::~ContextEventCounter() {}

// ============================================================================
// Initialization
// ============================================================================
StatusCode ContextEventCounter::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm
  
// TODO initialization procedure

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  return StatusCode::SUCCESS;
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode ContextEventCounter::execute() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  
  if (!m_ctxtSpecCounter) {
    m_ctxtSpecCounter = new int(1);
  } else {
    ++(*m_ctxtSpecCounter);
  }
  debug() << "Context " << Gaudi::Hive::currentContextId() << " "
          << m_ctxtSpecCounter.get() << " -> " << *m_ctxtSpecCounter << endmsg;
  
  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode ContextEventCounter::finalize() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  info() << "Partial counts:" << endmsg;

  info() << "Total count of events: " << m_ctxtSpecCounter.accumulate(
          [this] (const int* p) -> int {
            const int r = (p) ? *p : 0;
            // print partial counts
            this->info() << " " << r << endmsg;
            return r;
          }
          , 0) << endmsg;

  m_ctxtSpecCounter.deleteAll();

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

// ============================================================================

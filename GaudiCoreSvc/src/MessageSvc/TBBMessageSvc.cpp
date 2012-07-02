// Include files

// From Gaudi
#include "GaudiKernel/SvcFactory.h"
// local
#include "TBBMessageSvc.h"

// ----------------------------------------------------------------------------
// Implementation file for class: TBBMessageSvc
//
// 22/06/2012: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_SERVICE_FACTORY(TBBMessageSvc);

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
TBBMessageSvc::TBBMessageSvc(const std::string& name, ISvcLocator* pSvcLocator)
  : MessageSvc(name, pSvcLocator), m_barrier(new(tbb::task::allocate_root()) tbb::empty_task)
{
  m_barrier->increment_ref_count();
}

// ============================================================================
// Destructor
// ============================================================================
TBBMessageSvc::~TBBMessageSvc() {
  i_processMessages();
  m_barrier->wait_for_all();
  tbb::task::destroy(*m_barrier);
}

// ============================================================================
// Initialization
// ============================================================================
StatusCode TBBMessageSvc::initialize() {
  StatusCode sc = MessageSvc::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode TBBMessageSvc::finalize() {
  return MessageSvc::finalize(); // must be called after all other actions
}

void TBBMessageSvc::reportMessage(const Message& msg, int outputLevel) {
  m_messageQueue.push(new MessageType(msg, outputLevel));
  i_processMessages();
}

// ============================================================================

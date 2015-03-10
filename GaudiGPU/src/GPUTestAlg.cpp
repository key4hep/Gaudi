// Include files

// From Gaudi
#include "GaudiKernel/AlgFactory.h"
// local
#include "GPUTestAlg.h"

// ----------------------------------------------------------------------------
// Implementation file for class: GPUTestAlg
// 
// 10/03/2015: Illya Shapoval
// ----------------------------------------------------------------------------
DECLARE_ALGORITHM_FACTORY(GPUTestAlg)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
GPUTestAlg::GPUTestAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator)
{
  
}

// ============================================================================
// Destructor
// ============================================================================
GPUTestAlg::~GPUTestAlg() {}

// ============================================================================
// Initialization
// ============================================================================
StatusCode GPUTestAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm
  
// TODO initialization procedure

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  return StatusCode::SUCCESS;
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode GPUTestAlg::execute() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  
// TODO execution logic

  
  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode GPUTestAlg::finalize() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;
  
// TODO Implement finalize

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

// ============================================================================
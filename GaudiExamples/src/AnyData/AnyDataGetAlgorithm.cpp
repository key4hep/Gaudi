// Include files

 // from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/AnyDataWrapper.h"

// local
#include "AnyDataGetAlgorithm.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AnyDataGetAlgorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AnyDataGetAlgorithm )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AnyDataGetAlgorithm::AnyDataGetAlgorithm( const std::string& name,
                                          ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
   declareProperty("Locations", m_locations);
}
//=============================================================================
// Destructor
//=============================================================================
AnyDataGetAlgorithm::~AnyDataGetAlgorithm() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode AnyDataGetAlgorithm::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AnyDataGetAlgorithm::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  for (const auto& l : m_locations) {
     auto base = getIfExists<AnyDataWrapperBase>(l);
     if (base) {
        info() << "Got base from " << l << endmsg;
     }
     const auto i = dynamic_cast<const AnyDataWrapper<int>*>(base);
     if (i) {
        info() << "Got int from " << l << ": " << *(i->getData()) << endmsg;
     }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AnyDataGetAlgorithm::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

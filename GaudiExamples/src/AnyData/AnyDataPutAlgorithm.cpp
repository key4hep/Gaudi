// Include files
#include <vector>

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/AnyDataWrapper.h"

// local
#include "AnyDataPutAlgorithm.h"

//-----------------------------------------------------------------------------
// Implementation file for class : AnyDataPutAlgorithm
//
// 2016-05-26 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( AnyDataPutAlgorithm )

namespace {
   using std::vector;
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AnyDataPutAlgorithm::AnyDataPutAlgorithm( const std::string& name,
                                          ISvcLocator* pSvcLocator)
: GaudiAlgorithm ( name , pSvcLocator )
{
   declareProperty("Location", m_loc = "Test");
}
//=============================================================================
// Destructor
//=============================================================================
AnyDataPutAlgorithm::~AnyDataPutAlgorithm() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode AnyDataPutAlgorithm::initialize() {
   StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
   if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

   if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

   return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AnyDataPutAlgorithm::execute() {

   if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

   auto i = new AnyDataWrapper<int>{0};
   auto j = new AnyDataWrapper<vector<int>>{vector<int>{0, 1, 2, 3}};

   put(i, m_loc + "/One");
   put(j, m_loc + "/Two");

   return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AnyDataPutAlgorithm::finalize() {

   if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

   return GaudiAlgorithm::finalize();  // must be called after all other actions
}

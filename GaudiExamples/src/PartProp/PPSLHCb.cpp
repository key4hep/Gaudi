// $Id: PPSLHCb.cpp,v 1.1 2008/05/29 18:12:56 marcocle Exp $
// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IParticlePropertySvc.h"

// local
#include "PPSLHCb.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PPSLHCb
//
// 2008-05-23 : Marco CLEMENCIC
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PPSLHCb );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PPSLHCb::PPSLHCb( const std::string& name,
                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{

}
//=============================================================================
// Destructor
//=============================================================================
PPSLHCb::~PPSLHCb() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode PPSLHCb::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  svc<IParticlePropertySvc>("ParticlePropertySvc",true);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PPSLHCb::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode PPSLHCb::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================

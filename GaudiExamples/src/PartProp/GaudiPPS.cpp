// Include files
// from Gaudi
#include "GaudiKernel/IParticlePropertySvc.h"

// local
#include "GaudiPPS.h"

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiPPS
//
// 2008-05-23 : Marco CLEMENCIC
//-----------------------------------------------------------------------------
namespace GaudiExamples {
// Declaration of the Algorithm Factory
DECLARE_COMPONENT( GaudiPPS )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GaudiPPS::GaudiPPS( const std::string& name,
                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{

}
//=============================================================================
// Destructor
//=============================================================================
GaudiPPS::~GaudiPPS() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode GaudiPPS::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  svc<IParticlePropertySvc>("Gaudi::ParticlePropertySvc",true);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode GaudiPPS::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode GaudiPPS::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

} // namespace GaudiExamples
//=============================================================================

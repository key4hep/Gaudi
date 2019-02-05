#define GAUDI_RNDMGENSVC_RNDMGENGENERATORS_CPP

// Framework include files
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/SmartIF.h"

// Construct and initialize the generator
Rndm::Numbers::Numbers( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par ) {
  StatusCode status = initialize( svc, par );
  if ( !status.isSuccess() ) { throw GaudiException( "Initialization failed !", "Rndm::Numbers", status ); }
}

// Standard destructor
Rndm::Numbers::~Numbers() { finalize().ignore(); }

// Initialize the generator
StatusCode Rndm::Numbers::initialize( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par ) {
  if ( svc && !m_generator ) m_generator = svc->generator( par );
  return m_generator ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

// Initialize the generator
StatusCode Rndm::Numbers::finalize() {
  if ( m_generator ) {
    m_generator->finalize().ignore();
    m_generator.reset();
  }
  return StatusCode::SUCCESS;
}

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
// Construct and initialize the generator
Rndm::Numbers::Numbers( IRndmGenSvc* svc, const IRndmGen::Param& par ) {
  StatusCode status = initialize( svc, par );
  if ( !status.isSuccess() ) { throw GaudiException( "Initialization failed !", "Rndm::Numbers", status ); }
}

// Initialize the generator
StatusCode Rndm::Numbers::initialize( IRndmGenSvc* svc, const IRndmGen::Param& par ) {
  return initialize( SmartIF<IRndmGenSvc>( svc ), par );
}
#endif

/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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

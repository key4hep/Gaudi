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
  // Initialization
  //=============================================================================
  StatusCode GaudiPPS::initialize() {
    StatusCode sc = Algorithm::initialize(); // must be executed first
    if ( sc.isFailure() ) return sc;         // error printed already by Algorithm

    if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;

    auto pps = service<IParticlePropertySvc>( "Gaudi::ParticlePropertySvc", true );

    return StatusCode::SUCCESS;
  }

  //=============================================================================
  // Main execution
  //=============================================================================
  StatusCode GaudiPPS::execute() {

    if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

    return StatusCode::SUCCESS;
  }

} // namespace GaudiExamples
//=============================================================================

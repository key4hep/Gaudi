/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <GaudiKernel/Algorithm.h>

/** @class AuditorTestAlg AuditorTestAlg.cpp
 *
 *
 *  @author Marco Clemencic
 *  @date   Apr 2, 2008
 */
class AuditorTestAlg : public Algorithm {
public:
  /// Standard constructor
  AuditorTestAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~AuditorTestAlg() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

protected:
private:
};

//-----------------------------------------------------------------------------
// Implementation for class : AuditorTestAlg
//
// Apr 2, 2008 : Marco Clemencic
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( AuditorTestAlg )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AuditorTestAlg::AuditorTestAlg( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {}
//=============================================================================
// Destructor
//=============================================================================
AuditorTestAlg::~AuditorTestAlg() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode AuditorTestAlg::initialize() {
  StatusCode sc = Algorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;         // error printed already by Algorithm

  debug() << "==> Initialize" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode AuditorTestAlg::execute() {

  debug() << "==> Execute" << endmsg;

  auditorSvc()->before( "loop", name(), EventContext{} );
  for ( long i = 0; i < 1000000; ++i ) {}
  auditorSvc()->after( "loop", name(), EventContext{} );

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode AuditorTestAlg::finalize() {

  debug() << "==> Finalize" << endmsg;

  return Algorithm::finalize(); // must be called after all other actions
}

//=============================================================================

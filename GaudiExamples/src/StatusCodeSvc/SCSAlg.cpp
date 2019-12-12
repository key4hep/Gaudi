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
#include "SCSAlg.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::execute() {

  fncChecked();
  fncUnchecked();
  fncUnchecked2();
  fncIgnored();

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::test() { return StatusCode::SUCCESS; }

void __attribute__( ( noinline ) ) SCSAlg::fncChecked() { test().isSuccess(); }

void __attribute__( ( noinline ) ) SCSAlg::fncUnchecked() { test(); }

void SCSAlg::fncIgnored() {
  // Ignore all unchecked StatusCode instances in the current scope.
  StatusCode::ScopedDisableChecking _sc_ignore;

  test();
}

// Static Factory declaration
DECLARE_COMPONENT( SCSAlg )

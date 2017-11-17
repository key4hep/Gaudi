#include "SCSAlg.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::execute()
{

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

void SCSAlg::fncIgnored()
{
  // Ignore all unchecked StatusCode instances in the current scope.
  StatusCode::ScopedDisableChecking _sc_ignore;

  test();
}

// Static Factory declaration
DECLARE_COMPONENT( SCSAlg )

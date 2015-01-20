#include "SCSAlg.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"

///////////////////////////////////////////////////////////////////////////
SCSAlg::SCSAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
  Algorithm(name, pSvcLocator)
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::initialize() {
  return Algorithm::initialize();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::execute() {

  MsgStream log(msgSvc(), name());


  fncChecked();
  fncUnchecked();
  fncUnchecked2();
  fncIgnored();

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode SCSAlg::finalize() {
  return Algorithm::finalize();
}

StatusCode SCSAlg::test() {
  return StatusCode::SUCCESS;
}

void SCSAlg::fncChecked() {

  test().isSuccess();

}

void SCSAlg::fncUnchecked() {

  test();

}

#ifndef special_fncUnchecked2
void SCSAlg::fncUnchecked2() {

  test();

}
#endif

void SCSAlg::fncIgnored() {
  // Ignore all unchecked StatusCode instances in the current scope.
  StatusCode::ScopedDisableChecking _sc_ignore;

  test();

}

// Static Factory declaration
DECLARE_COMPONENT(SCSAlg)

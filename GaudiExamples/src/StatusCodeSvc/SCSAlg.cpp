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

void SCSAlg::fncUnchecked2() {

  test();

}

// Static Factory declaration
DECLARE_COMPONENT(SCSAlg)

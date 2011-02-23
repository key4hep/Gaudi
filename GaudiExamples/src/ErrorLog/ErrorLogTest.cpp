// $Id: ErrorLogTest.cpp,v 1.3 2006/11/30 10:35:26 mato Exp $


// Include files
#include "ErrorLogTest.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/RndmGenerators.h"

#include "GaudiKernel/IssueSeverity.h"

DECLARE_ALGORITHM_FACTORY(ErrorLogTest)

//------------------------------------------------------------------------------
ErrorLogTest::ErrorLogTest(const std::string& name,
				 ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator)
//------------------------------------------------------------------------------
{
}


//------------------------------------------------------------------------------
StatusCode ErrorLogTest::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

//   if (service("THistSvc",m_ths).isFailure()) {
//     log << MSG::ERROR << "Couldn't get THistSvc" << endmsg;
//     return StatusCode::FAILURE;
//   }


  IssueSeverity err0 =ISSUE(IssueSeverity::ERROR,"ERROR level ErrObj");
  IssueSeverity war = ISSUE(IssueSeverity::WARNING,"this is a warning");
  IssueSeverity fat = ISSUE(IssueSeverity::FATAL,"this is a fatal");

  //  StatusCode sc(StatusCode::SUCCESS, ISSUE(IssueSeverity::DEBUG,"debug3 ErrObj"));

  StatusCode sc = STATUSCODE(StatusCode::SUCCESS, IssueSeverity::DEBUG, "debug");

  StatusCode sc2(ISSUE(IssueSeverity::INFO,"info ErrObj"));

  return sc2;

}


//------------------------------------------------------------------------------
StatusCode ErrorLogTest::execute()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  ISSUE(IssueSeverity::WARNING,"warmomg level ErrObj in execute");

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode ErrorLogTest::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::WARNING << "Finalizing..." << endmsg;



  return StatusCode::SUCCESS;
}

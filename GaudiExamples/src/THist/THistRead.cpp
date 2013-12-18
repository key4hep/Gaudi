// Include files
#include "THistRead.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ITHistSvc.h"
#include <math.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TTree.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TError.h"

DECLARE_COMPONENT(THistRead)

//------------------------------------------------------------------------------
THistRead::THistRead(const std::string& name,
				 ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator), m_ths(0)
//------------------------------------------------------------------------------
{
  m_h1 = 0;
}


//------------------------------------------------------------------------------
StatusCode THistRead::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  if (service("THistSvc",m_ths).isFailure()) {
    log << MSG::ERROR << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // stream read1, 1D in "/xxx"
  TH1 *h1(0);
  if (m_ths->regHist("/read1/xxx/1Dgauss").isFailure() ||
      m_ths->getHist("/read1/xxx/1Dgauss",h1).isFailure()) {
    log << MSG::ERROR << "Couldn't read gauss1d" << endmsg;
  } else {
    log << MSG::INFO << h1->GetName() << ": " << h1->GetEntries()
	<< endmsg;
  }


  // stream read2, 2D tree in "/"
  TH2* h2(0);
  if (m_ths->regHist("/read2/2Dgauss").isFailure() ||
      m_ths->getHist("/read2/2Dgauss",h2).isFailure()) {
    log << MSG::ERROR << "Couldn't read 2Dgauss" << endmsg;
  } else {
    log << MSG::INFO << h2->GetName() << ": " << h2->GetEntries()
	<< endmsg;
  }

  // 3D tree in "/"
  TH3* h3(0);
  if (m_ths->regHist("/read2/3Dgauss").isFailure() ||
      m_ths->getHist("/read2/3Dgauss",h3).isFailure()) {
    log << MSG::ERROR << "Couldn't read 3Dgauss" << endmsg;
  } else {
    log << MSG::INFO << h3->GetName() << ": " << h3->GetEntries()
	<< endmsg;
  }

  // Profile in "/"
  TH1* tp(0);
  if (m_ths->regHist("/read2/profile").isFailure() ||
      m_ths->getHist("/read2/profile",tp).isFailure()) {
    log << MSG::ERROR << "Couldn't read profile" << endmsg;
  } else {
    log << MSG::INFO << tp->GetName() << ": " << tp->GetEntries()
	<< endmsg;
  }


  // Tree with branches in "/trees/stuff"
  TTree *tr(0);
  if (m_ths->regTree("/read2/trees/stuff/treename").isFailure() ||
      m_ths->getTree("/read2/trees/stuff/treename",tr).isFailure()) {
    log << MSG::ERROR << "Couldn't read tree" << endmsg;
  } else {
    log << MSG::INFO << tr->GetName() << ": " << tr->GetEntries()
	<< endmsg;
  }


  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode THistRead::execute()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );


  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode THistRead::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::DEBUG << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}

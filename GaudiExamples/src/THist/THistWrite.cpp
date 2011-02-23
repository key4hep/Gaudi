// $Id: THistWrite.cpp,v 1.3 2008/05/13 10:05:47 marcocle Exp $

// Include files
#include "THistWrite.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ITHistSvc.h"
#include <math.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TTree.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TError.h"

DECLARE_ALGORITHM_FACTORY(THistWrite)

//------------------------------------------------------------------------------
THistWrite::THistWrite(const std::string& name,
				 ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator), m_ths(0)
//------------------------------------------------------------------------------
{
  m_h1 = 0;
}


//------------------------------------------------------------------------------
StatusCode THistWrite::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  if (service("THistSvc",m_ths).isFailure()) {
    log << MSG::ERROR << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }


  // Temporary Trees
  TH1F* h1 = new TH1F("TempHist1","Temporary Tree 1",100,0.,100.);
  if (m_ths->regHist("TempHist1",h1).isFailure()) {
    log << MSG::ERROR << "Couldn't register TempHist1" << endmsg;
  }

  TH1F* h1a = new TH1F("TempHist1a","Temporary Tree 1a",100,0.,100.);
  if (m_ths->regHist("other/TempHist1a",h1a).isFailure()) {
    log << MSG::ERROR << "Couldn't register TempHist1a" << endmsg;
  }



  // Write to stream "new"
  TH1F* h2 = new TH1F("Tree2","Tree 2",100,0.,100.);
  if (m_ths->regHist("/new/Tree2",h2).isFailure()) {
    log << MSG::ERROR << "Couldn't register Tree2" << endmsg;
  }



  // Update to stream "upd", dir "/xxx"
  TH1F* h3 = new TH1F("1Dgauss","1D Gaussian",100,-50.,50.);
  if (m_ths->regHist("/upd/xxx/gauss1d",h3).isFailure()) {
    log << MSG::ERROR << "Couldn't register gauss1d" << endmsg;
  }

  // Recreate 2D tree in "/"
  TH2F* h3a = new TH2F("2Dgauss","2D Gaussian",100,-50.,50.,100,-50,50);
  if (m_ths->regHist("/rec/gauss2d",h3a).isFailure()) {
    log << MSG::ERROR << "Couldn't register gauss2d" << endmsg;
  }

  // 3D tree in "/"
  TH3F* h4 = new TH3F("3Dgauss","3D Gaussian",100,-50.,50.,100,-50,50,
		      100,-50,50);
  if (m_ths->regHist("/rec/gauss3d",h4).isFailure()) {
    log << MSG::ERROR << "Couldn't register gauss3d" << endmsg;
  }

  // Profile in "/"
  TH1* tp = new TProfile("profile","profile",100, -50., -50.);
  if (m_ths->regHist("/rec/prof",tp).isFailure()) {
    log << MSG::ERROR << "Couldn't register prof" << endmsg;
  }


  // Tree with branches in "/trees/stuff"
  TTree *tr = new TTree("treename","tree title");
  if (m_ths->regTree("/rec/trees/stuff/tree1",tr).isFailure()) {
    log << MSG::ERROR << "Couldn't register tr" << endmsg;
  }


  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode THistWrite::execute()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  Rndm::Numbers gauss(randSvc(),       Rndm::Gauss(0.,15.));

  static int n = 0;

  double x = sin(double(n)) * 52. + 50.;

  TH1 *h(0);
  TH2 *h2(0);
  if (m_ths->getHist("TempHist1",h).isSuccess()) {
    h->Fill(x);
  } else {
    log << MSG::ERROR << "Couldn't retrieve TempHist 1" << endmsg;
  }

  if (m_ths->getHist("other/TempHist1a",h).isSuccess()) {
    h->Fill(x);
  } else {
    log << MSG::ERROR << "Couldn't retrieve TempHist 1a" << endmsg;
  }


  if (m_ths->getHist("/new/Tree2",h).isSuccess()) {
    h->Fill(x);
  } else {
    log << MSG::ERROR << "Couldn't retrieve Tree2" << endmsg;
  }

  if (m_ths->getHist("/upd/xxx/gauss1d",h).isSuccess()) {
    for (int i=0; i<1000; ++i) {
      h->Fill(gauss(),1.);
    }
  } else {
    log << MSG::ERROR << "Couldn't retrieve 1Dgauss" << endmsg;
  }

  if (m_ths->getHist("/rec/gauss2d",h2).isSuccess()) {
    for (int i=0; i<1000; ++i) {
      h2->Fill(gauss(),gauss(),1.);
    }
  } else {
    log << MSG::ERROR << "Couldn't retrieve 2Dgauss" << endmsg;
  }

  TH3 *h3(0);
  if (m_ths->getHist("/rec/gauss3d",h3).isSuccess()) {
    for (int i=0; i<1000; ++i) {
      h3->Fill(gauss(),gauss(),gauss(),1.);
    }
  } else {
    log << MSG::ERROR << "Couldn't retrieve 3Dgauss" << endmsg;
  }

  TTree *tr;
  if (m_ths->getTree("/rec/trees/stuff/tree1",tr).isFailure()) {
    log << MSG::ERROR << "Couldn't retrieve tree tree1" << endmsg;
  } else {
    if (n == 0) {
      int p1,p2,p3;
      tr->Branch("branch1",&p1,"point1/I");
      tr->Branch("branch2",&p2,"point2/I");
      tr->Branch("branch3",&p3,"point3/I");

      for (int i=0; i<1000; i++) {
	p1 = i;
	p2 = i%10;
	p3 = i%7;

	tr->Fill();
      }
    }
  }


  n++;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
void
THistWrite::listKeys(TDirectory *td) {
//------------------------------------------------------------------------------

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "printing keys for: " << td->GetPath()
      << "  (" << td->GetList()->GetSize() << ")" << endmsg;
  TIter nextkey(td->GetList());
  while (TKey *key = (TKey*)nextkey()) {
    if (key != 0) {
      log << MSG::INFO << key->GetName() << " ("
	  << key->IsA()->GetName() << ") "
// 	  << key->GetObjectStat()
// 	  << "  " << key->IsOnHeap()
	  << key->GetCycle()
       	  << endmsg;
    } else { log << MSG::INFO << "key == 0" << endmsg; }

  }

  return;
}

//------------------------------------------------------------------------------
StatusCode THistWrite::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalizing..." << endmsg;


  return StatusCode::SUCCESS;
}

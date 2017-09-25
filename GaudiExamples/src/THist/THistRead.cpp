// Include files
#include "THistRead.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/RndmGenerators.h"
#include <math.h>

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TKey.h"
#include "TTree.h"

DECLARE_COMPONENT( THistRead )

//------------------------------------------------------------------------------
THistRead::THistRead( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ), m_ths( nullptr )
//------------------------------------------------------------------------------
{
  m_h1 = nullptr;
}

//------------------------------------------------------------------------------
StatusCode THistRead::initialize()
//------------------------------------------------------------------------------
{
  if ( service( "THistSvc", m_ths ).isFailure() ) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // stream read1, 1D in "/xxx"
  TH1* h1 = nullptr;
  if ( m_ths->regHist( "/read1/xxx/1Dgauss" ).isFailure() || m_ths->getHist( "/read1/xxx/1Dgauss", h1 ).isFailure() ) {
    error() << "Couldn't read gauss1d" << endmsg;
  } else {
    info() << h1->GetName() << ": " << h1->GetEntries() << endmsg;
  }

  // stream read2, 2D tree in "/"
  TH2* h2 = nullptr;
  if ( m_ths->regHist( "/read2/2Dgauss" ).isFailure() || m_ths->getHist( "/read2/2Dgauss", h2 ).isFailure() ) {
    error() << "Couldn't read 2Dgauss" << endmsg;
  } else {
    info() << h2->GetName() << ": " << h2->GetEntries() << endmsg;
  }

  // 3D tree in "/"
  TH3* h3 = nullptr;
  if ( m_ths->regHist( "/read2/3Dgauss" ).isFailure() || m_ths->getHist( "/read2/3Dgauss", h3 ).isFailure() ) {
    error() << "Couldn't read 3Dgauss" << endmsg;
  } else {
    info() << h3->GetName() << ": " << h3->GetEntries() << endmsg;
  }

  // Profile in "/"
  TH1* tp = nullptr;
  if ( m_ths->regHist( "/read2/profile" ).isFailure() || m_ths->getHist( "/read2/profile", tp ).isFailure() ) {
    error() << "Couldn't read profile" << endmsg;
  } else {
    info() << tp->GetName() << ": " << tp->GetEntries() << endmsg;
  }

  // Tree with branches in "/trees/stuff"
  TTree* tr = nullptr;
  if ( m_ths->regTree( "/read2/trees/stuff/treename" ).isFailure() ||
       m_ths->getTree( "/read2/trees/stuff/treename", tr ).isFailure() ) {
    error() << "Couldn't read tree" << endmsg;
  } else {
    info() << tr->GetName() << ": " << tr->GetEntries() << endmsg;
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode THistRead::execute()
//------------------------------------------------------------------------------
{
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode THistRead::finalize()
//------------------------------------------------------------------------------
{
  debug() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}

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
  StatusCode sc1 = m_ths->regHist( "/read1/xxx/1Dgauss" );
  TH1* h1 = m_ths->getHistTH1( "/read1/xxx/1Dgauss" );
  if ( sc1.isFailure() || h1 == nullptr ) {
    error() << "Couldn't read gauss1d" << endmsg;
  } else {
    info() << h1->GetName() << ": " << h1->GetEntries() << endmsg;
  }

  // stream read2, 2D tree in "/"
  StatusCode sc2 = m_ths->regHist( "/read2/2Dgauss" );
  TH2* h2 = m_ths->getHistTH2( "/read2/2Dgauss" );
  if ( sc2.isFailure() || h2 == nullptr ) {
    error() << "Couldn't read 2Dgauss" << endmsg;
  } else {
    info() << h2->GetName() << ": " << h2->GetEntries() << endmsg;
  }

  // 3D tree in "/"
  StatusCode sc3 = m_ths->regHist( "/read2/3Dgauss" );
  TH3* h3 = m_ths->getHistTH3( "/read2/3Dgauss" );
  if ( sc3.isFailure() || h3 == nullptr ) {
    error() << "Couldn't read 3Dgauss" << endmsg;
  } else {
    info() << h3->GetName() << ": " << h3->GetEntries() << endmsg;
  }

  // Profile in "/"
  StatusCode sc4 = m_ths->regHist( "/read2/profile" );
  TH1* tp = m_ths->getHistTH1( "/read2/profile" );
  if ( sc4.isFailure() || tp == nullptr ) {
    error() << "Couldn't read profile" << endmsg;
  } else {
    info() << tp->GetName() << ": " << tp->GetEntries() << endmsg;
  }

  // Tree with branches in "/trees/stuff"
  StatusCode sc5 = m_ths->regTree( "/read2/trees/stuff/treename" );
  TTree* tr = m_ths->getTree( "/read2/trees/stuff/treename" );
  if ( sc5.isFailure() || tr == nullptr ) {
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

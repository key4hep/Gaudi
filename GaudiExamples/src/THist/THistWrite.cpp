// Include files
#include "THistWrite.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/RndmGenerators.h"
#include <math.h>

#include "TDirectory.h"
#include "TError.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TKey.h"
#include "TProfile.h"
#include "TTree.h"

DECLARE_COMPONENT( THistWrite )

//------------------------------------------------------------------------------
THistWrite::THistWrite( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ), m_ths( 0 )
//------------------------------------------------------------------------------
{
  m_h1 = 0;
}

//------------------------------------------------------------------------------
StatusCode THistWrite::initialize()
//------------------------------------------------------------------------------
{

  if ( service( "THistSvc", m_ths ).isFailure() ) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Temporary Trees
  std::unique_ptr<TH1F> h1 = std::make_unique<TH1F>( "TempHist1", "Temporary Tree 1", 100, 0., 100. );
  if ( m_ths->regHist( "TempHist1", std::move(h1) ).isFailure() ) {
    error() << "Couldn't register TempHist1" << endmsg;
  }

  std::unique_ptr<TH1F> h1a = std::make_unique<TH1F>( "TempHist1a", "Temporary Tree 1a", 100, 0., 100. );
  if ( m_ths->regHist( "other/TempHist1a", std::move(h1a) ).isFailure() ) {
    error() << "Couldn't register TempHist1a" << endmsg;
  }

  // Write to stream "new"
  std::unique_ptr<TH1F> h2 = std::make_unique<TH1F>( "Tree2", "Tree 2", 100, 0., 100. );
  if ( m_ths->regHist( "/new/Tree2", std::move(h2) ).isFailure() ) {
    error() << "Couldn't register Tree2" << endmsg;
  }

  // Update to stream "upd", dir "/xxx"
  std::unique_ptr<TH1F> h3 = std::make_unique<TH1F>( "1Dgauss", "1D Gaussian", 100, -50., 50. );
  if ( m_ths->regHist( "/upd/xxx/gauss1d", std::move(h3) ).isFailure() ) {
    error() << "Couldn't register gauss1d" << endmsg;
  }

  // Recreate 2D tree in "/"
  std::unique_ptr<TH2F> h3a = std::make_unique<TH2F>( "2Dgauss", "2D Gaussian", 100, -50., 50., 100, -50, 50 );
  if ( m_ths->regHist( "/rec/gauss2d", std::move(h3a) ).isFailure() ) {
    error() << "Couldn't register gauss2d" << endmsg;
  }

  // 3D tree in "/"
  std::unique_ptr<TH3F> h4 = std::make_unique<TH3F>( "3Dgauss", "3D Gaussian", 100, -50., 50., 100, -50, 50, 100, -50, 50 );
  if ( m_ths->regHist( "/rec/gauss3d", std::move(h4) ).isFailure() ) {
    error() << "Couldn't register gauss3d" << endmsg;
  }

  // Profile in "/"
  std::unique_ptr<TH1> tp = std::make_unique<TProfile>( "profile", "profile", 100, -50., -50. );
  if ( m_ths->regHist( "/rec/prof", std::move(tp) ).isFailure() ) {
    error() << "Couldn't register prof" << endmsg;
  }

  // Tree with branches in "/trees/stuff"
  std::unique_ptr<TTree> tr = std::make_unique<TTree>( "treename", "tree title" );
  if ( m_ths->regTree( "/rec/trees/stuff/tree1", std::move(tr)).isFailure() ) {
    error() << "Couldn't register tr" << endmsg;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode THistWrite::execute()
//------------------------------------------------------------------------------
{
  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0., 15. ) );

  static int n = 0;

  double x = sin( double( n ) ) * 52. + 50.;

  TH1* h( 0 );
  h = m_ths->getHist( "TempHist1" );
  if ( h != nullptr ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 1" << endmsg;
  }

  h = m_ths->getHist( "other/TempHist1a" );
  if ( h != nullptr ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 1a" << endmsg;
  }

  h = m_ths->getHist( "/new/Tree2" );
  if ( h != nullptr ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve Tree2" << endmsg;
  }

  h = m_ths->getHist( "/upd/xxx/gauss1d" );
  if ( h != nullptr ) {
    for ( int i = 0; i < 1000; ++i ) {
      h->Fill( gauss(), 1. );
    }
  } else {
    error() << "Couldn't retrieve 1Dgauss" << endmsg;
  }

  TH2* h2( 0 );
  h2 = m_ths->getHistAsTH2( "/rec/gauss2d" );
  if ( h2 != nullptr ) {
    for ( int i = 0; i < 1000; ++i ) {
      h2->Fill( gauss(), gauss(), 1. );
    }
  } else {
    error() << "Couldn't retrieve 2Dgauss" << endmsg;
  }

  TH3* h3( 0 );
  h3 = m_ths->getHistAsTH3( "/rec/gauss3d" );
  if ( h3 != nullptr ) {
    for ( int i = 0; i < 1000; ++i ) {
      h3->Fill( gauss(), gauss(), gauss(), 1. );
    }
  } else {
    error() << "Couldn't retrieve 3Dgauss" << endmsg;
  }

  TTree* tr;
  tr = m_ths->getTree( "/rec/trees/stuff/tree1" );
  if ( tr == nullptr ) {
    error() << "Couldn't retrieve tree tree1" << endmsg;
  } else {
    if ( n == 0 ) {
      int p1, p2, p3;
      tr->Branch( "branch1", &p1, "point1/I" );
      tr->Branch( "branch2", &p2, "point2/I" );
      tr->Branch( "branch3", &p3, "point3/I" );

      for ( int i = 0; i < 1000; i++ ) {
        p1 = i;
        p2 = i % 10;
        p3 = i % 7;

        tr->Fill();
      }
    }
  }

  n++;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
void THistWrite::listKeys( TDirectory* td )
{
  //------------------------------------------------------------------------------

  info() << "printing keys for: " << td->GetPath() << "  (" << td->GetList()->GetSize() << ")" << endmsg;
  TIter nextkey( td->GetList() );
  while ( TKey* key = (TKey*)nextkey() ) {
    if ( key != 0 ) {
      info() << key->GetName() << " (" << key->IsA()->GetName() << ") "
             // 	  << key->GetObjectStat()
             // 	  << "  " << key->IsOnHeap()
             << key->GetCycle() << endmsg;
    } else {
      info() << "key == 0" << endmsg;
    }
  }

  return;
}

//------------------------------------------------------------------------------
StatusCode THistWrite::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}

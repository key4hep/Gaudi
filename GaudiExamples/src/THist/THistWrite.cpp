// Include files
#include "THistWrite.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/LockedHandle.h"
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
THistWrite::THistWrite( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator )
    , m_ths( nullptr )
//------------------------------------------------------------------------------
{}

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
  if ( m_ths->regHist( "TempHist1", std::move( h1 ) ).isFailure() ) {
    error() << "Couldn't register TempHist1" << endmsg;
  }

  std::unique_ptr<TH1F> h1a = std::make_unique<TH1F>( "TempHist1a", "Temporary Tree 1a", 100, 0., 100. );
  if ( m_ths->regHist( "other/TempHist1a", std::move( h1a ) ).isFailure() ) {
    error() << "Couldn't register TempHist1a" << endmsg;
  }

  // Write to stream "new"
  std::unique_ptr<TH1F> h2 = std::make_unique<TH1F>( "Tree2", "Tree 2", 100, 0., 100. );
  if ( m_ths->regHist( "/new/Tree2", std::move( h2 ) ).isFailure() ) { error() << "Couldn't register Tree2" << endmsg; }

  // Update to stream "upd", dir "/xxx"
  std::unique_ptr<TH1F> h3 = std::make_unique<TH1F>( "1Dgauss", "1D Gaussian", 100, -50., 50. );
  if ( m_ths->regHist( "/upd/xxx/gauss1d", std::move( h3 ) ).isFailure() ) {
    error() << "Couldn't register gauss1d" << endmsg;
  }

  // Recreate 2D tree in "/"
  std::unique_ptr<TH2F> h3a = std::make_unique<TH2F>( "2Dgauss", "2D Gaussian", 100, -50., 50., 100, -50, 50 );
  if ( m_ths->regHist( "/rec/gauss2d", std::move( h3a ) ).isFailure() ) {
    error() << "Couldn't register gauss2d" << endmsg;
  }

  // 3D tree in "/"
  std::unique_ptr<TH3F> h4 =
      std::make_unique<TH3F>( "3Dgauss", "3D Gaussian", 100, -50., 50., 100, -50, 50, 100, -50, 50 );
  if ( m_ths->regHist( "/rec/gauss3d", std::move( h4 ) ).isFailure() ) {
    error() << "Couldn't register gauss3d" << endmsg;
  }

  TH1F* h5 = new TH1F( "TempHist5", "Temporary Tree 5", 100, 0., 100. );
  if ( m_ths->regHist( "TempHist5", std::unique_ptr<TH1F>( h5 ) ).isFailure() ) {
    error() << "Couldn't register TempHist5" << endmsg;
  }
  if ( strcmp( h5->GetName(), "TempHist5" ) ) {
    error() << "Couldn't use TempHist5 afterwards. getName = " << h5->GetName() << endmsg;
  }

  TH1D* h6 = new TH1D( "TempHist6", "Temporary Tree 6", 100, 0., 100. );
  if ( m_ths->regHist( "TempHist6", std::unique_ptr<TH1D>( h6 ), h6 ).isFailure() ) {
    error() << "Couldn't register TempHist6" << endmsg;
  }
  if ( strcmp( h6->GetName(), "TempHist6" ) ) {
    error() << "Couldn't use TempHist6 afterwards. getName = " << h6->GetName() << endmsg;
  }

  TH1D* h7 = new TH1D( "TempHist7", "Temporary Tree 7", 100, 0., 100. );
  if ( m_ths->regHist( "TempHist7", h7 ).isFailure() ) { error() << "Couldn't register TempHist7" << endmsg; }
  if ( strcmp( h7->GetName(), "TempHist7" ) ) {
    error() << "Couldn't use TempHist7 afterwards. getName = " << h7->GetName() << endmsg;
  }

  // Profile in "/"
  std::unique_ptr<TH1> tp = std::make_unique<TProfile>( "profile", "profile", 100, -50., -50. );
  if ( m_ths->regHist( "/rec/prof", std::move( tp ) ).isFailure() ) { error() << "Couldn't register prof" << endmsg; }

  // Tree with branches in "/trees/stuff"
  std::unique_ptr<TTree> tr = std::make_unique<TTree>( "treename", "tree title" );
  if ( m_ths->regTree( "/rec/trees/stuff/tree1", std::move( tr ) ).isFailure() ) {
    error() << "Couldn't register tr" << endmsg;
  }

  // Update to stream "upd", dir "/xxx"
  std::unique_ptr<TH1F> h3s = std::make_unique<TH1F>( "1Dgauss_shared", "1D Gaussian", 100, -50., 50. );
  LockedHandle<TH1>     lh1( nullptr, nullptr );
  if ( m_ths->regShared( "/upd/xxx/gauss1d_shared", std::move( h3s ), lh1 ).isFailure() ) {
    error() << "Couldn't register gauss1d_shared" << endmsg;
  }

  // Recreate 2D tree in "/"
  std::unique_ptr<TH2F> h3sa = std::make_unique<TH2F>( "2Dgauss_shared", "2D Gaussian", 100, -50., 50., 100, -50, 50 );
  LockedHandle<TH2>     lh2( nullptr, nullptr );
  if ( m_ths->regShared( "/rec/gauss2d_shared", std::move( h3sa ), lh2 ).isFailure() ) {
    error() << "Couldn't register gauss2d_shared" << endmsg;
  }

  // 3D tree in "/"
  std::unique_ptr<TH3F> h4s =
      std::make_unique<TH3F>( "3Dgauss_shared", "3D Gaussian", 100, -50., 50., 100, -50, 50, 100, -50, 50 );
  LockedHandle<TH3> lh3( nullptr, nullptr );
  if ( m_ths->regShared( "/rec/gauss3d_shared", std::move( h4s ), lh3 ).isFailure() ) {
    error() << "Couldn't register gauss3d_shared" << endmsg;
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

  TH1* h( nullptr );
  if ( m_ths->getHist( "TempHist1", h ).isSuccess() ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 1" << endmsg;
  }

  if ( m_ths->getHist( "other/TempHist1a", h ).isSuccess() ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 1a" << endmsg;
  }

  if ( m_ths->getHist( "/new/Tree2", h ).isSuccess() ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve Tree2" << endmsg;
  }

  if ( m_ths->getHist( "/upd/xxx/gauss1d", h ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { h->Fill( gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 1Dgauss" << endmsg;
  }

  TH2* h2( nullptr );
  if ( m_ths->getHist( "/rec/gauss2d", h2 ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { h2->Fill( gauss(), gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 2Dgauss" << endmsg;
  }

  TH3* h3( nullptr );
  if ( m_ths->getHist( "/rec/gauss3d", h3 ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { h3->Fill( gauss(), gauss(), gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 3Dgauss" << endmsg;
  }

  if ( m_ths->getHist( "TempHist5", h ).isSuccess() ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 5" << endmsg;
  }

  if ( m_ths->getHist( "TempHist6", h ).isSuccess() ) {
    h->Fill( x );
  } else {
    error() << "Couldn't retrieve TempHist 6" << endmsg;
  }

  LockedHandle<TH1> lh1( nullptr, nullptr );
  if ( m_ths->getShared( "/upd/xxx/gauss1d_shared", lh1 ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { lh1->Fill( gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 1Dgauss_shared" << endmsg;
  }

  LockedHandle<TH2> lh2( nullptr, nullptr );
  if ( m_ths->getShared( "/rec/gauss2d_shared", lh2 ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { lh2->Fill( gauss(), gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 2Dgauss_shared" << endmsg;
  }

  LockedHandle<TH3> lh3( nullptr, nullptr );
  if ( m_ths->getShared( "/rec/gauss3d_shared", lh3 ).isSuccess() ) {
    for ( int i = 0; i < 1000; ++i ) { lh3->Fill( gauss(), gauss(), gauss(), 1. ); }
  } else {
    error() << "Couldn't retrieve 3Dgauss_shared" << endmsg;
  }

  TTree* tr( nullptr );
  if ( m_ths->getTree( "/rec/trees/stuff/tree1", tr ).isFailure() ) {
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
void THistWrite::listKeys( TDirectory* td ) {
  //------------------------------------------------------------------------------

  info() << "printing keys for: " << td->GetPath() << "  (" << td->GetList()->GetSize() << ")" << endmsg;
  TIter nextkey( td->GetList() );
  while ( TKey* key = (TKey*)nextkey() ) {
    if ( key != 0 ) {
      info() << key->GetName() << " (" << key->IsA()->GetName()
             << ") "
             // << key->GetObjectStat()
             // << "  " << key->IsOnHeap()
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

  if ( m_ths->deReg( "/temp/TempHist6" ).isFailure() ) {
    error() << "Failed to deregister histogram TempHist6" << endmsg;
  }

  TH1* h7 = nullptr;
  if ( m_ths->getHist( "TempHist7", h7 ).isFailure() ) { error() << "Couldn't retrieve TempHist7" << endmsg; }
  if ( m_ths->deReg( h7 ).isFailure() ) { error() << "Failed to deregister histogram TempHist7" << endmsg; }

  return StatusCode::SUCCESS;
}

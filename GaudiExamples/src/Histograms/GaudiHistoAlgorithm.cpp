// Include files
#include "boost/assign/list_of.hpp"

// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"

// local
#include "GaudiHistoAlgorithm.h"

// onl needed for old style histogram for speed comparision
#include "AIDA/IHistogram1D.h"

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiHistoAlgorithm
//
// 2005-08-12 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT(GaudiHistoAlgorithm)


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GaudiHistoAlgorithm::GaudiHistoAlgorithm( const std::string& name,
                                          ISvcLocator* pSvcLocator)
  : GaudiHistoAlg ( name , pSvcLocator )
{
}
//=============================================================================
// Destructor
//=============================================================================
GaudiHistoAlgorithm::~GaudiHistoAlgorithm() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode GaudiHistoAlgorithm::initialize()
{
  // must be called first
  const StatusCode sc = GaudiHistoAlg::initialize();
  if ( sc.isFailure() ) return sc;

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode GaudiHistoAlgorithm::execute()
{

  // count calls
  static int nCalls(0);

  // some random number generators, just to provide numbers
  static Rndm::Numbers  Gauss   ( randSvc() , Rndm::Gauss       (   0.0 ,  1.0 ) ) ;
  static Rndm::Numbers  Flat    ( randSvc() , Rndm::Flat        ( -10.0 , 10.0 ) ) ;
  static Rndm::Numbers  Expo    ( randSvc() , Rndm::Exponential (   1.0        ) ) ;
  static Rndm::Numbers  Breit   ( randSvc() , Rndm::BreitWigner (   0.0 ,  1.0 ) ) ;
  static Rndm::Numbers  Poisson ( randSvc() , Rndm::Poisson     (   2.0        ) ) ;
  static Rndm::Numbers  Binom   ( randSvc() , Rndm::Binomial    (   8   , 0.25 ) ) ;

  // cache some numbers
  const double gauss   ( Gauss()   );
  const double flat    ( Flat()    );
  const double expo    ( Expo()    );
  const double breit   ( Breit()   );
  const double poisson ( Poisson() );
  const double binom   ( Binom()   );

  // =============== Histogramming Examples =================================

  // 1D plots with auto ID
  plot1D( gauss, "Gaussian mean=0, sigma=1",  -5,  5, 100 );

  // 1D plot with forced numerical ID
  plot1D( expo,  101, "Exponential",  0, 5, 100 );
  plot1D( breit, 102, "Breit",      -5, 5, 100 );

  // 1D plot with forced alpha-numeric ID
  plot1D( poisson, "poisson", "Poisson",   -5, 5, 100 );

  // 1D plot with forced alpha-numeric ID in a sub directory
  plot1D( binom,   "subdir1/bino",   "Binominal", -5, 5, 100 );
  plot1D( binom,   "subdir2/bino",   "Binominal", -5, 5, 100 );

  // 2D plots with auto ID
  plot2D( flat, gauss,    "Gaussian V Flat", -5, 5, -5, 5 );
  plot2D( flat, expo,    "Exponential V Flat", -5, 5, 0, 5 );

  // 2D with  forced alpha-numeric ID
  plot2D( poisson, binom, "binVpois",  "Binom V poisson",  -5,  5, -5, 5 );
  plot2D( poisson, expo,  "expoVpois", "Expo V poisson",  -5,  5, -5, 5 );

  // 3D plot with auto ID
  plot3D( gauss, expo, breit,       "3D plot AutoID",   -5,5, 0,5, -5,5 );

  // 3D plot with forced alpha-numeric ID
  plot3D( gauss, expo, breit, "3d", "3D plot ForcedID", -5,5, 0,5, -5,5 );

  // 1D profile histo with auto ID
  profile1D( gauss , expo  , "Expo V Gauss 1DProf"  ,  -5 ,  5 , 50       ) ;
  profile1D( gauss , expo  , "Expo V Gauss 1DProf s",  -5 ,  5 , 50 , "s" ) ;
  profile1D( flat  , gauss , "Gauss V Flat 1DProf"  , -10 , 10 , 10       ) ;
  profile1D( flat  , gauss , "Gauss V Flat 1DProf S", -10 , 10 , 10 , "s" ) ;
  profile1D( flat  , gauss , "Gauss V Flat 1DProf, with limits-I"   ,
             -10 , 10 , 10 , ""  ,  0 , 5 );
  profile1D( flat  , gauss , "Gauss V Flat 1DProf, with limits-I  s",
             -10 , 10 , 10 , "s" ,  0 , 5 );
  profile1D( flat  , gauss , "Gauss V Flat 1DProf, with limits-II"  ,
             -10 , 10 , 10 , ""  , -5 , 0 );
  profile1D( flat  , gauss , "Gauss V Flat 1DProf, with limits-II s",
             -10 , 10 , 10 , "s" , -5 , 0 );

  // 2D profile histo with forced ID
  profile2D( gauss, expo, poisson, "2dprof", "2D profile1", -5, 5, 0, 5 );
  profile2D( gauss, expo, poisson, 321, "2D profile2", -5, 5, 0, 5 );

  // variable binning
  using namespace boost::assign;
  const GaudiAlg::HistoBinEdges edgesX = list_of<double>(-5)(-4)(-2.5)(0)(1)(2.25)(4)(5);
  const GaudiAlg::HistoBinEdges edgesY = list_of<double>(-5)(-3.7)(-2)(0.5)(1)(2)(4.5)(5);
  const GaudiAlg::HistoBinEdges edgesZ = list_of<double>(-5)(-3)(0)(5);

  // 1D
  plot1D( flat, "varBinning/x", "1D Variable Binning", edgesX );
  // 2D
  plot2D( flat, gauss, "varBinning/y", "2D Variable Binning", edgesX, edgesY );
  // 3D
  plot3D( flat, gauss, expo, "varBinning/z", "3D Variable Binning", edgesX, edgesY, edgesZ );
  // 1D profile
  profile1D( flat, gauss,
             "varBinning/a", "1D Profile Variable Binning", edgesX );
  // 2D Profile
  profile2D( flat, gauss, expo, "varBinning/b", "2D Profile Variable Binning", edgesX, edgesY );

  // ============================================================================
  // The following is just a comparision of fill times for the various 1D methods
  // to illustrate that the shortcut methods are not significantly slower
  // than directly interacting with the histoSvc (At least on linux. On windows
  // there is a difference, due either to the implementation or the
  // non-optimised build)
  // ===========================================================================

  // old style filling
  static IHistogram1D* test(0);
  if (0==nCalls) { test = histoSvc()->book( "OldStyle/1112", "Old Style Histo", 100, -5, 5 ); }
  if (nCalls>0) chronoSvc()->chronoStart("1DOldStyle") ;
  test->fill(gauss);
  if (nCalls>0) chronoSvc()->chronoStop ("1DOldStyle") ;

  // timing test for histo fill with autoID (skip first booking call)
  if (nCalls>0) chronoSvc()->chronoStart("1DAutoID") ;
  plot1D( gauss, "AutoID time test", -5,  5, 100 );
  if (nCalls>0) chronoSvc()->chronoStop ("1DAutoID") ;

  // timing test for histo fill with forced numeric ID (skip first booking call)
  if (nCalls>0) chronoSvc()->chronoStart("1DForcedNumericID") ;
  plot1D( gauss, 1111, "Forced Numeric ID time test", -5,  5, 100 );
  if (nCalls>0) chronoSvc()->chronoStop ("1DForcedNumericID") ;

  // timing test for histo fill with forced alpha ID (skip first booking call)
  if (nCalls>0) chronoSvc()->chronoStart("1DForcedAlphaID") ;
  plot1D( gauss, "test1", "Forced Alpha ID time test", -5,  5, 100 );
  if (nCalls>0) chronoSvc()->chronoStop ("1DForcedAlphaID") ;

  if (0==nCalls) Print( "Filling Histograms...... Please be patient !" );
  ++nCalls;
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode GaudiHistoAlgorithm::finalize()
{
  // must be called after all other actions
  return GaudiHistoAlg::finalize();
}

//=============================================================================

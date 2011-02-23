// $Id: Aida2Root.cpp,v 1.6 2008/01/17 15:06:55 marcocle Exp $

#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning(disable:2259)
#endif

// ============================================================================
// Include files 
// ============================================================================
// ROOT 
// ============================================================================
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TProfile2D.h"
// ============================================================================
// AIDA 
// ============================================================================
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiUtils/HistoStats.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp" 
// ============================================================================
/** @class Aida2Root
 *
 *  simple example to illustrate the usage of class Gaudi::Utils::Aida2ROOT
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2007-01-24
 */
class Aida2Root : public GaudiHistoAlg
{
public:
  /// execution of the algorithm 
  StatusCode execute  () { return StatusCode::SUCCESS ; };
  /// finalization of the algorithm 
  StatusCode finalize () ;
public:
  // standard constructor
  Aida2Root 
  ( const std::string& name , 
    ISvcLocator*       pSvc ) 
    : GaudiHistoAlg ( name , pSvc )
    //
    , m_1Ds ()
    , m_2Ds ()
    , m_3Ds ()
    , m_1Ps ()
    , m_2Ps ()
  {
    m_1Ds.push_back ( "SimpleHistos/Gaussian mean=0, sigma=1"            ) ;
    m_1Ds.push_back ( "SimpleHistos/101"          ) ;
    m_1Ds.push_back ( "SimpleHistos/102"          ) ;
    m_1Ds.push_back ( "SimpleHistos/1111"         ) ;
    m_1Ds.push_back ( "SimpleHistos/test1"        ) ;
    m_1Ds.push_back ( "SimpleHistos/subdir2/bino" ) ;
    m_1Ds.push_back ( "SimpleHistos/subdir1/bino" ) ;
    m_1Ds.push_back ( "SimpleHistos/poisson"      ) ;
    declareProperty ( "Histos1D" , m_1Ds          ) ;
    //
    m_2Ds.push_back ( "SimpleHistos/Gaussian V Flat"            ) ;
    m_2Ds.push_back ( "SimpleHistos/Exponential V Flat"            ) ;
    m_2Ds.push_back ( "SimpleHistos/binVpois"     ) ;
    m_2Ds.push_back ( "SimpleHistos/expoVpois"    ) ;
    declareProperty ( "Histos2D" , m_2Ds          ) ;
    //
    m_3Ds.push_back ( "SimpleHistos/3D plot AutoID"            ) ;
    m_3Ds.push_back ( "SimpleHistos/3d"           ) ;
    declareProperty ( "Histos3D" , m_3Ds          ) ;
    //
    m_1Ps.push_back ( "SimpleHistos/Expo V Gauss 1DProf"            ) ;
    declareProperty ( "Profs1D"  , m_1Ps          ) ;
    //
    m_2Ps.push_back ( "SimpleHistos/321"          ) ;
    m_2Ps.push_back ( "SimpleHistos/2dprof"       ) ;
    declareProperty ( "Profs2D"  , m_2Ps ) ;
    // 
    setProperty     ( "PropertiesPrint" , "True"  ).ignore() ;
  }
  /// virtual destructor 
  virtual ~Aida2Root() {}
private:
  // the default constructor is disabled 
  Aida2Root() ;
  // the default constructor is disabled 
  Aida2Root( const Aida2Root& ) ;
  // the assignement  is disabled
  Aida2Root& operator=( const Aida2Root& ) ;
private:
  typedef std::vector<std::string> List ;
  // the list of 1D-histograms 
  List m_1Ds ; ///< the list of 1D-histograms 
  // the list of 2D-histograms 
  List m_2Ds ; ///< the list of 2D-histograms 
  // the list of 3D-histograms 
  List m_3Ds ; ///< the list of 3D-histograms 
  // the list of 1D-profiles
  List m_1Ps ; ///< the list of 1D-profiles 
  // the list of 2D-profiles 
  List m_2Ps ; ///< the list of 2D-profiles  
} ;
// ============================================================================
/// Declaration of the Algorithm Factory
// ============================================================================
DECLARE_ALGORITHM_FACTORY(Aida2Root)
// ============================================================================
namespace 
{
  inline
  std::string print ( const double       aida   , 
                      const double       root   , 
                      const std::string& name   ,
                      const std::string& format ) 
  {
    boost::format fmt ( format ) ;
    fmt % name % aida % root % ( aida - root ) ;
    return fmt.str() ;
  }
}
// ============================================================================
/// finalize the algorithm 
// ============================================================================
StatusCode Aida2Root::finalize() 
{
  
  always() << "Get the native ROOT representation of histograms!" << endmsg ;
  
  {  // loop over all 1D-histograms
    for ( List::const_iterator ipath = m_1Ds.begin() ; 
          m_1Ds.end() != ipath ; ++ipath ) 
    {
      /// retrieve the historam by full path:
      AIDA::IHistogram1D* aida = 0 ;
      StatusCode sc = histoSvc()->retrieveObject( *ipath , aida ) ;
      if ( sc.isFailure() || 0 == aida ) 
      { return Error ( "Unable to retrieve 1D-histogram '" + (*ipath) + "'"  ) ; }
      /// convert it to ROOT 
      TH1D* root = Gaudi::Utils::Aida2ROOT::aida2root ( aida ) ;
      if ( 0 == root ) 
      { return Error ( "Unable to convert to ROOT the 1D-histogram '"+(*ipath)+"'") ; }
      /// use the native printout from ROOT 
      info() << "The native ROOT printout for 1D-histogram '" << (*ipath) << "':" << endmsg ;
      root->Print() ;  

      info () << " |  Compare       | AIDA/HistoStats |     ROOT/TH1    |      Delta      | "  << endmsg ;      
      const std::string format = " | %1$-14.14s | %2$ 15.8g | %3$- 15.8g | %4$= 15.8g | "  ;
      info () << print 
        ( Gaudi::Utils::HistoStats::mean        ( aida ) , 
          root->GetMean      ()    , "'mean'"        , format  ) << endmsg ;
      info () << print 
        ( Gaudi::Utils::HistoStats::meanErr     ( aida ) , 
          root->GetMeanError ()    , "'meanErr'"     , format  ) << endmsg ;
      info () << print 
        ( Gaudi::Utils::HistoStats::rms         ( aida ) , 
          root->GetRMS       ()    , "'rms'"         , format  ) << endmsg ;
      info () << print 
        ( Gaudi::Utils::HistoStats::rmsErr      ( aida ) , 
          root->GetRMSError  ()    , "'rmsErr'"      , format  ) << endmsg ;
      info () << print 
        ( Gaudi::Utils::HistoStats::skewness    ( aida ) , 
          root->GetSkewness ()     , "'skewness'"    , format  ) << endmsg ;      
      info () << print 
        ( Gaudi::Utils::HistoStats::skewnessErr ( aida ) , 
          root->GetSkewness ( 11 ) , "'skewnessErr'" , format  ) << endmsg ;      
      info () << print 
        ( Gaudi::Utils::HistoStats::kurtosis  ( aida ) , 
          root->GetKurtosis ()     , "'kurtosis'"    , format  ) << endmsg ;
      info () << print 
        ( Gaudi::Utils::HistoStats::kurtosisErr ( aida ) , 
          root->GetKurtosis ( 11 ) , "'kurtosisErr'" , format  ) << endmsg ;
    }
  }

  { // loop over all 2D-histograms
    for ( List::const_iterator ipath = m_2Ds.begin() ; 
          m_2Ds.end() != ipath ; ++ipath ) 
    {
      /// retrieve the historam by full path:
      AIDA::IHistogram2D* aida = 0 ;
      StatusCode sc = histoSvc()->retrieveObject( *ipath , aida ) ;
      if ( sc.isFailure() || 0 == aida ) 
      { return Error ( "Unable to retrieve 2D-histogram '" + (*ipath) + "'"  ) ; }
      /// convert it to ROOT 
      TH2D* root = Gaudi::Utils::Aida2ROOT::aida2root ( aida ) ;
      if ( 0 == root ) 
      { return Error ( "Unable to convert to ROOT the 2D-histogram '"+(*ipath)+"'") ; }
      /// use the native printout from ROOT 
      info() << "The native ROOT printout for 2D-histogram '" << (*ipath) << "':" << endmsg ;
      root->Print() ;  
    }
  }
  
  { // loop over all 3D-histograms
    for ( List::const_iterator ipath = m_3Ds.begin() ; 
          m_3Ds.end() != ipath ; ++ipath ) 
    {
      /// retrieve the historam by full path:
      AIDA::IHistogram3D* aida = 0 ;
      StatusCode sc = histoSvc()->retrieveObject( *ipath , aida ) ;
      if ( sc.isFailure() || 0 == aida ) 
      { return Error ( "Unable to retrieve 3D-histogram '" + (*ipath) + "'"  ) ; }
      /// convert it to ROOT 
	TH3D* root = Gaudi::Utils::Aida2ROOT::aida2root ( aida ) ;
      if ( 0 == root ) 
      { return Error ( "Unable to convert to ROOT the 3D-histogram '"+(*ipath)+"'") ; }
      /// use the native printout from ROOT 
      info() << "The native ROOT printout for 3D-histogram '" << (*ipath) << "':" << endmsg ;
      root->Print() ;  
    }
  }
  

  { // loop over all 1D-profiles 
    for ( List::const_iterator ipath = m_1Ps.begin() ; 
          m_1Ps.end() != ipath ; ++ipath ) 
    {
      /// retrieve the historam by full path:
      AIDA::IProfile1D* aida = 0 ;
      StatusCode sc = histoSvc()->retrieveObject( *ipath , aida ) ;
      if ( sc.isFailure() || 0 == aida ) 
      { return Error ( "Unable to retrieve 1D-profile '" + (*ipath) + "'"  ) ; }
      /// convert it to ROOT 
	TProfile* root = Gaudi::Utils::Aida2ROOT::aida2root ( aida ) ;
      if ( 0 == root ) 
      { return Error ( "Unable to convert to ROOT the 1D-profile '"+(*ipath)+"'") ; }
      /// use the native printout from ROOT 
      info() << "The native ROOT printout for 1D-profile '" << (*ipath) << "':" << endmsg ;
      root->Print() ;  
    }
  }
  
  
  { // loop over all 2D-profiles 
    for ( List::const_iterator ipath = m_2Ps.begin() ; 
          m_2Ps.end() != ipath ; ++ipath ) 
    {
      /// retrieve the historam by full path:
      AIDA::IProfile2D* aida = 0 ;
      StatusCode sc = histoSvc()->retrieveObject( *ipath , aida ) ;
      if ( sc.isFailure() || 0 == aida ) 
      { Error ( "Unable to retrieve 2D-profile '" + (*ipath) + "'"  ) ; }
      /// convert it to ROOT 
	TProfile2D* root = Gaudi::Utils::Aida2ROOT::aida2root ( aida ) ;
      if ( 0 == root ) 
      { Error ( "Unable to convert to ROOT the 2D-profile '"+(*ipath)+"'") ; }
      /// use the native printout from ROOT 
      info() << "The native ROOT printout for 2D-profile '" << (*ipath) << "':" << endmsg ;
      root->Print() ;  
    }
  }
  
  return GaudiHistoAlg::finalize() ;
}


// ============================================================================
// The END 
// ============================================================================


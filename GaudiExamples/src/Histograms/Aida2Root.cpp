#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning( disable : 2259 )
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
  StatusCode execute() override { return StatusCode::SUCCESS; };
  /// finalization of the algorithm
  StatusCode finalize() override;

public:
  // standard constructor
  Aida2Root( const std::string& name, ISvcLocator* pSvc ) : GaudiHistoAlg( name, pSvc )
  {
    setProperty( "PropertiesPrint", "True" ).ignore();
  }
  /// virtual destructor
  ~Aida2Root() override = default;

private:
  Gaudi::Property<std::vector<std::string>> m_1Ds{
      this,
      "Histos1D",
      {"SimpleHistos/Gaussian mean=0, sigma=1", "SimpleHistos/101", "SimpleHistos/102", "SimpleHistos/1111",
       "SimpleHistos/test1", "SimpleHistos/subdir2/bino", "SimpleHistos/subdir1/bino", "SimpleHistos/poisson"},
      "list of 1D-histograms"};

  Gaudi::Property<std::vector<std::string>> m_2Ds{this,
                                                  "Histos2D",
                                                  {"SimpleHistos/Gaussian V Flat", "SimpleHistos/Exponential V Flat",
                                                   "SimpleHistos/binVpois", "SimpleHistos/expoVpois"},
                                                  "list of 2D-histograms"};

  Gaudi::Property<std::vector<std::string>> m_3Ds{
      this, "Histos3D", {"SimpleHistos/3D plot AutoID", "SimpleHistos/3d"}, "list of 3D-histograms"};

  Gaudi::Property<std::vector<std::string>> m_1Ps{
      this, "Profs1D", {"SimpleHistos/Expo V Gauss 1DProf"}, "list of 1D-profiles"};

  Gaudi::Property<std::vector<std::string>> m_2Ps{
      this, "Profs2D", {"SimpleHistos/321", "SimpleHistos/2dprof"}, "list of 2D-profiles"};
};
// ============================================================================
/// Declaration of the Algorithm Factory
// ============================================================================
DECLARE_COMPONENT( Aida2Root )
// ============================================================================
namespace
{
  inline std::string print( const double aida, const double root, const std::string& name, const std::string& format )
  {
    boost::format fmt( format );
    fmt % name % aida % root % ( aida - root );
    return fmt.str();
  }
}
// ============================================================================
/// finalize the algorithm
// ============================================================================
StatusCode Aida2Root::finalize()
{

  always() << "Get the native ROOT representation of histograms!" << endmsg;

  { // loop over all 1D-histograms
    for ( auto& path : m_1Ds ) {
      /// retrieve the historam by full path:
      AIDA::IHistogram1D* aida = 0;
      StatusCode          sc   = histoSvc()->retrieveObject( path, aida );
      if ( sc.isFailure() || 0 == aida ) {
        return Error( "Unable to retrieve 1D-histogram '" + ( path ) + "'" );
      }
      /// convert it to ROOT
      TH1D* root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
      if ( 0 == root ) {
        return Error( "Unable to convert to ROOT the 1D-histogram '" + ( path ) + "'" );
      }
      /// use the native printout from ROOT
      info() << "The native ROOT printout for 1D-histogram '" << ( path ) << "':" << endmsg;
      root->Print();

      info() << " |  Compare       | AIDA/HistoStats |     ROOT/TH1    |      Delta      | " << endmsg;
      const std::string format = " | %1$-14.14s | %2$ 15.8g | %3$- 15.8g | %4$= 15.8g | ";
      info() << print( Gaudi::Utils::HistoStats::mean( aida ), root->GetMean(), "'mean'", format ) << endmsg;
      info() << print( Gaudi::Utils::HistoStats::meanErr( aida ), root->GetMeanError(), "'meanErr'", format ) << endmsg;
      info() << print( Gaudi::Utils::HistoStats::rms( aida ), root->GetRMS(), "'rms'", format ) << endmsg;
      info() << print( Gaudi::Utils::HistoStats::rmsErr( aida ), root->GetRMSError(), "'rmsErr'", format ) << endmsg;
      info() << print( Gaudi::Utils::HistoStats::skewness( aida ), root->GetSkewness(), "'skewness'", format )
             << endmsg;
      info() << print( Gaudi::Utils::HistoStats::skewnessErr( aida ), root->GetSkewness( 11 ), "'skewnessErr'", format )
             << endmsg;
      info() << print( Gaudi::Utils::HistoStats::kurtosis( aida ), root->GetKurtosis(), "'kurtosis'", format )
             << endmsg;
      info() << print( Gaudi::Utils::HistoStats::kurtosisErr( aida ), root->GetKurtosis( 11 ), "'kurtosisErr'", format )
             << endmsg;
    }
  }

  { // loop over all 2D-histograms
    for ( auto& path : m_2Ds ) {
      /// retrieve the historam by full path:
      AIDA::IHistogram2D* aida = 0;
      StatusCode          sc   = histoSvc()->retrieveObject( path, aida );
      if ( sc.isFailure() || 0 == aida ) {
        return Error( "Unable to retrieve 2D-histogram '" + ( path ) + "'" );
      }
      /// convert it to ROOT
      TH2D* root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
      if ( 0 == root ) {
        return Error( "Unable to convert to ROOT the 2D-histogram '" + ( path ) + "'" );
      }
      /// use the native printout from ROOT
      info() << "The native ROOT printout for 2D-histogram '" << ( path ) << "':" << endmsg;
      root->Print();
    }
  }

  { // loop over all 3D-histograms
    for ( auto& path : m_3Ds ) {
      /// retrieve the historam by full path:
      AIDA::IHistogram3D* aida = 0;
      StatusCode          sc   = histoSvc()->retrieveObject( path, aida );
      if ( sc.isFailure() || 0 == aida ) {
        return Error( "Unable to retrieve 3D-histogram '" + ( path ) + "'" );
      }
      /// convert it to ROOT
      TH3D* root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
      if ( 0 == root ) {
        return Error( "Unable to convert to ROOT the 3D-histogram '" + ( path ) + "'" );
      }
      /// use the native printout from ROOT
      info() << "The native ROOT printout for 3D-histogram '" << ( path ) << "':" << endmsg;
      root->Print();
    }
  }

  { // loop over all 1D-profiles
    for ( auto& path : m_1Ps ) {
      /// retrieve the historam by full path:
      AIDA::IProfile1D* aida = 0;
      StatusCode        sc   = histoSvc()->retrieveObject( path, aida );
      if ( sc.isFailure() || 0 == aida ) {
        return Error( "Unable to retrieve 1D-profile '" + ( path ) + "'" );
      }
      /// convert it to ROOT
      TProfile* root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
      if ( 0 == root ) {
        return Error( "Unable to convert to ROOT the 1D-profile '" + ( path ) + "'" );
      }
      /// use the native printout from ROOT
      info() << "The native ROOT printout for 1D-profile '" << ( path ) << "':" << endmsg;
      root->Print();
    }
  }

  { // loop over all 2D-profiles
    for ( auto& path : m_2Ps ) {
      /// retrieve the historam by full path:
      AIDA::IProfile2D* aida = 0;
      StatusCode        sc   = histoSvc()->retrieveObject( path, aida );
      if ( sc.isFailure() || 0 == aida ) {
        Error( "Unable to retrieve 2D-profile '" + ( path ) + "'" );
      }
      /// convert it to ROOT
      TProfile2D* root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
      if ( 0 == root ) {
        Error( "Unable to convert to ROOT the 2D-profile '" + ( path ) + "'" );
      }
      /// use the native printout from ROOT
      info() << "The native ROOT printout for 2D-profile '" << ( path ) << "':" << endmsg;
      root->Print();
    }
  }

  return GaudiHistoAlg::finalize();
}

// ============================================================================
// The END
// ============================================================================

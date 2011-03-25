// $Id: $
// Include files

//-----------------------------------------------------------------------------
// Implementation file for class : HistoLabels
//-----------------------------------------------------------------------------
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning(disable:4996)
#endif

// local
#include "GaudiUtils/HistoLabels.h"
#include "GaudiUtils/Aida2ROOT.h"

// ============================================================================
// ROOT
// ============================================================================
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TProfile2D.h"

// Private namespace
namespace
{
  //--------------------------------------------------------------------

  template <typename R, typename A>
  bool setAxisLabels_( A* aida,
                       const std::string& xAxis,
                       const std::string& yAxis )
  {
    if (!aida) return false;
    R * root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
    if (!root) return false;
    root->SetXTitle(xAxis.c_str());
    root->SetYTitle(yAxis.c_str());
    return true;
  }

  //--------------------------------------------------------------------

  bool setBinLabels_( TAxis* axis,
                      const Gaudi::Utils::Histos::BinLabels & labels )
  {
    if (!axis) return false;
    const unsigned nbins = axis->GetNbins();
    for ( Gaudi::Utils::Histos::BinLabels::const_iterator i = labels.begin();
         i != labels.end(); ++i )
    {
      if ( 1+i->first <= 0 || 1+i->first > nbins ) return false;
      // Argh... ROOT bins start counting at '1' instead of '0'
      axis -> SetBinLabel( 1 + i->first, i->second.c_str() );
    }
    return true;
  }

  //--------------------------------------------------------------------

  template <typename R, typename A>
  bool setBinLabels_( A* aida,
                      const Gaudi::Utils::Histos::BinLabels& labels )
  {
    if (!aida) return false;
    R * root = Gaudi::Utils::Aida2ROOT::aida2root( aida );
    if (!root) return false;
    return setBinLabels_( root->GetXaxis(), labels );
  }

  template <typename Histogram>
  bool setBinLabels_( Histogram* hist,
                      const Gaudi::Utils::Histos::Labels& labels )
  {
    Gaudi::Utils::Histos::BinLabels l;
    l.reserve(labels.size());
    for ( unsigned i = 0; i<labels.size(); ++i )
    {
      l.push_back( Gaudi::Utils::Histos::BinLabel(i,labels[i]) );
    }
    return Gaudi::Utils::Histos::setBinLabels(hist,l);
  }

  //--------------------------------------------------------------------

}

namespace Gaudi
{
  namespace Utils
  {
    namespace Histos
    {

  // --------------------------------------------------------------------------

      bool setBinLabels( AIDA::IHistogram1D* hist,
                         const BinLabels& labels )
      {
        return setBinLabels_<TH1D>(hist,labels);
      }

      bool setBinLabels( AIDA::IProfile1D* hist,
                         const BinLabels& labels )
      {
        return setBinLabels_<TProfile>(hist,labels);
      }

      bool setBinLabels( AIDA::IHistogram1D* hist,
                         const Labels& labels )
      {
        return setBinLabels_(hist,labels);
      }

      bool setBinLabels( AIDA::IProfile1D* hist,
                         const Labels& labels )
      {
        return setBinLabels_(hist,labels);
      }

      bool setBinLabels( AIDA::IHistogram2D* hist,
                         const Labels& xlabels,
                         const Labels& ylabels )
      {
        if (!hist) return false;
        TH2D * h2d = Gaudi::Utils::Aida2ROOT::aida2root( hist );
        if (!h2d)  return false;
        BinLabels lx;
        lx.reserve(xlabels.size());
        for ( unsigned int i = 0; i < xlabels.size(); ++i )
        {
          lx.push_back( Gaudi::Utils::Histos::BinLabel( i , xlabels[i] ) );
        }
        BinLabels ly;
        ly.reserve(ylabels.size());
        for ( unsigned int i = 0; i < ylabels.size(); ++i )
        {
          ly.push_back(Gaudi::Utils::Histos::BinLabel( i , ylabels[i] ) );
        }
        return ( setBinLabels_( h2d->GetXaxis(), lx ) &&
                 setBinLabels_( h2d->GetYaxis(), ly ) );
      }

      bool setBinLabels( AIDA::IHistogram2D* hist,
                         const BinLabels& xlabels,
                         const BinLabels& ylabels)
      {
        TH2D * h2d = Gaudi::Utils::Aida2ROOT::aida2root( hist );
        return ( h2d &&
                 setBinLabels_( h2d->GetXaxis(), xlabels ) &&
                 setBinLabels_( h2d->GetYaxis(), ylabels ) );
      }

      bool setBinLabels( AIDA::IProfile2D* hist,
                         const Labels& xlabels,
                         const Labels& ylabels )
      {
        if (!hist) return false;
        TProfile2D * h2d = Gaudi::Utils::Aida2ROOT::aida2root( hist );
        if (!h2d)  return false;
        BinLabels lx;
        lx.reserve(xlabels.size());
        for ( unsigned int i = 0; i < xlabels.size(); ++i )
        {
          lx.push_back(Gaudi::Utils::Histos::BinLabel( i , xlabels[i] ) );
        }
        BinLabels ly;
        ly.reserve(ylabels.size());
        for ( unsigned int i = 0; i < ylabels.size(); ++i )
        {
          ly.push_back(Gaudi::Utils::Histos::BinLabel( i , ylabels[i] ) );
        }
        return ( setBinLabels_( h2d->GetXaxis(), lx ) &&
                 setBinLabels_( h2d->GetYaxis(), ly ) );
      }

      bool setBinLabels( AIDA::IProfile2D* hist,
                         const BinLabels& xlabels,
                         const BinLabels& ylabels )
      {
        TProfile2D * h2d = Gaudi::Utils::Aida2ROOT::aida2root( hist );
        return ( h2d &&
                 setBinLabels_( h2d->GetXaxis(), xlabels ) &&
                 setBinLabels_( h2d->GetYaxis(), ylabels ) );
      }

      // --------------------------------------------------------------------------

      bool setAxisLabels( AIDA::IHistogram1D* hist,
                          const std::string & xAxis,
                          const std::string & yAxis  )
      {
        return setAxisLabels_<TH1D>( hist, xAxis, yAxis );
      }

      bool setAxisLabels( AIDA::IProfile1D* hist,
                          const std::string & xAxis,
                          const std::string & yAxis  )
      {
        return setAxisLabels_<TProfile>( hist, xAxis, yAxis );
      }

      bool setAxisLabels( AIDA::IHistogram2D* hist,
                          const std::string & xAxis,
                          const std::string & yAxis  )
      {
        return setAxisLabels_<TH2D>( hist, xAxis, yAxis );
      }

      bool setAxisLabels( AIDA::IProfile2D* hist,
                          const std::string & xAxis,
                          const std::string & yAxis  )
      {
        return setAxisLabels_<TProfile2D>( hist, xAxis, yAxis );
      }

      // --------------------------------------------------------------------------

    }
  }
}

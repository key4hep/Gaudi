#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning( disable : 2259 )
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning( disable : 4996 )
#endif

#include "GaudiPI.h"
#include <GaudiCommonSvc/HistogramUtility.h>
#include <GaudiCommonSvc/P1D.h>
#include <GaudiKernel/ObjectFactory.h>
#include <cmath>

std::pair<DataObject*, AIDA::IProfile1D*> Gaudi::createProf1D( const std::string& title, int nBins, double xlow,
                                                               double xup, double ylow, double yup,
                                                               const std::string& opt )
{
  auto _p = new TProfile( title.c_str(), title.c_str(), nBins, xlow, xup, ylow, yup, opt.c_str() );
  auto p  = new Profile1D( _p );
  return {p, p};
}

std::pair<DataObject*, AIDA::IProfile1D*> Gaudi::createProf1D( const std::string& title, const Edges& e, double ylow,
                                                               double yup, const std::string& opt )
{
  auto p =
      new Profile1D( new TProfile( title.c_str(), title.c_str(), e.size() - 1, &e.front(), ylow, yup, opt.c_str() ) );
  return {p, p};
}

std::pair<DataObject*, AIDA::IProfile1D*> Gaudi::createProf1D( const AIDA::IProfile1D& hist )
{
  TProfile* h = getRepresentation<AIDA::IProfile1D, TProfile>( hist );
  auto n      = ( h ? new Profile1D( new TProfile( *h ) ) : nullptr );
  return {n, n};
}

namespace Gaudi
{
  template <>
  int Generic1D<AIDA::IProfile1D, TProfile>::binEntries( int index ) const
  {
    return int( m_rep->GetBinEntries( rIndex( index ) ) + 0.5 );
  }

  template <>
  void* Generic1D<AIDA::IProfile1D, TProfile>::cast( const std::string& className ) const
  {
    if ( className == "AIDA::IProfile1D" )
      return const_cast<AIDA::IProfile1D*>( (AIDA::IProfile1D*)this );
    else if ( className == "AIDA::IProfile" )
      return const_cast<AIDA::IProfile*>( (AIDA::IProfile*)this );
    else if ( className == "AIDA::IBaseHistogram" )
      return const_cast<AIDA::IBaseHistogram*>( (AIDA::IBaseHistogram*)this );
    return nullptr;
  }

  template <>
  void Generic1D<AIDA::IProfile1D, TProfile>::adoptRepresentation( TObject* rep )
  {
    TProfile* imp = dynamic_cast<TProfile*>( rep );
    if ( !imp ) throw std::runtime_error( "Cannot adopt native histogram representation." );
    m_rep.reset( imp );
    m_axis.initialize( m_rep->GetXaxis(), true );
    const TArrayD* a = m_rep->GetSumw2();
    if ( !a || ( a && a->GetSize() == 0 ) ) m_rep->Sumw2();
    setTitle( m_rep->GetTitle() );
  }
}

Gaudi::Profile1D::Profile1D() : Base( new TProfile() ) { init( "", false ); }

Gaudi::Profile1D::Profile1D( TProfile* rep ) : Base( rep ) { init( m_rep->GetTitle() ); }

void Gaudi::Profile1D::init( const std::string& title, bool initialize_axis )
{
  m_classType = "IProfile1D";
  setTitle( title );
  setName( title );
  if ( initialize_axis ) {
    axis().initialize( m_rep->GetXaxis(), false );
  }
  // m_rep->SetErrorOption("s");
  m_rep->SetDirectory( nullptr );
  m_sumEntries = 0;
}

bool Gaudi::Profile1D::setBinContents( int i, int entries, double height, double /*error*/, double spread,
                                       double /* centre */ )
{
  m_rep->SetBinEntries( rIndex( i ), entries );
  // set content takes in root height * entries
  m_rep->SetBinContent( rIndex( i ), height * entries );
  // set error takes sqrt of bin sum(w*y**2)
  double sumwy2Bin = ( spread * spread + height * height ) * entries;
  m_rep->SetBinError( rIndex( i ), sqrt( sumwy2Bin ) );
  m_sumEntries += entries;
  // not very efficient (but do evey bin since root cannot figure out by himself)
  m_rep->SetEntries( m_sumEntries );
  return true;
}

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison is correct
#pragma warning( disable : 1572 )
#endif
bool Gaudi::Profile1D::fill( double x, double y, double weight )
{
  ( weight == 1. ) ? m_rep->Fill( x, y ) : m_rep->Fill( x, y, weight );
  return true;
}

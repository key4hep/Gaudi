/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#  pragma warning( disable : 2259 )
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#  pragma warning( disable : 4996 )
#endif
#include "GaudiPI.h"
#include <Gaudi/MonitoringHub.h>
#include <GaudiCommonSvc/H1D.h>
#include <GaudiCommonSvc/H2D.h>
#include <GaudiCommonSvc/HistogramUtility.h>
#include <GaudiCommonSvc/P1D.h>
#include <GaudiKernel/ObjectFactory.h>
#include <TH2D.h>
#include <TProfile.h>
#include <array>

#include <Gaudi/Histograming/Sink/Utils.h>

namespace {
  using AIDA::IHistogram1D;
  using AIDA::IHistogram2D;
  using AIDA::IProfile1D;
} // namespace

std::pair<DataObject*, IHistogram2D*> Gaudi::createH2D( ISvcLocator* svcLocator, const std::string& path,
                                                        const std::string& title, int binsX, double iminX, double imaxX,
                                                        int binsY, double iminY, double imaxY ) {
  auto p = new Histogram2D( new TH2D( title.c_str(), title.c_str(), binsX, iminX, imaxX, binsY, iminY, imaxY ) );
  svcLocator->monitoringHub().registerEntity( "", path, "histogram:Histogram:double", *p );
  return { p, p };
}

std::pair<DataObject*, IHistogram2D*> Gaudi::createH2D( ISvcLocator* svcLocator, const std::string& path,
                                                        const std::string& title, const Edges& eX, const Edges& eY ) {
  auto p = new Histogram2D(
      new TH2D( title.c_str(), title.c_str(), eX.size() - 1, &eX.front(), eY.size() - 1, &eY.front() ) );
  svcLocator->monitoringHub().registerEntity( "", path, "histogram:Histogram:double", *p );
  return { p, p };
}

std::pair<DataObject*, IHistogram2D*> Gaudi::createH2D( ISvcLocator* svcLocator, const std::string& path, TH2D* rep ) {
  auto p = new Histogram2D( rep );
  svcLocator->monitoringHub().registerEntity( "", path, "histogram:Histogram:double", *p );
  return { p, p };
}

std::pair<DataObject*, IHistogram2D*> Gaudi::createH2D( ISvcLocator* svcLocator, const std::string& path,
                                                        const IHistogram2D& hist ) {
  TH2D*        h = getRepresentation<AIDA::IHistogram2D, TH2D>( hist );
  Histogram2D* n = h ? new Histogram2D( new TH2D( *h ) ) : nullptr;
  if ( n ) { svcLocator->monitoringHub().registerEntity( path, h->GetName(), "histogram:Histogram:double", *n ); }
  return { n, n };
}

std::pair<DataObject*, IHistogram1D*> Gaudi::slice1DX( const std::string& nam, const IHistogram2D& hist, int first,
                                                       int last ) {
  TH2*  r = getRepresentation<IHistogram2D, TH2>( hist );
  TH1D* t = r ? r->ProjectionX( "_px", first, last, "e" ) : nullptr;
  if ( t ) t->SetName( nam.c_str() );
  Histogram1D* p = ( t ? new Histogram1D( t ) : nullptr );
  return { p, p };
}

std::pair<DataObject*, IHistogram1D*> Gaudi::slice1DY( const std::string& nam, const IHistogram2D& hist, int first,
                                                       int last ) {
  TH2*  r = getRepresentation<IHistogram2D, TH2>( hist );
  TH1D* t = r ? r->ProjectionY( "_py", first, last, "e" ) : nullptr;
  if ( t ) t->SetName( nam.c_str() );
  Histogram1D* p = ( t ? new Histogram1D( t ) : nullptr );
  return { p, p };
}

std::pair<DataObject*, IHistogram1D*> Gaudi::project1DY( const std::string& nam, const IHistogram2D& hist, int first,
                                                         int last ) {
  TH2*  r = getRepresentation<IHistogram2D, TH2>( hist );
  TH1D* t = r ? r->ProjectionY( "_px", first, last, "e" ) : nullptr;
  if ( t ) t->SetName( nam.c_str() );
  Histogram1D* p = ( t ? new Histogram1D( t ) : nullptr );
  return { p, p };
}

std::pair<DataObject*, IProfile1D*> Gaudi::profile1DX( const std::string& nam, const IHistogram2D& hist, int first,
                                                       int last ) {
  TH2*      r = Gaudi::getRepresentation<IHistogram2D, TH2>( hist );
  TProfile* t = r ? r->ProfileX( "_pfx", first, last, "e" ) : nullptr;
  if ( t ) t->SetName( nam.c_str() );
  Profile1D* p = ( t ? new Profile1D( t ) : nullptr );
  return { p, p };
}

std::pair<DataObject*, IProfile1D*> Gaudi::profile1DY( const std::string& nam, const IHistogram2D& hist, int first,
                                                       int last ) {
  TH2*      r = getRepresentation<IHistogram2D, TH2>( hist );
  TProfile* t = r ? r->ProfileY( "_pfx", first, last, "e" ) : nullptr;
  if ( t ) t->SetName( nam.c_str() );
  Profile1D* p = ( t ? new Profile1D( t ) : nullptr );
  return { p, p };
}

namespace Gaudi {
  template <>
  void* Generic2D<IHistogram2D, TH2D>::cast( const std::string& className ) const {
    if ( className == "AIDA::IHistogram2D" )
      return (IHistogram2D*)this;
    else if ( className == "AIDA::IHistogram" )
      return (IHistogram*)this;
    return nullptr;
  }

  template <>
  int Generic2D<IHistogram2D, TH2D>::binEntries( int indexX, int indexY ) const {
    if ( binHeight( indexX, indexY ) <= 0 ) return 0;
    double xx = binHeight( indexX, indexY ) / binError( indexX, indexY );
    return int( xx * xx + 0.5 );
  }

  template <>
  void Generic2D<AIDA::IHistogram2D, TH2D>::adoptRepresentation( TObject* rep ) {
    TH2D* imp = dynamic_cast<TH2D*>( rep );
    if ( !imp ) throw std::runtime_error( "Cannot adopt native histogram representation." );
    m_rep.reset( imp );
    m_xAxis.initialize( m_rep->GetXaxis(), true );
    m_yAxis.initialize( m_rep->GetYaxis(), true );
    const TArrayD* a = m_rep->GetSumw2();
    if ( !a || ( a && a->GetSize() == 0 ) ) m_rep->Sumw2();
    setTitle( m_rep->GetTitle() );
  }
} // namespace Gaudi

Gaudi::Histogram2D::Histogram2D() : Base( new TH2D() ) {
  m_rep->Sumw2();
  m_sumwx = m_sumwy = 0;
  setTitle( "" );
  m_rep->SetDirectory( nullptr );
}

Gaudi::Histogram2D::Histogram2D( TH2D* rep ) {
  adoptRepresentation( rep );
  m_sumwx = m_sumwy = 0;
  m_rep->SetDirectory( nullptr );
}

bool Gaudi::Histogram2D::setBinContents( int i, int j, int entries, double height, double error, double centreX,
                                         double centreY ) {
  m_rep->SetBinContent( rIndexX( i ), rIndexY( j ), height );
  m_rep->SetBinError( rIndexX( i ), rIndexY( j ), error );
  // accumulate sumwx for in range bins
  if ( i >= 0 && j >= 0 ) {
    m_sumwx += centreX * height;
    m_sumwy += centreY * height;
  }
  m_sumEntries += entries;
  return true;
}

bool Gaudi::Histogram2D::reset() {
  m_sumwx = 0;
  m_sumwy = 0;
  return Base::reset();
}

nlohmann::json Gaudi::Histogram2D::toJSON() const { return *m_rep.get(); }

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison are meant
#  pragma warning( push )
#  pragma warning( disable : 1572 )
#endif
bool Gaudi::Histogram2D::fill( double x, double y, double weight ) {
  // avoid race conditiosn when filling the histogram
  auto guard = std::scoped_lock{ m_fillSerialization };
  ( weight == 1. ) ? m_rep->Fill( x, y ) : m_rep->Fill( x, y, weight );
  return true;
}

bool Gaudi::Histogram2D::setRms( double rmsX, double rmsY ) {
  m_rep->SetEntries( m_sumEntries );
  std::vector<double> stat( 11 );
  stat[0] = sumBinHeights();
  stat[1] = 0;
  if ( equivalentBinEntries() != 0 ) stat[1] = ( sumBinHeights() * sumBinHeights() ) / equivalentBinEntries();
  stat[2]      = m_sumwx;
  double meanX = 0;
  if ( sumBinHeights() != 0 ) meanX = m_sumwx / sumBinHeights();
  stat[3]      = ( meanX * meanX + rmsX * rmsX ) * sumBinHeights();
  stat[4]      = m_sumwy;
  double meanY = 0;
  if ( sumBinHeights() != 0 ) meanY = m_sumwy / sumBinHeights();
  stat[5] = ( meanY * meanY + rmsY * rmsY ) * sumBinHeights();
  stat[6] = 0;
  m_rep->PutStats( &stat.front() );
  return true;
}

void Gaudi::Histogram2D::copyFromAida( const IHistogram2D& h ) {
  // implement here the copy
  std::string titlestr = h.title();
  const char* title    = titlestr.c_str();
  if ( h.xAxis().isFixedBinning() && h.yAxis().isFixedBinning() )
    m_rep.reset( new TH2D( title, title, h.xAxis().bins(), h.xAxis().lowerEdge(), h.xAxis().upperEdge(),
                           h.yAxis().bins(), h.yAxis().lowerEdge(), h.yAxis().upperEdge() ) );
  else {
    Edges eX, eY;
    for ( int i = 0; i < h.xAxis().bins(); ++i ) eX.push_back( h.xAxis().binLowerEdge( i ) );
    // add also upperedges at the end
    eX.push_back( h.xAxis().upperEdge() );
    for ( int i = 0; i < h.yAxis().bins(); ++i ) eY.push_back( h.yAxis().binLowerEdge( i ) );
    // add also upperedges at the end
    eY.push_back( h.yAxis().upperEdge() );
    m_rep.reset( new TH2D( title, title, eX.size() - 1, &eX.front(), eY.size() - 1, &eY.front() ) );
  }
  m_xAxis.initialize( m_rep->GetXaxis(), true );
  m_yAxis.initialize( m_rep->GetYaxis(), true );
  m_rep->Sumw2();
  m_sumEntries = 0;
  m_sumwx      = 0;
  m_sumwy      = 0;
  // statistics
  double sumw  = h.sumBinHeights();
  double sumw2 = 0;
  if ( h.equivalentBinEntries() != 0 ) sumw2 = ( sumw * sumw ) / h.equivalentBinEntries();
  double sumwx  = h.meanX() * h.sumBinHeights();
  double sumwx2 = ( h.meanX() * h.meanX() + h.rmsX() * h.rmsX() ) * h.sumBinHeights();
  double sumwy  = h.meanY() * h.sumBinHeights();
  double sumwy2 = ( h.meanY() * h.meanY() + h.rmsY() * h.rmsY() ) * h.sumBinHeights();
  double sumwxy = 0;

  // copy the contents in  (AIDA underflow/overflow are -2,-1)
  for ( int i = -2; i < xAxis().bins(); ++i ) {
    for ( int j = -2; j < yAxis().bins(); ++j ) {
      // root binning starts from one !
      m_rep->SetBinContent( rIndexX( i ), rIndexY( j ), h.binHeight( i, j ) );
      m_rep->SetBinError( rIndexX( i ), rIndexY( j ), h.binError( i, j ) );
      // calculate statistics
      if ( i >= 0 && j >= 0 ) { sumwxy += h.binHeight( i, j ) * h.binMeanX( i, j ) * h.binMeanY( i, j ); }
    }
  }
  // need to do set entries after setting contents otherwise root will recalculate them
  // taking into account how many time  SetBinContents() has been called
  m_rep->SetEntries( h.allEntries() );
  // fill stat vector
  std::array<double, 11> stat = { { sumw, sumw2, sumwx, sumwx2, sumwy, sumwy2, sumwxy } };
  m_rep->PutStats( stat.data() );
}

#ifdef __ICC
// re-enable icc remark #1572
#  pragma warning( pop )
#endif

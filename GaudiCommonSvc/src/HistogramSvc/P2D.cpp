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
#include <GaudiCommonSvc/HistogramUtility.h>
#include <GaudiCommonSvc/P2D.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/ObjectFactory.h>

#include <Gaudi/Histograming/Sink/Utils.h>

#include <TH2D.h>
#include <TProfile2D.h>

namespace Gaudi {
  template <>
  void* Generic2D<AIDA::IProfile2D, TProfile2D>::cast( const std::string& className ) const {
    return className == "AIDA::IProfile2D"
               ? const_cast<AIDA::IProfile2D*>( static_cast<const AIDA::IProfile2D*>( this ) )
           : className == "AIDA::IProfile" ? const_cast<AIDA::IProfile*>( static_cast<const AIDA::IProfile*>( this ) )
           : className == "AIDA::IBaseHistogram"
               ? const_cast<AIDA::IBaseHistogram*>( static_cast<const AIDA::IBaseHistogram*>( this ) )
               : nullptr;
  }

  template <>
  int Generic2D<AIDA::IProfile2D, TProfile2D>::binEntries( int idX, int idY ) const {
    int rBin = m_rep->GetBin( rIndexX( idX ), rIndexY( idY ) );
    return int( m_rep->GetBinEntries( rBin ) + 0.5 );
  }

  template <>
  void Generic2D<AIDA::IProfile2D, TProfile2D>::adoptRepresentation( TObject* rep ) {
    TProfile2D* imp = dynamic_cast<TProfile2D*>( rep );
    if ( !imp ) throw std::runtime_error( "Cannot adopt native histogram representation." );
    m_rep.reset( imp );
    m_xAxis.initialize( m_rep->GetXaxis(), true );
    m_yAxis.initialize( m_rep->GetYaxis(), true );
    setTitle( m_rep->GetTitle() );
  }
} // namespace Gaudi

std::pair<DataObject*, AIDA::IProfile2D*> Gaudi::createProf2D( ISvcLocator* svcLocator, const std::string& path,
                                                               const std::string& title, const Edges& eX,
                                                               const Edges& eY, double /* zlow */, double /* zup */ ) {
  // Not implemented in ROOT! Can only use TProfile2D with no z-limits
  auto p = new Profile2D( new TProfile2D( title.c_str(), title.c_str(), eX.size() - 1, &eX.front(), eY.size() - 1,
                                          &eY.front() /*,zlow,zup */ ) );
  svcLocator->monitoringHub().registerEntity( "", path, "histogram:ProfileHistogram:double", *p );
  return { p, p };
}

std::pair<DataObject*, AIDA::IProfile2D*> Gaudi::createProf2D( ISvcLocator* svcLocator, const std::string& path,
                                                               const std::string& title, int binsX, double xlow,
                                                               double xup, int binsY, double ylow, double yup,
                                                               double zlow, double zup ) {
  auto p =
      new Profile2D( new TProfile2D( title.c_str(), title.c_str(), binsX, xlow, xup, binsY, ylow, yup, zlow, zup ) );
  svcLocator->monitoringHub().registerEntity( "", path, "histogram:ProfileHistogram:double", *p );
  return { p, p };
}

std::pair<DataObject*, AIDA::IProfile2D*> Gaudi::createProf2D( ISvcLocator* svcLocator, const std::string& path,
                                                               const AIDA::IProfile2D& hist ) {
  auto h = getRepresentation<AIDA::IProfile2D, TProfile2D>( hist );
  auto n = ( h ? new Profile2D( new TProfile2D( *h ) ) : nullptr );
  if ( n ) { svcLocator->monitoringHub().registerEntity( "", path, "histogram:ProfileHistogram:double", *n ); }
  return { n, n };
}

nlohmann::json Gaudi::Profile2D::toJSON() const { return *m_rep.get(); }

Gaudi::Profile2D::Profile2D( TProfile2D* rep ) {
  m_classType = "IProfile2D";
  rep->SetDirectory( nullptr );
  adoptRepresentation( rep );
  m_sumEntries = 0;
}

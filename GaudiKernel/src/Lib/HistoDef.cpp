/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// ============================================================================
// ST D& STL
// ============================================================================
#include <ostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/HistoDef.h>
#include <GaudiKernel/IHistogramSvc.h>
#include <GaudiKernel/ToStream.h>
// ============================================================================
/** @file
 *  The implementation file for class Gaudi::Histo1DDef and related functions
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2007-09-17
 */
// ============================================================================
/*  full constructor from edges, #bins and the title
 *  @param low  the low  edge of the histogram
 *  @param high the high edge of the histogram
 *  @param bins number of bins
 *  @param title the histogram title
 */
// ============================================================================
Gaudi::Histo1DDef::Histo1DDef( const double low, const double high, const int bins, std::string title )
    : m_title( std::move( title ) ), m_low( low ), m_high( high ), m_bins( bins ) {}
// ============================================================================
/*  full constructor from edges, #bins and the title
 *  @param title the histogram title
 *  @param low  the low  edge of the histogram
 *  @param high the high edge of the histogram
 *  @param bins number of bins
 */
// ============================================================================
Gaudi::Histo1DDef::Histo1DDef( std::string title, const double low, const double high, const int bins )
    : m_title( std::move( title ) ), m_low( low ), m_high( high ), m_bins( bins ) {}
// ============================================================================
// printout of the histogram definition
// ============================================================================
std::ostream& Gaudi::Histo1DDef::fillStream( std::ostream& o ) const {
  return o << "(" << Gaudi::Utils::toString( title() ) << "," << lowEdge() << "," << highEdge() << "," << bins() << ")";
}
namespace Gaudi {
  // ============================================================================
  // ordering operator (to please BoundedVerifier)
  // ============================================================================
  bool operator<( const Gaudi::Histo1DDef& left, const Gaudi::Histo1DDef& right ) {
    return std::tie( left.m_title, left.m_low, left.m_high, left.m_bins ) <
           std::tie( right.m_title, right.m_low, right.m_high, right.m_bins );
  }
  // ============================================================================
  // the streamer operator for class Gaudi::Histo1DDef
  // ============================================================================
  std::ostream& operator<<( std::ostream& o, const Gaudi::Histo1DDef& histo ) { return histo.fillStream( o ); }
} // namespace Gaudi
// ============================================================================

// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service
 *  @param path full path in Histogram Data Store
 *  @param hist histogram desctriprion
 */
// ============================================================================
AIDA::IHistogram1D* Gaudi::Histos::book( IHistogramSvc* svc, const std::string& path, const Gaudi::Histo1DDef& hist ) {
  return svc ? svc->book( path, hist.title(), hist.bins(), hist.lowEdge(), hist.highEdge() ) : nullptr;
}
// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service
 *  @param dir  directory path in Histogram Data Store
 *  @param id  histogram identifier
 *  @param hist histogram desctriprion
 */
// ============================================================================
AIDA::IHistogram1D* Gaudi::Histos::book( IHistogramSvc* svc, const std::string& dir, const std::string& id,
                                         const Gaudi::Histo1DDef& hist ) {
  return svc ? svc->book( dir, id, hist.title(), hist.bins(), hist.lowEdge(), hist.highEdge() ) : nullptr;
}
// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service
 *  @param dir  directory path in Histogram Data Store
 *  @param id  histogram identifier
 *  @param hist histogram desctriprion
 */
// ============================================================================
AIDA::IHistogram1D* Gaudi::Histos::book( IHistogramSvc* svc, const std::string& dir, const int id,
                                         const Gaudi::Histo1DDef& hist ) {
  return svc ? svc->book( dir, id, hist.title(), hist.bins(), hist.lowEdge(), hist.highEdge() ) : nullptr;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

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
#ifndef GAUDIKERNEL_HISTODEF_H
#define GAUDIKERNEL_HISTODEF_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iosfwd>
#include <string>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// Forward decalrations
// ============================================================================
class IHistogramSvc; ///< GaudiKernel
namespace AIDA {
  class IHistogram1D;
} // namespace AIDA
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  /** @class Histo1DDef HistoDef.h GaudiKernel/HistoDef.h
   *  Simple helper class for description of 1D-histogram
   *  The class is targeted to act as the primary "histogram property",
   *  but clearly have significantly wider application range
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-09-17
   */
  class GAUDI_API Histo1DDef final {
  public:
    // ========================================================================
    /** full constructor from edges, #bins and the title
     *  @param low  the low  edge of the histogram
     *  @param high the high edge of the histogram
     *  @param bins number of bins
     *  @param title the historgam title
     */
    Histo1DDef( double low, double high, int bins = 100, std::string title = "" );
    // ========================================================================
    /** full constructor from edges, #bins and the title
     *  @param title the historgam title
     *  @param low  the low  edge of the histogram
     *  @param high the high edge of the histogram
     *  @param bins number of bins
     */
    Histo1DDef( std::string title = "", double low = 0.0, double high = 1.0, int bins = 100 );
    // ========================================================================
  public:
    // ========================================================================
    /// get the low edge
    double lowEdge() const { return m_low; }
    /// get the high  edge
    double highEdge() const { return m_high; }
    /// get the number of bins
    int bins() const { return m_bins; }
    /// get the title
    const std::string& title() const { return m_title; }
    // ========================================================================
  public:
    // ========================================================================
    /// set low edge
    void setLowEdge( double value ) { m_low = value; }
    /// set high edge
    void setHighEdge( double value ) { m_high = value; }
    /// set number of bis
    void setBins( int value ) { m_bins = value; }
    /// set the title
    void setTitle( std::string value ) { m_title = std::move( value ); }
    // ========================================================================
  public:
    // ========================================================================
    /// printout of the histogram definition
    std::ostream& fillStream( std::ostream& o ) const;
    // ========================================================================
  public:
    // ========================================================================
    /// ordering operator (to please BoundedVerifier)
    friend bool operator<( const Histo1DDef& left, const Histo1DDef& right );
    /// equality operator
    friend bool operator==( const Histo1DDef& left, const Histo1DDef& right );
    /// non-equality
    friend bool operator!=( const Histo1DDef& left, const Histo1DDef& right );
    // ========================================================================
    /// the streamer operator for class Gaudi::Histo1DDef
    friend std::ostream& operator<<( std::ostream& o, const Gaudi::Histo1DDef& histo );
    // ========================================================================
  public:
    // ========================================================================
    /// check if all fields are "reasonable"
    bool ok() const { return 0 < bins() && lowEdge() < highEdge(); }
    // ========================================================================
  private:
    // ========================================================================
    // Histogram title
    std::string m_title; ///< Histogram title
    // Low  Edge
    double m_low; ///< Low  Edge
    // High Edge
    double m_high; ///< High Edge
    // Number of bins
    int m_bins; ///< Number of bins
    // ========================================================================
  };
  // ==========================================================================
  /** @namespace Gaudi::Histos
   *  collection of simple utilities to deal with histograms
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-09-17
   */
  namespace Histos {
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc pointer to Histogram Service
     *  @param path full path in Histogram Data Store
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book( IHistogramSvc* svc, const std::string& path, const Gaudi::Histo1DDef& hist );
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc  pointer to Histogram Service
     *  @param dir  directory path in Histogram Data Store
     *  @param id   historgam identifier
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book( IHistogramSvc* svc, const std::string& dir, const std::string& id,
                                        const Gaudi::Histo1DDef& hist );
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc pointer to Histogram Service
     *  @param dir  directory path in Histogram Data Store
     *  @param id  historgam identifier
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book( IHistogramSvc* svc, const std::string& dir, const int id,
                                        const Gaudi::Histo1DDef& hist );
    // ========================================================================
  } // namespace Histos
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_HISTODEF_H

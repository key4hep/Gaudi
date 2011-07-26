// $Id: HistoDef.h,v 1.1 2007/09/26 16:13:41 marcocle Exp $
// ============================================================================
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
class IHistogramSvc ;                      ///< GaudiKernel
namespace AIDA { class IHistogram1D ; }    ///< AIDA
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  /** @class Histo1DDef HistoDef.h GaudiKernel/HistoDef.h
   *  Simple helper class for description of 1D-histogram
   *  The class is targeted to act as the primary "histogram property",
   *  but clearly have significantly wider application range
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-09-17
   */
  class GAUDI_API Histo1DDef
  {
  public:
    // ========================================================================
    /** full constructor from edges, #bins and the title
     *  @param low  the low  edge of the histogram
     *  @param high the high edge of the histogram
     *  @param bins number of bins
     *  @param title the historgam title
     */
    Histo1DDef
    ( const double       low         ,
      const double       high        ,
      const int          bins  = 100 ,
      const std::string& title = ""  );
    // ========================================================================
    /** full constructor from edges, #bins and the title
     *  @param title the historgam title
     *  @param low  the low  edge of the histogram
     *  @param high the high edge of the histogram
     *  @param bins number of bins
     */
    Histo1DDef
    ( const std::string& title = ""  ,
      const double       low   = 0.0 ,
      const double       high  = 1.0 ,
      const int          bins  = 100 ) ;
    // ========================================================================
    /// destructor
    virtual ~Histo1DDef( ); ///< virtual destructor
    // ========================================================================
  public:
    // ========================================================================
    /// get the low edge
    double             lowEdge  () const { return m_low   ; }
    /// get the high  edge
    double             highEdge () const { return m_high  ; }
    /// get the number of bins
    int                bins     () const { return m_bins  ; }
    /// get the title
    const std::string& title    () const { return m_title ; }
    // ========================================================================
  public:
    // ========================================================================
    /// set low edge
    void setLowEdge  ( const double       value ) { m_low   = value ; }
    /// set high edge
    void setHighEdge ( const double       value ) { m_high  = value ; }
    /// set number of bis
    void setBins     ( const int          value ) { m_bins  = value ; }
    /// set the title
    void setTitle    ( const std::string& value ) { m_title = value ; }
    // ========================================================================
  public:
    // ========================================================================
    /// printout of the histogram definition
    std::ostream& fillStream ( std::ostream& o ) const ;
    // ========================================================================
  public:
    // ========================================================================
    /// ordering operator (to please BoundedVerifier)
    bool operator< ( const Histo1DDef& right ) const ;
    /// equality operator
    bool operator==( const Histo1DDef& right ) const ;
    /// non-equality
    bool operator!=( const Histo1DDef& right ) const ;
    // ========================================================================
  public:
    // ========================================================================
    /// check if all fields are "reasonable"
    bool ok () const { return 0 < bins() && lowEdge() < highEdge() ; }
    // ========================================================================
  private:
    // ========================================================================
    // Histogram title
    std::string          m_title      ;   ///< Histogram title
    // Low  Edge
    double               m_low        ;   ///< Low  Edge
    // High Edge
    double               m_high       ;   ///< High Edge
    // Number of bins
    int                  m_bins       ;   ///< Number of bins
    // ========================================================================
  };
  // ==========================================================================
  /// the streamer operator for class Gaudi::Histo1DDef
  GAUDI_API std::ostream& 
  operator<<( std::ostream& o , const Gaudi::Histo1DDef& histo ) ;
  // ==========================================================================
  /** @namespace Gaudi::Histos
   *  collection of simple utilities to deal with histograms
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-09-17
   */
  namespace Histos
  {
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc pointer to Histogram Service
     *  @param path full path in Histogram Data Store
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book
    ( IHistogramSvc*           svc  ,
      const std::string&       path ,
      const Gaudi::Histo1DDef& hist ) ;
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc  pointer to Histogram Service
     *  @param dir  directory path in Histogram Data Store
     *  @param id   historgam identifier
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book
    ( IHistogramSvc*           svc  ,
      const std::string&       dir  ,
      const std::string&       id   ,
      const Gaudi::Histo1DDef& hist ) ;
    // ========================================================================
    /** helper function to book 1D-histogram
     *  @param svc pointer to Histogram Service
     *  @param dir  directory path in Histogram Data Store
     *  @param id  historgam identifier
     *  @param hist histogram desctription
     */
    GAUDI_API AIDA::IHistogram1D* book
    ( IHistogramSvc*           svc  ,
      const std::string&       dir  ,
      const int                id   ,
      const Gaudi::Histo1DDef& hist ) ;
    // ========================================================================
  } // end of namespace Gaudi::Histos
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_HISTODEF_H
// ============================================================================

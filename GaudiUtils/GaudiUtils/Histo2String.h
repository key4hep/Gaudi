#ifndef GAUDIUTILS_HISTO2STRING_H
#define GAUDIUTILS_HISTO2STRING_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <iosfwd>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// forward declarations
// ============================================================================
// Forward declarations
// ============================================================================
// AIDA
// ============================================================================
namespace AIDA
{
  class IHistogram1D ;
  class IHistogram2D ;
}
// ============================================================================
// ROOT
// ============================================================================
class TH1D ;                                                            // ROOT
class TH2D ;                                                            // ROOT
class TH1F ;                                                            // ROOT
class TH2F ;                                                            // ROOT
// ============================================================================
namespace Gaudi
{
  // ===========================================================================
  namespace Utils
  {
    // =========================================================================
    /** stream the ROOT histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const TH1D&   histo         ,
      std::ostream& stream        ,
      const bool    asXML = false ) ;
    // =========================================================================
    /** stream the ROOT histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const TH2D&   histo         ,
      std::ostream& stream        ,
      const bool    asXML = false ) ;
    // =========================================================================
    /** stream the ROOT histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const TH1F&   histo         ,
      std::ostream& stream        ,
      const bool    asXML = false ) ;
    // =========================================================================
    /** stream the ROOT histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const TH2F&   histo         ,
      std::ostream& stream        ,
      const bool    asXML = false ) ;
    // ========================================================================
    /** stream the AIDA histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const AIDA::IHistogram1D& histo         ,
      std::ostream&             stream        ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** stream the AIDA histogram into output stream
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param stream (OUTPUT) the stream
     *  @param asXML  (INPUT)  use XML-format
     *  @return the updated stream
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::ostream& toStream
    ( const AIDA::IHistogram2D& histo         ,
      std::ostream&             stream        ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const TH1D&               histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const TH2D&               histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const TH1F&               histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const TH2F&               histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const AIDA::IHistogram1D& histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const AIDA::IHistogram2D& histo         ,
      const bool                asXML = false ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString
    ( const AIDA::IHistogram1D* histo ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString ( AIDA::IHistogram1D* histo ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString ( const TH1D* histo ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString ( TH1D* histo ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString ( const TH2D* histo ) ;
    // ========================================================================
    /** convert the histogram into the string
     *  @param histo  (INPUT)  the histogram to be streamed
     *  @param asXML  (INPUT)  use XML-format
     *  @return the string representation of the histogram
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    GAUDI_API std::string toString ( TH2D* histo ) ;
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_HISTO2STRING_H
// ============================================================================

#ifndef GAUDIUTILS_HISTOPARSERS_H
#define GAUDIUTILS_HISTOPARSERS_H 1
// =============================================================================
// Include files
// =============================================================================
// GaudiKernel
// =============================================================================
#include "GaudiKernel/StatusCode.h"
// =============================================================================
// forward declarations
// =============================================================================
// AIDA
// =============================================================================
namespace AIDA
{
  class IHistogram1D ;                                                   // AIDA
  class IHistogram2D ;                                                   // AIDA
}
// =============================================================================
// ROOT
// =============================================================================
class TH1D ;                                                             // ROOT
class TH2D ;                                                             // ROOT
class TH1F ;                                                             // ROOT
class TH2F ;                                                             // ROOT
// =============================================================================
namespace Gaudi
{
  // ===========================================================================
  namespace Parsers
  {
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH1D& result , const std::string& input ) ;
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH2D& result , const std::string& input ) ;
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH1F& result , const std::string& input ) ;
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH2F& result , const std::string& input ) ;
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( AIDA::IHistogram1D& result , const std::string& input ) ;
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( AIDA::IHistogram2D& result , const std::string& input ) ;
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH1D*& result , const std::string& input ) ;
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse
    ( TH2D*& result , const std::string& input ) ;
    // =========================================================================
  } //                                           end of namespace Gaudi::Parsers
  // ===========================================================================
} // end of namespace Gaudi
// =============================================================================
// The END
// =============================================================================
#endif // GAUDIUTILS_HISTOPARSERS_H
// =============================================================================

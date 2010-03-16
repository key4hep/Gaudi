// $Id: $
// ============================================================================
#ifndef GAUDIUTILS_HISTODUMP_H 
#define GAUDIUTILS_HISTODUMP_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
/// forward declarations:
// ============================================================================
namespace AIDA 
{ 
  class  IHistogram1D ;                                                 // AIDA 
  class  IProfile1D   ;                                                 // AIDA 
}       
// ============================================================================
class TH1      ;                                                        // ROOT 
class TProfile ;                                                        // ROOT 
// ============================================================================
namespace Gaudi 
{
  // ==========================================================================
  namespace Utils 
  {
    // ========================================================================
    namespace Histos 
    {
      // ======================================================================
      /** dump the text representation of the histogram 
       *  @param histo  (INPUT) the histogram 
       *  @param stream (OUTUT) the stream  
       *  @param width  (INPUT) the maximal  column width 
       *  @param height (INPUT) the proposed column height 
       *  @param errors (INPUT) print/plot errors
       *  @return the stream 
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::ostream& histoDump_ 
      ( const AIDA::IHistogram1D* histo          , 
        std::ostream&             stream         , 
        const std::size_t         width  = 80    ,  
        const std::size_t         height = 50    ,  
        const bool                errors = false ) ;
     // ======================================================================
      /** dump the text representation of the histogram 
       *  @param histo  (INPUT) the histogram 
       *  @param width  (INPUT) the maximal  column width 
       *  @param height (INPUT) the proposed column height 
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram       
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API
      std::string   histoDump 
      ( const AIDA::IHistogram1D* histo          , 
        const std::size_t         width  = 80    , 
        const std::size_t         height = 50    ,  
        const bool                errors = false ) ;
      // ======================================================================
      /** dump the text representation of 1D-profile 
       *  @param histo  (INPUT) the 1D-profile  
       *  @param stream (OUTUT) the stream  
       *  @param width  (INPUT) the maximal  column width 
       *  @param height (INPUT) the proposed column height 
       *  @param spread (INPUT) print/plot spread/rms ?
       *  @return the stream 
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::ostream& histoDump_ 
      ( const AIDA::IProfile1D*   histo          , 
        std::ostream&             stream         , 
        const std::size_t         width  = 80    ,  
        const std::size_t         height = 50    ,
        const bool                spread = true  ) ;
      // ====================================================================
      /** dump the text representation of the 1D-profile 
       *  @param histo  (INPUT) the histogram 
       *  @param width  (INPUT) the maximal  column width 
       *  @param height (INPUT) the proposed column height 
       *  @param spread (INPUT) print/plto spread vs rms 
       *  @return string representation of the histogram       
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API
      std::string   histoDump 
      ( const AIDA::IProfile1D*   histo          , 
        const std::size_t         width  = 80    , 
        const std::size_t         height = 50    ,  
        const bool                spread = true  ) ;
      // ======================================================================
      /** dump the text representation of the Profile 
       *  @param histo  (INPUT) the histogram 
       *  @param stream (OUTUT) the stream  
       *  @param width  (INPUT) the maximal column width 
       *  @param height (INPUT) the proposed coulmn height 
       *  @param spread (INPUT) print/plot rms versus erorr 
       *  @return the stream 
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::ostream& histoDump_
      ( const TProfile*           histo           , 
        std::ostream&             stream          ,
        const std::size_t         width   = 80    , 
        const std::size_t         height  = 50    ) ;
      // ====================================================================
      /** dump the text representation of the histogram 
       *  @param histo  (INPUT) the histogram 
       *  @param width  (INPUT) the maximal column width 
       *  @param height (INPUT) the propsoed coulmn height 
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram       
       *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::string   histoDump 
      ( const TProfile*           histo          , 
        const std::size_t         width  = 80    , 
        const std::size_t         height = 50    ) ;
      // ====================================================================
      /** dump the text representation of the histogram 
       *  @param histo  (INPUT) the histogram 
       *  @param stream (OUTUT) the stream  
       *  @param width  (INPUT) the maximal column width 
       *  @param height (INPUT) the proposed coulmn height 
       *  @param errors (INPUT) print/plot errors
       *  @return the stream 
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::ostream& histoDump_ 
      ( const TH1*                histo           , 
        std::ostream&             stream          ,
        const std::size_t         width   = 80    , 
        const std::size_t         height  = 50    ,  
        const bool                errors  = false ) ;
      // ====================================================================
      /** dump the text representation of the histogram 
       *  @param histo  (INPUT) the histogram 
       *  @param width  (INPUT) the maximal column width 
       *  @param height (INPUT) the propsoed coulmn height 
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram       
       *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
       *  @date 2009-09-19
       */ 
      GAUDI_API 
      std::string   histoDump 
      ( const TH1*                histo          , 
        const std::size_t         width  = 80    , 
        const std::size_t         height = 50    ,  
        const bool                errors = false ) ;
      // ======================================================================
    } //                                  end of namespace Gaudi::Utils::Histos 
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIUTILS_HISTODUMP_H
// ============================================================================

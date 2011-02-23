// $Id: Fill.h,v 1.1 2007/10/02 14:53:48 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_FILL_H
#define GAUDIALG_FILL_H 1
// ============================================================================
// Include files
// ============================================================================
// forward declarations from AIDA
// ============================================================================
namespace AIDA
{
  class IBaseHistogram ;
  class IHistogram     ;
  class IHistogram1D   ;
  class IHistogram2D   ;
  class IHistogram3D   ;
  class IProfile       ;
  class IProfile1D     ;
  class IProfile2D     ;
}
#include "GaudiKernel/Kernel.h"
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
      /** simple function to fill AIDA::IHistogram1D objects
       *  @see AIDA::IHistogram1D
       *  @param histo pointer to the histogram
       *  @param value value to be added to the histogram
       *  @param weight the "weight" assciated with this entry
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date 2007-10-02
       */
      GAUDI_API void fill
      ( AIDA::IHistogram1D* histo        ,
        const double        value        ,
        const double        weight = 1.0 ) ;
      // ======================================================================
      /** simple function to fill AIDA::IHistogram2D objects
       *  @see AIDA::IHistogram2D
       *  @param histo pointer to the histogram
       *  @param valueX value to be added to the histogram
       *  @param valueY value to be added to the histogram
       *  @param weight the "weight" assciated with this entry
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date 2007-10-02
       */
      GAUDI_API void fill
      ( AIDA::IHistogram2D* histo        ,
        const double        valueX       ,
        const double        valueY       ,
        const double        weight = 1.0 ) ;
      // ======================================================================
      /** simple function to fill AIDA::IHistogram3D objects
       *  @see AIDA::IHistogram3D
       *  @param histo pointer to the histogram
       *  @param valueX value to be added to the histogram
       *  @param valueY value to be added to the histogram
       *  @param valueZ value to be added to the histogram
       *  @param weight the "weight" assciated with this entry
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date 2007-10-02
       */
      GAUDI_API void fill
      ( AIDA::IHistogram3D* histo        ,
        const double        valueX       ,
        const double        valueY       ,
        const double        valueZ       ,
        const double        weight = 1.0 ) ;
      // ======================================================================
      /** simple function to fill AIDA::IProfile1D objects
       *  @see AIDA::IProfile1D
       *  @param histo pointer to the histogram
       *  @param valueX value to be added to the histogram
       *  @param valueY value to be added to the histogram
       *  @param weight the "weight" assciated with this entry
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date 2007-10-02
       */
      GAUDI_API void fill
      ( AIDA::IProfile1D*   histo        ,
        const double        valueX       ,
        const double        valueY       ,
        const double        weight = 1.0 ) ;
      // ======================================================================
      /** simple function to fill AIDA::IProfile2D objects
       *  @see AIDA::IProfile2D
       *  @param histo pointer to the histogram
       *  @param valueX value to be added to the histogram
       *  @param valueY value to be added to the histogram
       *  @param valueZ value to be added to the histogram
       *  @param weight the "weight" assciated with this entry
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date 2007-10-02
       */
      GAUDI_API void fill
      ( AIDA::IProfile2D*   histo        ,
        const double        valueX       ,
        const double        valueY       ,
        const double        valueZ       ,
        const double        weight = 1.0 ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IBaseHistogram* histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IHistogram*     histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IHistogram1D*   histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IHistogram2D*   histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IHistogram3D*   histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IProfile*       histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IProfile1D*     histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      /// get the title
      GAUDI_API std::string htitle
      ( const AIDA::IProfile2D*     histo      ,
        const std::string&          title = "" ) ;
      // ======================================================================
      GAUDI_API AIDA::IBaseHistogram* toBase ( AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      GAUDI_API AIDA::IBaseHistogram* toBase ( AIDA::IHistogram2D* histo ) ;
      // ======================================================================
      GAUDI_API AIDA::IBaseHistogram* toBase ( AIDA::IHistogram3D* histo ) ;
      // ======================================================================
      GAUDI_API AIDA::IBaseHistogram* toBase ( AIDA::IProfile1D*   histo ) ;
      // ======================================================================
      GAUDI_API AIDA::IBaseHistogram* toBase ( AIDA::IProfile2D*   histo ) ;
      // ======================================================================
    } // end of namespace Gaudi::Utils::Histos
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_FILL_H
// ============================================================================

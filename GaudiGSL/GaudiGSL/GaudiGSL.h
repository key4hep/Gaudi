// ============================================================================
#ifndef GAUDIGSL_GAUDIGSL_H
#define GAUDIGSL_GAUDIGSL_H 1
// Include files
#include "GaudiKernel/Kernel.h"

class IGslSvc;  // from GaudiGSL

/** @class GaudiGSL GaudiGSL.h GaudiGSL/GaudiGSL.h
 *
 *  Helper class to get (static) access to Gaudi  GSL Service
 *  General users are not supposed to use it.
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   29/04/2002
 */
struct GAUDI_API GaudiGSL
{

  /** static accessor to Gaudi GSL Service
   *  @return (const) pointer to Gaudi GSL Service
   */
  static const IGslSvc*   gslSvc () ;

  /**set new value for static Gaudi GSL Service
   *  @return (const) pointer to Gaudi GSL Service
   */
  static const IGslSvc*   setGslSvc ( const IGslSvc* value ) ;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GAUDIGSL_H
// ============================================================================

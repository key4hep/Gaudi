// $Id: GaudiGSL.h,v 1.2 2007/05/24 14:39:10 hmd Exp $
// ============================================================================
// CVS tag $Name:  $
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
class GAUDI_API GaudiGSL
{
public:

  /** static accessor to Gaudi GSL Service
   *  @return (const) pointer to Gaudi GSL Service
   */
  static const IGslSvc*   gslSvc () ;

  /**set new value for static Gaudi GSL Service
   *  @return (const) pointer to Gaudi GSL Service
   */
  static const IGslSvc*   setGslSvc ( const IGslSvc* value ) ;

private:

  static const IGslSvc* s_gslSvc ;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GAUDIGSL_H
// ============================================================================

// $Id: GslErrorHandlers.h,v 1.2 2007/05/24 14:39:10 hmd Exp $
// ============================================================================
// CVS tag $Name:  $
// ============================================================================
#ifndef GAUDIGSL_GSLERRORHANDLERS_H
#define GAUDIGSL_GSLERRORHANDLERS_H 1

#include "GaudiKernel/Kernel.h"

/** @namespace GslErrorHandlers
 *
 *  The collection of Error Handlers for GSL
 *
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   29/04/2002
 */

namespace GslErrorHandlers
{

  /** The simplest Gsl Error handler,
   *  It simply ingnores the error
   *
   *  @see IGslSvc
   *
   *  @param reason error reason (message)
   *  @param file   file  name
   *  @param line   line  number
   *  @param code   error code
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   29/04/2002
   */
  GAUDI_API void ignoreTheError
  ( const char* reason ,
    const char* file   ,
    int         line   ,
    int         code   );

  /** The simplest Gsl Error handler,
   *  It delegates the actual error handling to GSL Service
   *
   *  @see IGslSvc
   *
   *  @param reason error reason (message)
   *  @param file   file  name
   *  @param line   line  number
   *  @param code   error code
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   29/04/2002
   */
  GAUDI_API void handleTheError
  ( const char* reason ,
    const char* file   ,
    int         line   ,
    int         code   );

  /** The simple Gsl Error handler,
   *  it throwns the Gaudi Exception
   *
   *  @exception GaudiException
   *  @see       GaudiException
   *
   *  @param reason error reason (message)
   *  @param file   file  name
   *  @param line   line  number
   *  @param code   error code
   *
   *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
   *  @date   29/04/2002
   */
  GAUDI_API void throwException
  ( const char* reason ,
    const char* file   ,
    int         line   ,
    int         code   );

}

// ============================================================================
// The END
// ============================================================================
#endif // GSLERRORHANDLERS_H
// ============================================================================

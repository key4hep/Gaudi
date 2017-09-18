#ifndef GAUDIGSL_GSLERRORPrint_H
#define GAUDIGSL_GSLERRORPrint_H 1
// Include files
// from STL
#include <string>
// from GaudiKernel
#include "GaudiKernel/AlgTool.h"
// from GaudiGSL
#include "GaudiGSL/IGslErrorHandler.h"
#include "GaudiGSL/GslError.h"
// forward declaration

/** @class GslErrorPrint GslErrorPrint.h
 *
 *  Concrete GSL eror handler
 *  It is just prints the  GSL errors
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */

struct GslErrorPrint: extends<AlgTool,
                              IGslErrorHandler> {

  /** Standard constructor
   */
  using extends::extends;

  /** handle the GSL error
   *  @see IGslErrorHandler
   *  @param error  error to be handled
   *  @see GslError
   *  @return status code
   */
  StatusCode handle( const GslError& error ) const override;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLERRORPrint_H
// ============================================================================

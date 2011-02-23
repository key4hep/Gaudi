// $Id: GslErrorPrint.h,v 1.2 2006/11/30 10:40:53 mato Exp $
// ============================================================================
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

class GslErrorPrint: public extends1<AlgTool, IGslErrorHandler> {
public:

  /** handle the GSL error
   *  @see IGslErrorHandler
   *  @param error  error to be handled
   *  @see GslError
   *  @return status code
   */
  virtual StatusCode handle
  ( const GslError& error ) const ;


  /** Standard constructor
   *  @param type   tool type (?)
   *  @param name   tool name
   *  @param parent pointer to parent
   */
  GslErrorPrint
  ( const std::string& type   ,
    const std::string& name   ,
    const IInterface*  parent );

  /// destructor (protected and virtual)
  virtual ~GslErrorPrint( );

private:

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLERRORPrint_H
// ============================================================================

// $Id: GslErrorException.h,v 1.2 2006/11/30 10:40:53 mato Exp $
// ============================================================================
#ifndef GAUDIGSL_GSLERRORException_H
#define GAUDIGSL_GSLERRORException_H 1
// Include files
// from STL
#include <string>
// from GaudiKernel
#include "GaudiKernel/AlgTool.h"
// from GaudiGSL
#include "GaudiGSL/IGslErrorHandler.h"
#include "GaudiGSL/GslError.h"
// forward declaration

/** @class GslErrorException GslErrorException.h
 *
 *  Concrete GSL eror handler
 *  It is just thrown the exception
 *
 *  @attention The error handling could be "turned off"
 *  for selected error codes (e.g. GSL_SUCCESS or GSL_CONTINUE )
 *  using "IgnoreCodes" property
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */

class GslErrorException: public extends1<AlgTool, IGslErrorHandler> {
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
  GslErrorException
  ( const std::string& type   ,
    const std::string& name   ,
    const IInterface*  parent );

  /// destructor (protected and virtual)
  virtual ~GslErrorException( );

private:

  /// codes to be ignored:
  std::vector<int>    m_ignore ;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLERRORException_H
// ============================================================================

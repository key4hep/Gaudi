// $Id: GslSvc.h,v 1.2 2006/11/30 10:40:53 mato Exp $
// ============================================================================
#ifndef GAUDIGSL_GSLSVC_H
#define GAUDIGSL_GSLSVC_H 1
// Include files
// STD & STL
#include <vector>
#include <string>
// GaudiKernel
#include "GaudiKernel/Service.h"
// Include files
// GaudiGSL
#include "GaudiGSL/IGslSvc.h"
#include "GaudiGSL/GslError.h"

// forward declarations
class IGslErrorHandler                 ; // from GaudiGSL

/** @class GslSvc GslSvc.h
 *
 *  The implementation of IGslSvc interface
 *
 *  The Gsl Service is teh service which perform intelligent error handling
 *  for GSL (GNU Scientific Library)
 *
 *  It provides a choice between different "Error Handling Policies"
 *
 *  Current implementation supports the
 *  several "error handling policies", which are driven by
 *  "ErrorPolicy" property of class GslSvc:
 *
 *  - "GSL":
 *    The simplest error policy, no action, the error
 *    hadling defined in GSL does not redefined
 *
 *  - "Off":
 *    The GSL empty error handling is used, what means the
 *    ignorance of all GSL errors. Program continues after
 *    any error, so the return values from any library routines
 *    must be checked. This policy is recommended by GSL team for
 *    production programs
 *
 *  - "Abort"
 *    The default GSL error policy, causes abort on error.
 *
 *  - "Ignore"
 *    The policy which is essentially the same as "Off",
 *    but performed with local error handler GslErrorHandlers::ignoreTheError
 *
 *  - "Exception"
 *    This policy throws the GaudiException with tag *GSL Error" to be thrown
 *    at the error. The policy implemented by local error handler
 *    GslErrorHandlers::throwException
 *
 *  - "Handle"
 *    The policy which uses the call back from local error handler
 *    GslErrorHandlers::handleTheError to IGslSvc::handle method.
 *    Current implementation of this method is sequential execution of
 *    declared handlers, defined through the property "Handlers" of
 *    class GslSvc in a format:
 *    GslSvc.Handlers = { "H1Type/H1Name" , "H2Type/H2Name" , "H3ype/3Name" };
 *    The "handlers" must implement IGslErrorHandler interface
 *    The concrete handlers are provided in GaudiGSL package:
 *    class GslErorCount, GslErrorPrint and GslErrorException.
 *
 *
 *  @attention The error handling could be "turned off"
 *  for selected error codes (e.g. GSL_SUCCESS or GSL_CONTINUE )
 *  using "IgnoreCodes" property.
 *  This feature is active only for "Handle" error policy.
 *
 *  @see  GslErrorHandlers
 *  @see  GslErrorHandlers::ingnoreTheError
 *  @see  GslErrorHandlers::throwException
 *  @see  GslErrorHandlers::handleTheError
 *  @see IGslSvc
 *  @see IGslErrorHandler
 *  @see  GslErrorCount
 *  @see  GslErrorPrint
 *  @see  GslErrorException
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   29/04/2002
 */
class GslSvc: public extends1<Service, IGslSvc> {
public:

  /** handle the GSL error
   *  @see IGslSvc
   *  @param err   error
   *  @return status code
   */
  virtual StatusCode handle ( const GslError& error ) const ;

  /** retrieve the  current GSL error handler
   *  @see IGslSvc
   *  @return current GSL error handler
   */
  virtual GslErrorHandler  handler ()    const ;

  /** set new GSL error handler
   *  @see IGslSvc
   *  @param  handler   new GSL error handler
   *  @return GSL error handler
   */
  virtual GslErrorHandler  setHandler ( GslErrorHandler handler ) const ;

  /** transform GSL error code to Gaudi status code
   *  @see IGslSvc
   *  @param  error GLS error code
   *  @return status code
   */
  virtual StatusCode    status        ( const int error         ) const ;

  /** standard service initialization
   *  @see  Service
   *  @see IGslSvc
   *  @see IService
   *  @return status code
   */
  virtual StatusCode    initialize () ;

  /** standard service finalization
   *  @see  Service
   *  @see IGslSvc
   *  @see IService
   *  @return status code
   */
  virtual StatusCode    finalize   () ;

  /** Standard constructor
   *  @see Service
   *  @param name service name
   *  @param scv  pointer to service locator
   */
  GslSvc ( const std::string& name ,
           ISvcLocator*       svc  );

  /// destructor, virtual and protected
  virtual ~GslSvc() ;

private:

  ///  default constructor   is private
  GslSvc();
  ///  copy    constructor   is private
  GslSvc            ( const GslSvc& );
  ///  assignment operator  is private
  GslSvc& operator= ( const GslSvc& );

private:

  /// error policy
  std::string    m_errorPolicy ;

  /// external handlers
  typedef std::vector<std::string>       Names               ;
  Names                                  m_handlersTypeNames ;
  typedef std::vector<IGslErrorHandler*> Handlers            ;
  Handlers                               m_handlers          ;

  /// codes to be ignored
  std::vector<int>                       m_ignore            ;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLSVC_H
// ============================================================================

#ifndef GAUDIGSL_GSLSVC_H
#define GAUDIGSL_GSLSVC_H 1
// Include files
// STD & STL
#include <string>
#include <vector>
// GaudiKernel
#include "GaudiKernel/Service.h"
// Include files
// GaudiGSL
#include "GaudiGSL/GslError.h"
#include "GaudiGSL/IGslSvc.h"

// forward declarations
struct IGslErrorHandler; // from GaudiGSL

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
class GslSvc : public extends<Service, IGslSvc>
{
public:

  using extends::extends;

  ///  no copy constructor / assignment
  GslSvc( const GslSvc& ) = delete;
  GslSvc& operator=( const GslSvc& ) = delete;

  /** handle the GSL error
   *  @see IGslSvc
   *  @param err   error
   *  @return status code
   */
  StatusCode handle( const GslError& error ) const override;

  /** retrieve the  current GSL error handler
   *  @see IGslSvc
   *  @return current GSL error handler
   */
  GslErrorHandler handler() const override;

  /** set new GSL error handler
   *  @see IGslSvc
   *  @param  handler   new GSL error handler
   *  @return GSL error handler
   */
  GslErrorHandler setHandler( GslErrorHandler handler ) const override;

  /** transform GSL error code to Gaudi status code
   *  @see IGslSvc
   *  @param  error GLS error code
   *  @return status code
   */
  StatusCode status( const int error ) const override;

  /** standard service initialization
   *  @see  Service
   *  @see IGslSvc
   *  @see IService
   *  @return status code
   */
  StatusCode initialize() override;

  /** standard service finalization
   *  @see  Service
   *  @see IGslSvc
   *  @see IService
   *  @return status code
   */
  StatusCode finalize() override;

private:
  Gaudi::Property<std::string> m_errorPolicy{this, "ErrorPolicy", "GSL", ""};
  Gaudi::Property<std::vector<std::string>> m_handlersTypeNames{this, "Handlers", {}, ""};
  Gaudi::Property<std::vector<int>> m_ignore{this, "IgnoreCodes", {}, "codes to be ignored"};

  std::vector<IGslErrorHandler*> m_handlers;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIGSL_GSLSVC_H
// ============================================================================

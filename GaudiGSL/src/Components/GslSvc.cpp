// Include files
// from Gaudi
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
// STD & STL
#include <algorithm>
#include <functional>
// GaudiGSL
#include "GaudiGSL/GslError.h"
#include "GaudiGSL/IGslErrorHandler.h"
// local
#include "GaudiGSL/GaudiGSL.h"
#include "GaudiGSL/GslErrorHandlers.h"
#include "GslSvc.h"
// gsl
#include "gsl/gsl_errno.h"

// ============================================================================
/** @file
 *
 * Implementation file for class : GslSvc
 *
 * @author Vanya Belyaev Ivan.Belyaev@itep.ru
 * @date 29/04/2002
 */
// ============================================================================
DECLARE_COMPONENT( GslSvc )
// ============================================================================
/** standard service initialization
 *  @see  Service
 *  @see IService
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::initialize()
{
  // initialize the base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) {
    error() << " Error in initialization of base class 'Service'" << endmsg;
    return sc;
  }
  // activate the static accessor to the service
  GaudiGSL::setGslSvc( this );
  // set the error handlers
  if ( "GSL" == m_errorPolicy ) { /* nothing to do */
  } else if ( "Off" == m_errorPolicy ) {
    gsl_set_error_handler_off();
  } else if ( "Abort" == m_errorPolicy ) {
    gsl_set_error_handler( nullptr );
  } else if ( "Ignore" == m_errorPolicy ) {
    gsl_set_error_handler( GslErrorHandlers::ignoreTheError );
  } else if ( "Exception" == m_errorPolicy ) {
    gsl_set_error_handler( GslErrorHandlers::throwException );
  } else if ( "Handle" == m_errorPolicy ) {
    gsl_set_error_handler( GslErrorHandlers::handleTheError );
  } else {
    error() << " Unknown Error policy '" << m_errorPolicy << "'"
            << " Valid policies: "
            << "[ 'GSL' , 'Off' , 'Abort' , 'Ignore' , 'Exception' , 'Handle' ]" << endmsg;
    return StatusCode::FAILURE;
  }
  /// Prints the type of used handler
  /// get the handler
  GslErrorHandler handler = gsl_set_error_handler( nullptr );
  gsl_set_error_handler( handler );
  if ( handler ) {
    verbose() << " GSL Error Handler is '" << System::typeinfoName( typeid( *handler ) ) << "'" << endmsg;
  } else {
    info() << " Error Handler is NULL" << endmsg;
  }

  if ( !m_handlersTypeNames.empty() ) {
    /// Get Tool Service
    auto toolsvc = serviceLocator()->service<IToolSvc>( "ToolSvc" );
    if ( !toolsvc ) {
      error() << " Could not locate Tool Service! " << endmsg;
      return StatusCode::FAILURE;
    }
    for ( const auto& it : m_handlersTypeNames ) {
      auto pos             = it.find( '/' );
      IGslErrorHandler* eh = nullptr;
      if ( pos != std::string::npos ) {
        sc = toolsvc->retrieveTool( it.substr( 0, pos ), it.substr( pos + 1 ), eh, this );
      } else {
        sc = toolsvc->retrieveTool( it, it, eh, this );
      }
      if ( sc.isFailure() ) {
        error() << " Could not retrieve tool '" << it << "'" << endmsg;
        return sc;
      }
      if ( !eh ) {
        error() << " Could not retrieve tool '" << it << "'" << endmsg;
        return StatusCode::FAILURE;
      }
      m_handlers.push_back( eh );
    }
  }
  //
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
/** standard service finalization
 *  @see  Service
 *  @see IService
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::finalize()
{
  debug() << "==> Finalize" << endmsg;

  // deactivate the static accessor to the service
  GaudiGSL::setGslSvc( nullptr );

  // finalize the base class
  return Service::finalize();
}
// ============================================================================

// ============================================================================
/** retrieve the  current GSL error handler
 *  @return current GSL error handler
 */
// ============================================================================
IGslSvc::GslErrorHandler GslSvc::handler() const
{
  GslErrorHandler hh = gsl_set_error_handler( nullptr );
  gsl_set_error_handler( hh );
  return hh;
}
// ============================================================================

// ============================================================================
/** set new GSL error handler
 *  @param  handler   new GSL error handler
 *  @return GSL error handler
 */
// ============================================================================
IGslSvc::GslErrorHandler GslSvc::setHandler( IGslSvc::GslErrorHandler handler ) const
{
  gsl_set_error_handler( handler );
  {
    debug() << " New GSL handler is set '" << ( handler ? System::typeinfoName( typeid( handler ) ) : "NULL" ) << "'"
            << endmsg;
  }
  return handler;
}
// ============================================================================

// ============================================================================
/** transform GSL error code to Gaudi status code
 *  @param  error GLS error code
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::status( const int error ) const
{
  if ( GSL_SUCCESS == error ) {
    return StatusCode::SUCCESS;
  }
  StatusCode sc( error );
  if ( sc.isSuccess() ) {
    return StatusCode::FAILURE;
  }
  return sc;
}
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @param error error
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::handle( const GslError& error ) const
{
  StatusCode sc = StatusCode::SUCCESS;
  // code to be ignored?
  if ( m_ignore.end() != std::find( m_ignore.begin(), m_ignore.end(), error.code ) ) {
    return sc;
  }
  // invoke all handlers
  for ( auto handler = m_handlers.begin(); sc.isSuccess() && m_handlers.end() != handler; ++handler ) {
    sc = ( *handler )->handle( error );
  }
  //
  return sc;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

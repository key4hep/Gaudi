// $Id: GslSvc.cpp,v 1.2 2006/01/10 20:00:05 hmd Exp $
// Include files
// from Gaudi
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
// STD & STL
#include <algorithm>
#include <functional>
// GaudiGSL
#include "GaudiGSL/IGslErrorHandler.h"
#include "GaudiGSL/GslError.h"
// local
#include "GaudiGSL/GaudiGSL.h"
#include "GslSvc.h"
#include "GaudiGSL/GslErrorHandlers.h"
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

// ============================================================================
/** mandatory static factory for service instantiation
 *  @see  SvcFactory
 *  @see ISvcFactory
 *  @see    IFactory
 */
// ============================================================================
DECLARE_SERVICE_FACTORY(GslSvc)
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @param name service name
 *  @param scv  pointer to service locator
 */
// ============================================================================
GslSvc::GslSvc( const std::string& name ,
                ISvcLocator*       svc  )
  : base_class ( name , svc )
  , m_errorPolicy       ( "GSL" )
  , m_handlersTypeNames ()
  , m_handlers          ()
  , m_ignore            ()
{
  declareProperty( "ErrorPolicy" , m_errorPolicy       ) ;
  declareProperty( "Handlers"    , m_handlersTypeNames ) ;
  declareProperty( "IgnoreCodes" , m_ignore            ) ;
}
// ============================================================================

// ============================================================================
/// destructor
// ============================================================================
GslSvc::~GslSvc() {}
// ============================================================================

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
  MsgStream log( msgSvc() , name() );
  if( sc.isFailure() )
    { log << MSG::ERROR
          << " Error in initialization of base class 'Service'"<< endmsg;
    return sc;
    }
  // activate the static accessor to the service
  GaudiGSL::setGslSvc( this );
  // set the error handlers
  if      ( "GSL"       == m_errorPolicy ) { /* nothing to do */ }
  else if ( "Off"       == m_errorPolicy )
    { gsl_set_error_handler_off()                                ; }
  else if ( "Abort"     == m_errorPolicy )
    { gsl_set_error_handler ( 0 )                                ; }
  else if ( "Ignore"    == m_errorPolicy )
    { gsl_set_error_handler ( GslErrorHandlers::ignoreTheError ) ; }
  else if ( "Exception" == m_errorPolicy )
    { gsl_set_error_handler ( GslErrorHandlers::throwException ) ; }
  else if ( "Handle"    == m_errorPolicy )
    { gsl_set_error_handler ( GslErrorHandlers::handleTheError ) ; }
  else
    { log << MSG::ERROR
          << " Unknown Error policy '" << m_errorPolicy << "'"
          << " Valid policies: "
          << "[ 'GSL' , 'Off' , 'Abort' , 'Ignore' , 'Exception' , 'Handle' ]"
          << endmsg;
    return StatusCode::FAILURE ;
    }
  /// Prints the type of used handler
  /// get the handler
  GslErrorHandler handler = gsl_set_error_handler( 0 );
  gsl_set_error_handler( handler );
  if( 0 != handler )
    { log << MSG::VERBOSE
          << " GSL Error Handler is '"
          << System::typeinfoName( typeid(*handler) ) << "'"
          << endmsg; }
  else { log << MSG::INFO << " Error Handler is NULL" << endmsg ; }

  if( !m_handlersTypeNames.empty() )
    {
      /// Get Tool Service
      SmartIF<IToolSvc> toolsvc(serviceLocator()->service("ToolSvc"));
      if (!toolsvc.isValid()) {
        log << MSG::ERROR << " Could not locate Tool Service! " << endmsg;
        return StatusCode::FAILURE;
      }
      for( Names::const_iterator it = m_handlersTypeNames.begin() ;
           m_handlersTypeNames.end() != it ; ++it )
        {
          std::string::const_iterator ipos =
            std::find( it->begin() , it->end() , '/');
          const std::string::size_type pos = ipos - it->begin() ;
          IGslErrorHandler* eh = 0 ;
          if( it->end() != ipos )
            { sc = toolsvc->retrieveTool
                ( std::string( *it , 0 , pos )       ,
                  std::string( *it , pos + 1, it->length() ), eh , this ) ; }
          else
            { sc = toolsvc->retrieveTool
                ( *it , std::string( *it , pos + 1, it->length() ) ,
                  eh , this ) ; }
          if( sc.isFailure() )
            { log << MSG::ERROR
                  << " Could not retrieve tool '" << *it << "'"<< endmsg ;
            return sc ; }
          if( 0 == eh )
            { log << MSG::ERROR
                  << " Could not retrieve tool '" << *it << "'"<< endmsg ;
            return StatusCode::FAILURE  ; }
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
  MsgStream log(msgSvc(), name());
  log << MSG::DEBUG << "==> Finalize" << endmsg;

  // deactivate the static accessor to the service
  GaudiGSL::setGslSvc( 0 );

  // finalize the base class
  return Service::finalize() ;
}
// ============================================================================

// ============================================================================
/** retrieve the  current GSL error handler
 *  @return current GSL error handler
 */
// ============================================================================
IGslSvc::GslErrorHandler GslSvc::handler ()    const
{
  GslErrorHandler hh = gsl_set_error_handler( 0 );
  gsl_set_error_handler( hh );
  return hh ;
}
// ============================================================================

// ============================================================================
/** set new GSL error handler
 *  @param  handler   new GSL error handler
 *  @return GSL error handler
 */
// ============================================================================
IGslSvc::GslErrorHandler GslSvc::setHandler
( IGslSvc::GslErrorHandler handler ) const
{
  gsl_set_error_handler( handler );
  {
    MsgStream log( msgSvc(), name() );
    log << MSG::DEBUG << " New GSL handler is set '" ;
    if( 0 == handler ) { log << "NULL"                                  ; }
    else               { log << System::typeinfoName( typeid(handler) ) ; }
    log << "'" << endmsg ;
  }
  return handler ;
}
// ============================================================================

// ============================================================================
/** transform GSL error code to Gaudi status code
 *  @param  error GLS error code
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::status        ( const int error         ) const
{
  if( GSL_SUCCESS == error ){ return StatusCode::SUCCESS ; }
  StatusCode sc( error );
  if( sc.isSuccess()       ){ return StatusCode::FAILURE ; }
  return sc ;
}
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @param error error
 *  @return status code
 */
// ============================================================================
StatusCode GslSvc::handle
( const GslError& error ) const
{
  StatusCode sc = StatusCode::SUCCESS ;
  // code to be ignored?
  if( m_ignore.end() != std::find( m_ignore.begin () ,
                                   m_ignore.end   () ,
                                   error.code        ) ) { return sc ; }
  // invoke all handlers
  for( Handlers::const_iterator handler = m_handlers.begin() ;
       sc.isSuccess() && m_handlers.end() != handler ; ++handler )
    { sc = (*handler)->handle( error  ); }
  //
  return sc ;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================




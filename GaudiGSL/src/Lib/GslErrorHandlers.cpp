// $Id: GslErrorHandlers.cpp,v 1.1 2003/11/19 16:55:59 mato Exp $
// ============================================================================
//
// ============================================================================
// Include files
// GaudiKernel
#include "GaudiKernel/GaudiException.h"
// GaudiGSL
#include "GaudiGSL/IGslSvc.h"
#include "GaudiGSL/GslError.h"
// local
#include "GaudiGSL/GslErrorHandlers.h"
#include "GaudiGSL/GaudiGSL.h"

#include <sstream>

/** @file
 *
 *  Implementation file for functionns form GslErrorHandlers namespace
 *  @see GslErrorHandlers
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   29/04/2002
 */

void GslErrorHandlers::ignoreTheError
( const char* /* reason */ ,
  const char* /* file   */ ,
  int         /* line   */ ,
  int         /* code   */ )
{}

void GslErrorHandlers::handleTheError
( const char* reason ,
  const char* file   ,
  int         line   ,
  int         code   )
{
  // het the GSL service
  const IGslSvc* svc = GaudiGSL::gslSvc() ;
  // handle the error if service is valid
  if( 0 != svc ) { svc->handle( GslError( reason , file , line , code ) ) ; }
}

void GslErrorHandlers::throwException
( const char* reason ,
  const char* file   ,
  int         line   ,
  int         code   )
{
  std::ostringstream error;
  error << " GSL ErrorCode=" << code << ": '" << reason
        << "' in the file '" << file << "' at the line " << line;
  throw GaudiException( error.str() , "*GLS Error*" , StatusCode::FAILURE );
}

// ============================================================================
// The END
// ============================================================================

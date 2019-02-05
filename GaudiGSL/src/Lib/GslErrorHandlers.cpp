// ============================================================================
// Include files
// GaudiKernel
#include "GaudiKernel/GaudiException.h"
// GaudiGSL
#include "GaudiGSL/GslError.h"
#include "GaudiGSL/IGslSvc.h"
// local
#include "GaudiGSL/GaudiGSL.h"
#include "GaudiGSL/GslErrorHandlers.h"

#include <sstream>

/** @file
 *
 *  Implementation file for functionns form GslErrorHandlers namespace
 *  @see GslErrorHandlers
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   29/04/2002
 */

void GslErrorHandlers::ignoreTheError( const char* /* reason */, const char* /* file   */, int /* line   */,
                                       int /* code   */ ) {}

void GslErrorHandlers::handleTheError( const char* reason, const char* file, int line, int code ) {
  // het the GSL service
  const IGslSvc* svc = GaudiGSL::gslSvc();
  // handle the error if service is valid
  if ( nullptr != svc ) { svc->handle( GslError( reason, file, line, code ) ); }
}

void GslErrorHandlers::throwException( const char* reason, const char* file, int line, int code ) {
  std::ostringstream error;
  error << " GSL ErrorCode=" << code << ": '" << reason << "' in the file '" << file << "' at the line " << line;
  throw GaudiException( error.str(), "*GLS Error*", StatusCode::FAILURE );
}

// ============================================================================
// The END
// ============================================================================

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
};

void GslErrorHandlers::throwException 
( const char* reason , 
  const char* file   , 
  int         line   , 
  int         code   )
{
  std::string error( " GSL ErrorCode=" );
  static char s_aux[512];
  error += std::string( s_aux  , s_aux  + sprintf( s_aux , "%d" , code  ) );
  error += ": '"   ;
  error += reason  ;
  error += "' in the file '" ;
  error += file    ;
  error += "' at the line "  ;
  error += std::string( s_aux  , s_aux  + sprintf( s_aux , "%d" , line ) );
  error += "'"     ;
  throw GaudiException( error , "*GLS Error*" , StatusCode::FAILURE );
};

// ============================================================================
// The END 
// ============================================================================

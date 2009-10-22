// $Id: GslErrorException.cpp,v 1.2 2006/01/10 20:00:05 hmd Exp $
// ============================================================================
// ============================================================================
// Include files
// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"
// STD & STL
#include <algorithm>
// local
#include "GslErrorException.h"

// ============================================================================
/** @file
 *
 *  Implementation file for class GslErrorException
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */
// ============================================================================

// ============================================================================
/** Declaration of the Tool Factory
 *  @see  ToolFactory
 *  @see IToolFactory
 *  @see     IFactory
 */
// ============================================================================
DECLARE_TOOL_FACTORY(GslErrorException)
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @param type   tool type (?)
 *  @param name   tool name
 *  @param parent pointer to parent
 */
// ============================================================================
GslErrorException::GslErrorException
( const std::string& type   ,
  const std::string& name   ,
  const IInterface*  parent )
  : base_class ( type, name , parent )
  , m_ignore ()
{
  declareProperty ( "IgnoreCodes" , m_ignore );
};
// ============================================================================

// ============================================================================
/// destructor (protetced and virtual)
// ============================================================================
GslErrorException::~GslErrorException(){};
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @see IGslErrorHandler
 *  @param error  error to be handled
 *  @see GslError
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorException::handle
( const GslError& error  ) const
{
  StatusCode sc = StatusCode::SUCCESS ;
  // code to be ignored?
  if( m_ignore.end() != std::find( m_ignore.begin () ,
                                   m_ignore.end   () ,
                                   error.code        ) ) { return sc ; }
  //
  std::string message( " GSL ErrorCode=" );
  static char s_aux[512];
  message +=
    std::string( s_aux  , s_aux  + sprintf( s_aux , "%d" , error.code  ) );
  message += ": '"   ;
  message += error.reason  ;
  message += "' in the file '" ;
  message += error.file    ;
  message += "' at the line "  ;
  message +=
    std::string( s_aux  , s_aux  + sprintf( s_aux , "%d" , error.line ) );
  message += "'"     ;
  throw GaudiException( message , "*GLS Error*" , StatusCode::FAILURE );
  ///
  return StatusCode::SUCCESS ;
};
// ============================================================================


// ============================================================================
// The END
// ============================================================================

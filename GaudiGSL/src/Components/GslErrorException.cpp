// ============================================================================
// Include files
// from Gaudi
#include "GaudiKernel/GaudiException.h"
// STD & STL
#include <algorithm>
#include <string>
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
/** handle the GSL error
 *  @see IGslErrorHandler
 *  @param error  error to be handled
 *  @see GslError
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorException::handle( const GslError& error ) const
{
  // throw if code is not in the list of codes to be ignored
  if ( m_ignore.end() == std::find( m_ignore.begin(), m_ignore.end(), error.code ) ) {
    throw GaudiException( " GSL ErrorCode=" + std::to_string( error.code ) + ": '" + error.reason + "' in the file '" +
                              error.file + "' at line " + std::to_string( error.line ),
                          "*GLS Error*", StatusCode::FAILURE );
  }
  ///
  return StatusCode::SUCCESS;
}
// ============================================================================

DECLARE_COMPONENT( GslErrorException )

// ============================================================================
// The END
// ============================================================================

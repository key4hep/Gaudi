// Include files
// from Gaudi
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/Stat.h"
// local
#include "GslErrorPrint.h"

// ============================================================================
/** @file
 *
 *  Implementation file for class GslErrorPrint
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */
// ============================================================================
DECLARE_COMPONENT( GslErrorPrint )
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @see IGslErrorHandler
 *  @param error  error to be handled
 *  @see GslError
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorPrint::handle( const GslError& error ) const
{
  AlgTool::error() << " GSL code " << error.code << " Message '" << error.reason << "'"
                   << " File '" << error.file << "'"
                   << " Line " << error.line << endmsg;
  //
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

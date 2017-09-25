// Include files
// from Gaudi
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Stat.h"
// local
#include "GslErrorCount.h"

// ============================================================================
/** @file
 *
 *  Implementation file for class GslErrorCount
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   30/04/2002
 */
// ============================================================================
DECLARE_COMPONENT( GslErrorCount )
// ============================================================================

// ============================================================================
/** standard finalization of Tool
 *  @see  AlgTool
 *  @see IAlgTool
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorCount::finalize()
{
  // printout the Error table
  const std::string stars( 78, '*' );
  info() << stars << endmsg;
  AlgTool::error() << m_counters.size() << " GSL errors handled" << endmsg;
  for ( const auto& error : m_counters ) {
    AlgTool::error() << " #times " << error.second << " GSL code " << error.first.code << " Message '"
                     << error.first.reason << "'"
                     << " File '" << error.first.file << "'"
                     << " Line " << error.first.line << endmsg;
  }
  info() << stars << endmsg;
  // clear the counters
  m_counters.clear();
  // finalize the base class
  return AlgTool::finalize();
}
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @see IGslErrorHandler
 *  @param error  error to be handled
 *  @see GslError
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorCount::handle( const GslError& error ) const
{
  ++m_counters[error];
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

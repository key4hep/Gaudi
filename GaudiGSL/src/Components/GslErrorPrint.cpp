// $Id: GslErrorPrint.cpp,v 1.2 2006/01/10 20:00:05 hmd Exp $
// ============================================================================
// Include files
// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
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

// ============================================================================
/** Declaration of the Tool Factory
 *  @see  ToolFactory
 *  @see IToolFactory
 *  @see     IFactory
 */
// ============================================================================
DECLARE_COMPONENT(GslErrorPrint)
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @param type   tool type (?)
 *  @param name   tool name
 *  @param parent pointer to parent
 */
// ============================================================================
GslErrorPrint::GslErrorPrint
( const std::string& type   ,
  const std::string& name   ,
  const IInterface*  parent )
  : base_class ( type, name , parent )
{}
// ============================================================================

// ============================================================================
/// destructor (protetced and virtual)
// ============================================================================
GslErrorPrint::~GslErrorPrint(){}
// ============================================================================

// ============================================================================
/** handle the GSL error
 *  @see IGslErrorHandler
 *  @param error  error to be handled
 *  @see GslError
 *  @return status code
 */
// ============================================================================
StatusCode GslErrorPrint::handle
( const GslError& error  ) const
{
  MsgStream log( msgSvc() , name() );
  log << MSG::ERROR
      << " GSL code " << error.code
      << " Message '" << error.reason << "'"
      << " File '"    << error.file   << "'"
      << " Line "     << error.line   << endmsg ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================


// ============================================================================
// The END
// ============================================================================

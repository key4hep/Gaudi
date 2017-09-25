// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/StatEntity.h"
// ============================================================================
/** @file
 *  Implementation file with helper methods for interface ICounterSvc
 *  @date 2007-05-25
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 */
// ============================================================================
// Callback for printout with Counter pointers
// ============================================================================
StatusCode ICounterSvc::Printout::operator()( MsgStream& log, const Counter* cnt ) const
{
  return m_svc ? m_svc->defaultPrintout( log, cnt ) : StatusCode::FAILURE;
}
// ============================================================================
// Standard initializing constructor
// ============================================================================
ICounterSvc::Printout::Printout( ICounterSvc* svc ) : m_svc( svc ) {}

// ============================================================================
// The END
// ============================================================================

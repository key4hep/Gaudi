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

namespace
{
  struct ICounterSvcCategory : StatusCode::Category {
    const char* name() const override { return "ICounterSvc"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override
    {
      switch ( static_cast<ICounterSvc::Status>( code ) ) {
      case ICounterSvc::Status::COUNTER_NOT_PRESENT:
        return "COUNTER_NOT_PRESENT";
      case ICounterSvc::Status::COUNTER_EXISTS:
        return "COUNTER_EXISTS";
      case ICounterSvc::Status::COUNTER_REMOVED:
        return "COUNTER_REMOVED";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
}

STATUSCODE_ENUM_IMPL( ICounterSvc::Status, ICounterSvcCategory )

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

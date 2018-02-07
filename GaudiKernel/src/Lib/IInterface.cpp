#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"

namespace
{
  struct IInterfaceCategory : StatusCode::Category {
    const char* name() const override { return "IInterface"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override
    {
      switch ( static_cast<IInterface::Status>( code ) ) {
      case IInterface::Status::NO_INTERFACE:
        return "NO_INTERFACE";
      case IInterface::Status::VERSMISMATCH:
        return "VERSMISMATCH";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
}

STATUSCODE_ENUM_IMPL( IInterface::Status, IInterfaceCategory )

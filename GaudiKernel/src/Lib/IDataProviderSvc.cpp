#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/StatusCode.h"

namespace
{
  struct IDataProviderSvcCategory : StatusCode::Category {
    const char* name() const override { return "IDataProviderSvc"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override
    {
      switch ( static_cast<IDataProviderSvc::Status>( code ) ) {
      case IDataProviderSvc::Status::DOUBL_OBJ_PATH:
        return "DOUBL_OBJ_PATH";
      case IDataProviderSvc::Status::INVALID_OBJ_PATH:
        return "INVALID_OBJ_PATH";
      case IDataProviderSvc::Status::INVALID_ROOT:
        return "INVALID_ROOT";
      case IDataProviderSvc::Status::INVALID_OBJECT:
        return "INVALID_OBJECT";
      case IDataProviderSvc::Status::INVALID_PARENT:
        return "INVALID_PARENT";
      case IDataProviderSvc::Status::OBJ_NOT_LOADED:
        return "OBJ_NOT_LOADED";
      case IDataProviderSvc::Status::NO_DATA_LOADER:
        return "NO_DATA_LOADER";
      case IDataProviderSvc::Status::INVALID_OBJ_ADDR:
        return "INVALID_OBJ_ADDR";
      case IDataProviderSvc::Status::DIR_NOT_EMPTY:
        return "DIR_NOT_EMPTY";
      case IDataProviderSvc::Status::NO_MORE_LEVELS:
        return "NO_MORE_LEVELS";
      case IDataProviderSvc::Status::NO_ACCESS:
        return "NO_ACCESS";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
}

STATUSCODE_ENUM_IMPL( IDataProviderSvc::Status, IDataProviderSvcCategory )

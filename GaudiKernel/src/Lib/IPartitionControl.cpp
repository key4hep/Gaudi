#include "GaudiKernel/IPartitionControl.h"
#include "GaudiKernel/StatusCode.h"

namespace {
  struct IPartitionControlCategory : StatusCode::Category {
    const char* name() const override { return "IPartitionControl"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<IPartitionControl::Status>( code ) ) {
      case IPartitionControl::Status::PARTITION_NOT_PRESENT:
        return "PARTITION_NOT_PRESENT";
      case IPartitionControl::Status::PARTITION_EXISTS:
        return "PARTITION_EXISTS";
      case IPartitionControl::Status::NO_ACTIVE_PARTITION:
        return "NO_ACTIVE_PARTITION";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
} // namespace

STATUSCODE_ENUM_IMPL( IPartitionControl::Status, IPartitionControlCategory )

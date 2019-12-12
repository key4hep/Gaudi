/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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

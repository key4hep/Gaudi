/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/StatusCode.h>

namespace {
  struct IInterfaceCategory : StatusCode::Category {
    const char* name() const override { return "IInterface"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override {
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
} // namespace

STATUSCODE_ENUM_IMPL( IInterface::Status, IInterfaceCategory )

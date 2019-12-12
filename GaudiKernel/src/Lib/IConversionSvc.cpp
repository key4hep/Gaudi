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
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/StatusCode.h"

namespace {
  struct IConvSvcCategory : StatusCode::Category {
    const char* name() const override { return "IConversionSvc"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<IConversionSvc::Status>( code ) ) {
      case IConversionSvc::Status::INVALID_ADDRESS:
        return "INVALID_ADDRESS";
      case IConversionSvc::Status::INVALID_OBJECT:
        return "INVALID_OBJECT";
      case IConversionSvc::Status::NO_MEMORY:
        return "NO_MEMORY";
      case IConversionSvc::Status::BAD_STORAGE_TYPE:
        return "BAD_STORAGE_TYPE";
      case IConversionSvc::Status::NO_SOURCE_OBJECT:
        return "NO_SOURCE_OBJECT";
      case IConversionSvc::Status::NO_CONVERTER:
        return "NO_CONVERTER";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
} // namespace

STATUSCODE_ENUM_IMPL( IConversionSvc::Status, IConvSvcCategory )

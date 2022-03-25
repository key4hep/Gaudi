/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define GAUDIKERNEL_STATUSCODE_CPP 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include <exception>

constexpr const StatusCode::ErrorCode StatusCode::SUCCESS;
constexpr const StatusCode::ErrorCode StatusCode::FAILURE;
constexpr const StatusCode::ErrorCode StatusCode::RECOVERABLE;

namespace {
  /// Default StatusCode category
  struct DefaultCategory : public StatusCode::Category {

    const char* name() const override { return "Gaudi"; }

    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<StatusCode::ErrorCode>( code ) ) {
      case StatusCode::ErrorCode::SUCCESS:
        return "SUCCESS";
      case StatusCode::ErrorCode::FAILURE:
        return "FAILURE";
      case StatusCode::ErrorCode::RECOVERABLE:
        return "RECOVERABLE";
      default:
        return "UNKNOWN(" + std::to_string( code ) + ")";
      }
    }
  };
} // namespace

STATUSCODE_ENUM_IMPL( StatusCode::ErrorCode, DefaultCategory )

void StatusCode::i_doThrow( std::string_view message, std::string_view tag ) const {
  throw GaudiException{ std::string{ message }, std::string{ tag }, *this };
}

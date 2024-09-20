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
#include <GaudiKernel/FunctionalFilterDecision.h>

namespace Gaudi::Functional {

  struct FilterDecisionErrorCategory : StatusCode::Category {
    const char* name() const override { return "FilterDecision"; }
    bool        isSuccess( StatusCode::code_t code ) const override {
      return ( static_cast<FilterDecision>( code ) == FilterDecision::PASSED or
               static_cast<FilterDecision>( code ) == FilterDecision::FAILED );
    }
    bool        isRecoverable( StatusCode::code_t ) const override { return false; }
    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<FilterDecision>( code ) ) {
      case FilterDecision::PASSED:
        return "Filter decision: PASSED";
      case FilterDecision::FAILED:
        return "Filter decision: FAILED";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
} // namespace Gaudi::Functional

STATUSCODE_ENUM_IMPL( Gaudi::Functional::FilterDecision, Gaudi::Functional::FilterDecisionErrorCategory )

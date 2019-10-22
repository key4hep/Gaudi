#pragma once

#include "GaudiKernel/StatusCode.h"

namespace Gaudi::Functional {
  // define error enum / category
  enum class FilterDecision : StatusCode::code_t {
    PASSED = 123456,
    FAILED = 654321,
  };
} // namespace Gaudi::Functional

STATUSCODE_ENUM_DECL( Gaudi::Functional::FilterDecision )

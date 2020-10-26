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

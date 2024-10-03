/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Rust/AlgWrapper.h>
#include <rust/cxx.h>

namespace Gaudi::Rust::helpers {
  inline rust::String getPropertyValueHelper( AlgWrapper const& alg, std::string const& name ) {
    return alg.getPropertyValue( name );
  }
} // namespace Gaudi::Rust::helpers

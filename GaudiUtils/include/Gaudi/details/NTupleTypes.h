/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <cstddef>
#include <string>
#include <typeindex>

/// Type lookups by name for the generic ntuple writers, based on the ROOT dictionaries

namespace Gaudi::details {

  /// std::type_index of a fundamental type or of a class with a compiled dictionary
  std::type_index typeIndexOf( const std::string& typeName );

  struct VectorElement {
    std::string type;
    std::size_t size;
  };

  /// element type and size of a std::vector, throws if the elements can't be written one by one
  VectorElement vectorElementOf( const std::string& vectorType );

} // namespace Gaudi::details

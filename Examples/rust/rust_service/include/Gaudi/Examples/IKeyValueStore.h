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

#include <GaudiKernel/IInterface.h>
#include <optional>
#include <string>
#include <string_view>

namespace Gaudi::Examples {
  /// @brief Interface for a basic read-only key-value store
  ///
  /// A service implementing this interface will get the data from some storage
  /// and allow users to retrieve simple string values from it.
  struct IKeyValueStore : virtual IInterface {
    // InterfaceID declaration.
    DeclareInterfaceID( IKeyValueStore, 1, 0 );

    /// Return the value associated with the given key or nullopt if not found.
    virtual std::optional<std::string> get( std::string_view key ) const = 0;
  };
} // namespace Gaudi::Examples

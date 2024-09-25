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
#include <Gaudi/Examples/IKeyValueStore.h>
#include <Gaudi/Rust/ServiceBridge.h>
#ifdef GAUDI_TEST_PUBLIC_HEADERS_BUILD
#  include <string>
struct OptString final {
  std::string value;
  bool        is_set;
};
namespace rust {
  struct Str {
    const char* data;
    std::size_t size;
  };
} // namespace rust
#else
#  include <example_rust_service_bridge/lib.h>
#endif

namespace Gaudi::Rust {
  std::optional<std::string> to_optional( const OptString& value ) {
    if ( value.is_set ) {
      return std::string( value.value );
    } else {
      return std::nullopt;
    }
  }

  /// Helper class to wrap a Rust implementation of IKeyValueStore interface
  template <typename T>
  class IKeyValueStoreBridge : public extends<ServiceBridge<T>, Gaudi::Examples::IKeyValueStore> {
  public:
    using extends<ServiceBridge<T>, Gaudi::Examples::IKeyValueStore>::extends;

    // delegation ot IKeyValueStore methods
    // taking in to account the custom wrapping of std::optional
    std::optional<std::string> get( std::string_view key ) const override {
      return to_optional( this->m_impl->get( rust::Str( key.data(), key.size() ) ) );
    }
  };
} // namespace Gaudi::Rust

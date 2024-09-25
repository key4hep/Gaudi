/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Examples/IKeyValueStore.h>
#include <GaudiKernel/Service.h>
#include <example_rust_service_bridge/lib.h>

namespace {
  /// Helper to simplify the conversion from OptString to std::optional<std::string>
  std::optional<std::string> to_optional( const OptString& value ) {
    if ( value.is_set ) {
      return std::string( value.value );
    } else {
      return std::nullopt;
    }
  }

  /// Helper class to wrap a Rust implementation of IKeyValueStore interface
  template <typename T>
  class IKeyValueStoreBridge : public extends<Service, Gaudi::Examples::IKeyValueStore> {
  public:
    using extends::extends;

    // delegation of IStateful methods
    StatusCode initialize() override {
      return extends::initialize().andThen( [this] { m_impl->initialize(); } );
    }
    StatusCode start() override {
      return extends::start().andThen( [this] { m_impl->start(); } );
    }
    StatusCode stop() override {
      m_impl->stop();
      return extends::stop();
    }
    StatusCode finalize() override {
      m_impl->finalize();
      return extends::finalize();
    }

    // delegation ot IKeyValueStore methods
    // taking in to account the custom wrapping of std::optional
    std::optional<std::string> get( std::string_view key ) const override {
      return to_optional( m_impl->get( rust::Str( key.data(), key.size() ) ) );
    }

  private:
    // The implementation from rust is initialized from the C++ service instance to have
    // access to the C++ service methods (like name, etc.)
    rust::Box<T> m_impl = make_dummy_kvs( *this );
  };
} // namespace

// Make the bridge class available to Gaudi as a component
DECLARE_COMPONENT_WITH_ID( IKeyValueStoreBridge<DummyKvs>, "Gaudi::Examples::Rust::KeyValueStore" )

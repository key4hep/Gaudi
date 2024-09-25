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
  template <typename T>
  class RustServiceBridge : public extends<Service, Gaudi::Examples::IKeyValueStore> {
  public:
    using extends::extends;
    // RustServiceBridge(const std::string& name, ISvcLocator* svcLoc ) : extends(name, svcLoc) {
    //     m_impl->set_name(name);
    // }

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

    std::optional<std::string> get( std::string_view key ) const override {
      auto result = m_impl->get_wrapper( rust::Str( key.data(), key.size() ) );
      if ( result.found ) {
        return std::string( result.value );
      } else {
        return std::nullopt;
      }
    }

  private:
    rust::Box<T> m_impl = make_kvs( *this );
  };
} // namespace

DECLARE_COMPONENT_WITH_ID( RustServiceBridge<DummyKvs>, "Gaudi::Examples::Rust::KeyValueStore" )

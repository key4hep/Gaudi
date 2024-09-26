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
#include <Gaudi/Rust/IKeyValueStoreBridge.h>
#include <example_rust_service_bridge/lib.h>

namespace Gaudi::Rust {
  template <>
  struct ServiceBuilder<DummyKvs> {
    static rust::Box<DummyKvs> make( Service const& service ) { return make_dummy_kvs( service ); }
  };
} // namespace Gaudi::Rust

// Make the bridge class available to Gaudi as a component
DECLARE_COMPONENT_WITH_ID( Gaudi::Rust::IKeyValueStoreBridge<DummyKvs>, "Gaudi::Examples::Rust::KeyValueStore" )

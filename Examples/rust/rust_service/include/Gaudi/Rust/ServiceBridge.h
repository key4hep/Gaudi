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
#include <GaudiKernel/Service.h>
#ifdef GAUDI_TEST_PUBLIC_HEADERS_BUILD
namespace rust {
  template <typename T>
  struct Box;
}
#else
#  include <rust/cxx.h>
#endif

namespace Gaudi::Rust {
  template <typename T>
  struct ServiceBuilder {
    static rust::Box<T> make( Service const& service ) { static_assert( false, "No builder for this type" ); }
  };

  template <typename T>
  class ServiceBridge : public Service {
  public:
    using Service::Service;

    // delegation of IStateful methods
    StatusCode initialize() override {
      return Service::initialize().andThen( [this] { m_impl->initialize(); } );
    }
    StatusCode start() override {
      return Service::start().andThen( [this] { m_impl->start(); } );
    }
    StatusCode stop() override {
      m_impl->stop();
      return Service::stop();
    }
    StatusCode finalize() override {
      m_impl->finalize();
      return Service::finalize();
    }

  protected:
    // The implementation from Rust is initialized from the C++ service instance to have
    // access to the C++ service methods (like name, etc.)
    rust::Box<T> m_impl = ServiceBuilder<T>::make( *this );
  };
} // namespace Gaudi::Rust

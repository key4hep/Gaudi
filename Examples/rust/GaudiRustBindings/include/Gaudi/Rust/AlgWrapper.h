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

#include <Gaudi/Algorithm.h>
#include <mutex>

#if __has_include( <rust/cxx.h>)
#  include <rust/cxx.h>
#else
namespace rust {
  template <typename T>
  class Box;
}
#endif

namespace Gaudi::Rust {
  namespace details {
    // forward declarations
    struct WrappedAlg;
    void alg_initialize( ::rust::Box<WrappedAlg>& alg, Gaudi::Algorithm const& host );
    void alg_start( ::rust::Box<WrappedAlg>& alg, Gaudi::Algorithm const& host );
    void alg_stop( ::rust::Box<WrappedAlg>& alg, Gaudi::Algorithm const& host );
    void alg_finalize( ::rust::Box<WrappedAlg>& alg, Gaudi::Algorithm const& host );
    void alg_execute( ::rust::Box<WrappedAlg> const& alg, Gaudi::Algorithm const& host, EventContext const& ctx );

    /// Gaudi::Algorithm specialization that wraps an algorithm implemented in Rust.
    ///
    /// An algorithm implemented in Rust will have to provide a specialization of
    /// this class that implements the `instantiate_alg` method so that it calls
    /// the appropriate factory function from Rust.
    template <typename = void>
    class AlgWrapper : public Gaudi::Algorithm {
    public:
      using impl_t = ::rust::Box<WrappedAlg>;

      AlgWrapper( std::string const& name, ISvcLocator* svcLoc, impl_t&& impl ) : m_impl( std::move( impl ) ) {}

      StatusCode initialize() override {
        return Algorithm::initialize().andThen( [&]() { alg_initialize( m_impl, *this ); } );
      }
      StatusCode start() override {
        return Algorithm::start().andThen( [&]() { alg_start( m_impl, *this ); } );
      }
      StatusCode execute( const EventContext& ctx ) const override {
        alg_execute( m_impl, *this, ctx );
        return StatusCode::SUCCESS; // alg_execute(...) throws on error
      }
      StatusCode stop() override {
        alg_stop( m_impl, *this );
        return Algorithm::stop();
      }
      StatusCode finalize() override {
        alg_finalize( m_impl, *this );
        return Algorithm::finalize();
      }

    private:
      impl_t m_impl;
    };
  } // namespace details
  using AlgWrapper = details::AlgWrapper<>;
} // namespace Gaudi::Rust

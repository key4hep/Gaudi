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
    void alg_initialize( ::rust::Box<WrappedAlg>& alg );
    void alg_start( ::rust::Box<WrappedAlg>& alg );
    void alg_stop( ::rust::Box<WrappedAlg>& alg );
    void alg_finalize( ::rust::Box<WrappedAlg>& alg );
    void alg_execute( ::rust::Box<WrappedAlg> const& alg, EventContext const& ctx );

    /// Gaudi::Algorithm specialization that wraps an algorithm implemented in Rust.
    ///
    /// An algorithm implemented in Rust will have to provide a specialization of
    /// this class that implements the `instantiate_alg` method so that it calls
    /// the appropriate factory function from Rust.
    template <typename = void>
    class AlgWrapper : public Gaudi::Algorithm {
    public:
      using impl_t = rust::Box<WrappedAlg>;

      using Algorithm::Algorithm;
      ~AlgWrapper() {
        if ( m_impl ) delete m_impl;
      }

      StatusCode initialize() override {
        return Algorithm::initialize().andThen( [&]() { alg_initialize( impl() ); } );
      }
      StatusCode start() override {
        return Algorithm::start().andThen( [&]() { alg_start( impl() ); } );
      }
      StatusCode execute( const EventContext& ctx ) const override {
        alg_execute( impl(), ctx );
        return StatusCode::SUCCESS; // alg_execute(impl()) throws on error
      }
      StatusCode stop() override {
        alg_stop( impl() );
        return Algorithm::stop();
      }
      StatusCode finalize() override {
        alg_finalize( impl() );
        return Algorithm::finalize();
      }

    protected:
      virtual impl_t* factory() const = 0;

      impl_t& impl() {
        std::lock_guard<std::mutex> lock( m_impl_mutex );
        if ( !m_impl ) {
          m_impl = factory();
          if ( !m_impl ) { throw std::runtime_error( "Failed to instantiate algorithm" ); }
        }
        return *m_impl;
      }
      impl_t const& impl() const { return const_cast<AlgWrapper*>( this )->impl(); }

    private:
      mutable std::mutex m_impl_mutex;
      impl_t*            m_impl{ nullptr };
    };
  } // namespace details
  using AlgWrapper = details::AlgWrapper<>;
} // namespace Gaudi::Rust

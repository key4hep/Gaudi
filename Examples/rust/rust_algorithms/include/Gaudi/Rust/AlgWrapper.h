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
#include <example_rust_algorithm_bridge/lib.h>
#include <rust/cxx.h>

namespace Gaudi::Rust {
  /// Gaudi::Algorithm specialization that wraps an algorithm implemented in Rust.
  class AlgWrapper : public Gaudi::Algorithm {
  public:
    using Algorithm::Algorithm;

    StatusCode initialize() override {
      return Algorithm::initialize().andThen( [&]() { dynalg_initialize( m_impl ); } );
    }
    StatusCode start() override {
      return Algorithm::start().andThen( [&]() { dynalg_start( m_impl ); } );
    }
    StatusCode execute( const EventContext& ctx ) const override {
      dynalg_execute( m_impl, ctx );
      return StatusCode::SUCCESS; // dynalg_execute(m_impl) throws on error
    }
    StatusCode stop() override {
      dynalg_stop( m_impl );
      return Algorithm::stop();
    }
    StatusCode finalize() override {
      dynalg_finalize( m_impl );
      return Algorithm::finalize();
    }

  protected:
    rust::Box<DynAlg> m_impl = rust::Box<DynAlg>::from_raw( nullptr );
  };
} // namespace Gaudi::Rust

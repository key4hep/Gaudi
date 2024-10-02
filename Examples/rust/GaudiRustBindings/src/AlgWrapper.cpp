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
#include <Gaudi/Rust/AlgWrapper.h>
#include <gaudi_rust_bindings_bridge/lib.h>

namespace Gaudi::Rust {

  AlgWrapper::AlgWrapper( std::string const& name, ISvcLocator* svcLoc, details::WrappedAlg* dyn_alg_ptr )
      : Algorithm( name, svcLoc ), m_dyn_alg_ptr( dyn_alg_ptr ) {}
  AlgWrapper::~AlgWrapper() {
    if ( m_dyn_alg_ptr ) alg_drop( m_dyn_alg_ptr );
  }

  StatusCode AlgWrapper::initialize() {
    return Algorithm::initialize().andThen( [&]() { alg_initialize( m_dyn_alg_ptr, *this ); } );
  }
  StatusCode AlgWrapper::start() {
    return Algorithm::start().andThen( [&]() { alg_start( m_dyn_alg_ptr, *this ); } );
  }
  StatusCode AlgWrapper::execute( const EventContext& ctx ) const {
    alg_execute( m_dyn_alg_ptr, *this, ctx );
    return StatusCode::SUCCESS; // alg_execute(...) throws on error
  }
  StatusCode AlgWrapper::stop() {
    alg_stop( m_dyn_alg_ptr, *this );
    return Algorithm::stop();
  }
  StatusCode AlgWrapper::finalize() {
    alg_finalize( m_dyn_alg_ptr, *this );
    return Algorithm::finalize();
  }

} // namespace Gaudi::Rust

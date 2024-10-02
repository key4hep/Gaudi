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

namespace Gaudi::Rust {
  namespace details {
    // forward declarations
    struct WrappedAlg;

    /// Gaudi::Algorithm specialization that wraps an algorithm implemented in Rust.
    ///
    /// An algorithm implemented in Rust will have to provide a specialization of
    /// this class that implements the `instantiate_alg` method so that it calls
    /// the appropriate factory function from Rust.
    // template <typename = void>
    class AlgWrapper : public Gaudi::Algorithm {
    public:
      AlgWrapper( std::string const& name, ISvcLocator* svcLoc, WrappedAlg* dyn_alg_ptr );
      ~AlgWrapper();

      StatusCode initialize() override;
      // StatusCode start() override;
      StatusCode execute( const EventContext& ctx ) const override;

      // StatusCode execute( const EventContext& ctx ) const override;
      // StatusCode stop() override;
      // StatusCode finalize() override;

    private:
      WrappedAlg* m_dyn_alg_ptr;
    };
  } // namespace details
  using AlgWrapper = details::AlgWrapper;
} // namespace Gaudi::Rust

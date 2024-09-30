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
#include <example_rust_algorithm_bridge/lib.h>

namespace Gaudi::Examples {
  // Wrap a Rust algorithm with Gaudi::Rust::AlgWrapper
  struct MyRustCountingAlg : public Gaudi::Rust::AlgWrapper {
    using AlgWrapper::AlgWrapper;

    virtual impl_t* factory() const override { return new impl_t{ my_rust_counting_alg_factory( *this ) }; }
  };
  DECLARE_COMPONENT( MyRustCountingAlg )
} // namespace Gaudi::Examples

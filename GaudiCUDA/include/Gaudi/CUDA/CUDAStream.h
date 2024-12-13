/***********************************************************************************\
* (c) Copyright 2023-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
// Gaudi
#include <Gaudi/AsynchronousAlgorithm.h>

// CUDA
#ifndef __CUDACC__
#  include <cuda_runtime.h>
#endif

namespace Gaudi::CUDA {
  class Stream {
  private:
    cudaStream_t                        m_stream;
    const Gaudi::AsynchronousAlgorithm* m_parent;
    int                                 m_dependents;

  public:
    /// Create a new Stream. Should happen once per algorithm.
    Stream( const Gaudi::AsynchronousAlgorithm* parent );
    ~Stream();

    /// Access the internal cudaStream_t
    inline operator cudaStream_t() { return m_stream; }

    /// Access the parent algorithm
    inline const Gaudi::AsynchronousAlgorithm* parent() { return m_parent; }

    /// Yield fiber until stream is done
    StatusCode await();

    /// INTERNAL: Register a dependency
    void registerDependency() { ++m_dependents; }

    /// INTERNAL: Remove a dependency
    void removeDependency() { --m_dependents; }
  };
} // namespace Gaudi::CUDA

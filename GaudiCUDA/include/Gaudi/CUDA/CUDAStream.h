/***********************************************************************************\
* (c) Copyright 2023-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
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

// Forward declaration hack so we can have a cudaStream_t member here
struct CUstream_st;
typedef struct CUstream_st* cudaStream_t;

namespace Gaudi::CUDA {
  class Stream {
  private:
    cudaStream_t                        m_stream;
    const bool                          m_owning;
    const Gaudi::Algorithm*             m_parent;
    const Gaudi::AsynchronousAlgorithm* m_async_parent;
    int                                 m_dependents;

  public:
    /// Wrap an existing cudaStream
    Stream( const Gaudi::Algorithm* parent, cudaStream_t stream );
    /// Obtain a cudaStream from the pool
    Stream( const Gaudi::Algorithm* parent );
    ~Stream();

    /// Access the internal cudaStream_t
    operator cudaStream_t();

    /// Access the parent algorithm
    const Gaudi::Algorithm* parent();

    /// Access the parent AsynchronousAlgorithm
    const Gaudi::AsynchronousAlgorithm* asyncParent();

    /// Yield fiber until stream is done
    StatusCode await();

    /// INTERNAL: Register a dependency
    void registerDependency();

    /// INTERNAL: Remove a dependency
    void removeDependency();
  };
} // namespace Gaudi::CUDA

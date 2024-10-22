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
#include <Gaudi/CUDAAsynchronousAlgHelper.cuh>
#include <boost/fiber/cuda/waitfor.hpp>

#include <cstdio>
#include <cstring>
#include <tuple>

namespace Gaudi {
  namespace CUDA {
    const char* err_fmt( cudaError_t err ) {
      if ( err == cudaSuccess ) {
        return nullptr;
      } else {
        const char* errname = cudaGetErrorName( err );
        const char* errstr  = cudaGetErrorString( err );
        std::size_t msg_len = 28 + std::strlen( errname ) + std::strlen( errstr );
        char*       buf     = new char[msg_len];
        std::snprintf( buf, msg_len, "Encountered CUDA error %s: %s\n", errname, errstr );
        return buf;
      }
    }

    const char* create_stream( cudaStream_t* stream_ptr ) { return err_fmt( cudaStreamCreate( stream_ptr ) ); }

    const char* delete_stream( cudaStream_t stream ) { return err_fmt( cudaStreamDestroy( stream ) ); }

    const char* cuda_stream_await( cudaStream_t cudaStream ) {
      auto        res = boost::fibers::cuda::waitfor_all( cudaStream );
      cudaError_t err = std::get<1>( res );
      return err_fmt( err );
    }

  } // namespace CUDA
} // namespace Gaudi

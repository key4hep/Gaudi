#include "Gaudi/CUDAAcceleratedAlgHelper.cuh"
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

/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "CUDADeviceArray.h"

// Gaudi
#include <Gaudi/AsynchronousAlgorithm.h>
#include <Gaudi/CUDA/CUDAStream.h>

// CUDA
#ifndef __CUDACC__
#  include <cuda_runtime.h>
#endif

// Fibers
#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/mutex.hpp>

// standard library
#include <chrono>
#include <format>
#include <string>
#include <thread>

namespace Gaudi::CUDA::Detail {
  using namespace std::chrono_literals;
  namespace {
    const std::string DEVARREXC = "CUDADeviceArrayException";
    std::string       err_fmt( cudaError_t err, std::string file, int line ) {
      const char* errname = cudaGetErrorName( err );
      const char* errstr  = cudaGetErrorString( err );
      std::string errmsg =
          std::format( "Encountered CUDA error {} [{}]: {} on {}:{}", errname, int( err ), errstr, file, line );
      return errmsg;
    }

    boost::fibers::mutex              gpu_mem_mtx;
    boost::fibers::condition_variable gpu_mem_cv;
  } // namespace

  void* allocateWithStream( std::size_t size, Stream& stream ) {
    void*       devPtr     = nullptr;
    cudaError_t err        = cudaSuccess;
    auto        start_time = std::chrono::steady_clock::now();
    do {
      err = cudaMallocAsync( &devPtr, size, stream );
      if ( err == cudaSuccess ) { break; }
      if ( err == cudaErrorMemoryAllocation ) {
        cudaGetLastError();
        std::unique_lock lck( gpu_mem_mtx );
        gpu_mem_cv.wait( lck );
      } else {
        throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
      }
    } while ( err == cudaErrorMemoryAllocation );
    // In case we suspended
    stream.parent()->restoreAfterSuspend().orThrow( "Error restoring", DEVARREXC );
    return devPtr;
  }

  void* allocateNoStream( std::size_t size, Gaudi::AsynchronousAlgorithm* parent ) {
    void*       devPtr     = nullptr;
    cudaError_t err        = cudaSuccess;
    auto        start_time = std::chrono::steady_clock::now();
    do {
      err = cudaMalloc( &devPtr, size );
      if ( err == cudaSuccess ) { break; }
      if ( err == cudaErrorMemoryAllocation ) {
        cudaGetLastError();
        // If called from an AsynchronousAlgorithm, wait as in the with stream variant
        // Otherwise, the thread should sleep
        if ( parent != nullptr ) {
          std::unique_lock lck( gpu_mem_mtx );
          gpu_mem_cv.wait( lck );
          parent->restoreAfterSuspend().orThrow( "Error restoring", DEVARREXC );
        } else {
          std::this_thread::sleep_for( 100ms );
        }
      } else {
        throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
      }
    } while ( err == cudaErrorMemoryAllocation );
    return devPtr;
  }

  void freeWithStream( void* ptr, Stream& stream ) {
    cudaError_t err = cudaFreeAsync( ptr, stream );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
    gpu_mem_cv.notify_all();
  }

  void freeNoStream( void* ptr ) {
    cudaError_t err = cudaFree( ptr );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
    gpu_mem_cv.notify_all();
  }

  void copyHostToDeviceWithStream( void* devPtr, const void* hstPtr, std::size_t size, Stream& stream ) {
    cudaError_t err = cudaMemcpyAsync( devPtr, hstPtr, size, cudaMemcpyHostToDevice, stream );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
    // await stream to avoid deleting host memory before copy is done
    stream.await().orThrow( "Await error", DEVARREXC );
  }

  void copyHostToDeviceNoStream( void* devPtr, const void* hstPtr, std::size_t size ) {
    cudaError_t err = cudaMemcpy( devPtr, hstPtr, size, cudaMemcpyHostToDevice );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
  }

  void copyDeviceToHostWithStream( void* hstPtr, const void* devPtr, std::size_t size, Stream& stream ) {
    cudaError_t err = cudaMemcpyAsync( hstPtr, devPtr, size, cudaMemcpyDeviceToHost, stream );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
    // await stream to avoid deleting host memory before copy is done
    stream.await().orThrow( "Await error", DEVARREXC );
  }

  void copyDeviceToHostNoStream( void* hstPtr, const void* devPtr, std::size_t size ) {
    cudaError_t err = cudaMemcpy( hstPtr, devPtr, size, cudaMemcpyDeviceToHost );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
  }

  void copyDeviceToDeviceWithStream( void* destDevPtr, const void* srcDevPtr, std::size_t size, Stream& stream ) {
    cudaError_t err = cudaMemcpyAsync( destDevPtr, srcDevPtr, size, cudaMemcpyDeviceToDevice, stream );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
  }

  void copyDeviceToDeviceNoStream( void* destDevPtr, const void* srcDevPtr, std::size_t size ) {
    cudaError_t err = cudaMemcpy( destDevPtr, srcDevPtr, size, cudaMemcpyDeviceToDevice );
    if ( err != cudaSuccess ) {
      throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), DEVARREXC, StatusCode::FAILURE );
    }
  }
} // namespace Gaudi::CUDA::Detail

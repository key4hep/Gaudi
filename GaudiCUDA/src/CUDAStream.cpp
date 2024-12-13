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

// Gaudi
#include <Gaudi/AsynchronousAlgorithm.h>
#include <Gaudi/CUDA/CUDAStream.h>

// CUDA
#ifndef __CUDACC__
#  include <cuda_runtime.h>
#endif

// Others
#include <boost/fiber/cuda/waitfor.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/recursive_mutex.hpp>

// Standard Library
#include <cstdio>
#include <deque>
#include <format>
#include <mutex>
#include <string>

namespace Gaudi::CUDA {
  namespace {
    class StreamList {
      using Mutex_t  = std::recursive_mutex;
      using Stream_t = cudaStream_t;

    private:
      std::deque<Stream_t> queue;
      Mutex_t              queue_mtx;

    public:
      /// Push a stream onto the end of the queue
      void push( const Stream_t& s ) {
        std::unique_lock lck( queue_mtx );
        queue.push_back( s );
      }

      /// Pop a stream off the end of a queue. If use_cv, wait til one is available,
      /// else return false if queue is empty
      bool pop( Stream_t& s ) {
        std::unique_lock lck( queue_mtx );
        if ( queue.empty() ) { return false; }
        s = queue.front();
        queue.pop_front();
        return true;
      }

      ~StreamList() {
        Stream_t s;
        while ( pop( s ) ) {
          cudaStreamDestroy( s );
          s = nullptr;
        }
      }
    };
    StreamList  available_streams{};
    std::string err_fmt( cudaError_t err, std::string file, int line ) {
      const char* errname = cudaGetErrorName( err );
      const char* errstr  = cudaGetErrorString( err );
      std::string errmsg =
          std::format( "Encountered CUDA error {} [{}]: {} on {}:{}", errname, int( err ), errstr, file, line );
      return errmsg;
    }
  } // namespace
  Stream::Stream( const Gaudi::AsynchronousAlgorithm* parent )
      : m_stream( nullptr ), m_parent( parent ), m_dependents( 0 ) {
    if ( !available_streams.pop( m_stream ) ) {
      cudaError_t err = cudaStreamCreate( &m_stream );
      if ( err != cudaSuccess ) {
        cudaGetLastError();
        throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), "CUDAStreamException", StatusCode::FAILURE );
      }
      err = cudaStreamSynchronize( m_stream );
      if ( err != cudaSuccess ) {
        cudaGetLastError();
        throw GaudiException( err_fmt( err, __FILE__, __LINE__ ), "CUDAStreamException", StatusCode::FAILURE );
      }
    }
  }

  Stream::~Stream() {
    if ( m_dependents != 0 ) {
      m_parent->error() << std::format( "Stream destroyed before all its dependents ({} remaining)", m_dependents )
                        << endmsg;
    }
    if ( await().isFailure() ) { m_parent->error() << "Error in Stream destructor" << endmsg; }
    available_streams.push( m_stream );
  }

  StatusCode Stream::await() {
    auto        res        = boost::fibers::cuda::waitfor_all( m_stream );
    cudaError_t temp_error = std::get<1>( res );
    if ( ( temp_error ) != cudaSuccess ) {
      cudaGetLastError();
      std::string errmsg = err_fmt( temp_error, __FILE__, __LINE__ );
      m_parent->error() << errmsg << endmsg;
      return StatusCode::FAILURE;
    }
    return m_parent->restoreAfterSuspend();
  }
} // namespace Gaudi::CUDA

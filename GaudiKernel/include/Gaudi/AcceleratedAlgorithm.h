
/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
// ============================================================================
// Include files
// ============================================================================
// Gaudi
#include "Gaudi/Algorithm.h"
#include "Gaudi/CUDAAcceleratedAlgHelper.cuh"
#include "GaudiKernel/IHiveWhiteBoard.h"
// Others
#include "fmt/format.h"
#include <atomic>
#include <boost/fiber/all.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <chrono>

namespace Gaudi {
  /** Base class for accelerated algorithms.
   *
   *  Augments Gaudi::Algorithm by saving and restoring current slot whenever
   *  fiber is suspended and resumed. This requires using the member functions for
   *  suspending instead of the boost::fiber functions directly.
   *
   *  @author Beojan Stanislaus
   *  @date 2023
   */

/// Macro for case where we don't yet have a subclass / member function for a given
/// type of waiting
///
#define ACCALG_AWAIT( stmt )                                                                                           \
  stmt;                                                                                                                \
  if ( !whiteboard()->selectStore( *s_currentSlot ).isSuccess() ) {                                                    \
    msg() << MSG::ERROR << "Resetting slot from fiber_specific_ptr failed" << endmsg;                                  \
    return StatusCode::FAILURE;                                                                                        \
  }

  class GAUDI_API AcceleratedAlgorithm : virtual public Gaudi::Algorithm {
  protected:
    /// Contains current slot
    boost::fibers::fiber_specific_ptr<std::size_t> s_currentSlot{};

  public:
    StatusCode sysInitialize() override {
      setAccelerated( true );
      msg() << MSG::INFO << "Starting sysInitialize for AcceleratedAlgorithm" << endmsg;
      return Gaudi::Algorithm::sysInitialize();
    }

    StatusCode sysExecute( const EventContext& ctx ) override {
      msg() << MSG::INFO << "Starting sysExecute for AcceleratedAlgorithm on slot " << ctx.slot()
            << "with s_currentSlot = " << fmt::to_string( fmt::ptr( s_currentSlot.get() ) ) << endmsg;
      if ( s_currentSlot.get() == nullptr ) {
        s_currentSlot.reset( new std::size_t( ctx.slot() ) );
      } else if ( *s_currentSlot != ctx.slot() ) {
        error() << "Current slot is " << ctx.slot() << " but s_currentSlot exists and is " << *s_currentSlot << endmsg;
        return StatusCode::FAILURE;
      }
      return Gaudi::Algorithm::sysExecute( ctx );
    }

    /// Forwards to boost::this_fiber::yield
    StatusCode yield() const {
      boost::this_fiber::yield();
      return whiteboard()->selectStore( *s_currentSlot );
    }

    /// Forwards to boost::this_fiber::sleep_until
    template <typename Clock, typename Duration>
    StatusCode sleep_until( std::chrono::time_point<Clock, Duration> const& sleep_time ) const {
      boost::this_fiber::sleep_until( sleep_time );
      return whiteboard()->selectStore( *s_currentSlot );
    }

    /// Forwards to boost::this_fiber::sleep_for
    template <typename Rep, typename Period>
    StatusCode sleep_for( std::chrono::duration<Rep, Period> const& dur ) const {
      boost::this_fiber::sleep_for( dur );
      return whiteboard()->selectStore( *s_currentSlot );
    }

    /// Wrapper for CUDA stream await
    StatusCode cuda_stream_await( cudaStream_t cudaStream ) const {
      CUDA_CHECK( Gaudi::CUDA::cuda_stream_await( cudaStream ) );
      return whiteboard()->selectStore( *s_currentSlot );
      return StatusCode::SUCCESS;
    }

    /// Helper to allow other classes to print error messages
    void print_cuda_error( std::string msg_ ) const {
      msg() << MSG::ERROR << msg_ << endmsg;
      throw GaudiException( msg_, "CUDA_EXCEPTION", StatusCode::FAILURE );
    }
  };

  namespace CUDA {
    using namespace std::chrono_literals;

    class CUDAStream {
    private:
      cudaStream_t                       stream;
      const Gaudi::AcceleratedAlgorithm* parent;
      int                                nth_stream = 0;
      boost::unordered_flat_set<void*>   allocations{};

    public:
      CUDAStream( const Gaudi::AcceleratedAlgorithm* parent, std::string file = __FILE__, int line = __LINE__ );

      operator cudaStream_t() { return stream; }

      template <typename T>
      T* malloc( std::size_t len ) {
        void*       devPtr = nullptr;
        cudaError_t err    = cudaSuccess;
        if constexpr ( !std::is_same_v<T, void> ) { len *= sizeof( T ); }
        const auto starttime = std::chrono::steady_clock::now();
        do {
          err = cudaMallocAsync( &devPtr, len, stream );
          if ( err == cudaErrorMemoryAllocation ) {
            StatusCode sc = parent->sleep_for( 10ms );
            if ( sc.isFailure() ) { parent->print_cuda_error( "Yield error" ); }
          }
        } while ( err == cudaErrorMemoryAllocation );
        const double waittime =
            std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - starttime )
                .count() /
            1e6;
        if ( waittime >= 0.01 ) {
          fmt::print( "Waited {} to allocate {} of GPU memory\n", SI( waittime, "s" ), SI( len, "B" ) );
        }
        allocations.insert( devPtr );
        return static_cast<T*>( devPtr );
      }

      template <typename T>
      void free( T* d_ptr ) {
        auto iter = allocations.find( d_ptr );
        if ( iter == allocations.end() ) {
          parent->print_cuda_error( "Called stream.free on an allocation not created by this stream" );
        }
        cudaFreeAsync( static_cast<void*>( d_ptr ), stream );
        allocations.erase( iter );
      }

      ~CUDAStream();
    };
  } // namespace CUDA
} // namespace Gaudi

#include <Gaudi/AsynchronousAlgorithm.h>

#include <boost/fiber/cuda/waitfor.hpp>

#include <atomic>
#include <cstdio>
#include <memory_resource>
#include <string>
#include <tuple>

#include <boost/lockfree/queue.hpp>
#include <vecmem/memory/binary_page_memory_resource.hpp>

#include <fmt/format.h>

namespace Gaudi {
  namespace CUDA {
    std::atomic_int running_streams = 0;

    class StreamList {
    private:
      boost::lockfree::queue<cudaStream_t> queue{ 5 };

    public:
      boost::lockfree::queue<cudaStream_t>* operator->() { return &queue; }
      ~StreamList() {
        while ( !queue.empty() ) {
          cudaStream_t stream = nullptr;
          if ( queue.pop( stream ) ) cudaStreamDestroy( stream );
        }
      }
    };
    StreamList  available_streams{};
    std::string err_fmt( cudaError_t err, std::string file, int line ) {
      const char* errname = cudaGetErrorName( err );
      const char* errstr  = cudaGetErrorString( err );
      std::string errmsg =
          fmt::format( "Encountered CUDA error {} [{}]: {} on {}:{}", errname, int( err ), errstr, file, line );
      return errmsg;
    }

    cudaError_t cuda_stream_await( cudaStream_t cudaStream ) {
      auto        res = boost::fibers::cuda::waitfor_all( cudaStream );
      cudaError_t err = std::get<1>( res );
      return err;
    }

    namespace Detail {
      void* PinnedMemoryResource::do_allocate( std::size_t bytes, std::size_t /* alignment */ ) {
        cudaError_t firstErr = cudaGetLastError();
        if ( firstErr != cudaSuccess ) {
          // throw GaudiException(
          //     fmt::format( "First error {} ({}): {}", cudaGetErrorName( firstErr ), int( firstErr ), cudaGetErrorString( firstErr ) ),
          //     "CUDA_ERROR", StatusCode::FAILURE );
        }
        num_allocs.fetch_add( 1 );
        void*       ptr = nullptr;
        cudaError_t err = cudaMallocHost( &ptr, bytes );
        if ( err == cudaSuccess ) { return ptr; }
        if ( err == cudaErrorInvalidValue || err == cudaErrorMemoryAllocation ) {
          throw std::bad_alloc();
        } else {
          throw GaudiException( fmt::format( "in cudaMallocHost({}, /**/) {} ({}): {}", bytes, cudaGetErrorName( err ),
                                             int( err ), cudaGetErrorString( err ) ),
                                "CUDA_ERROR", StatusCode::FAILURE );
        }
      }

      void PinnedMemoryResource::do_deallocate( void* p, std::size_t bytes, std::size_t /* alignment */ ) {
        num_deallocs.fetch_add( 1 );
        cudaError_t err = cudaFreeHost( p );
        if ( err != cudaSuccess && err != cudaErrorCudartUnloading ) {
          throw GaudiException( fmt::format( "CUDA ERROR {}: {}", cudaGetErrorName( err ), cudaGetErrorString( err ) ),
                                "CUDA_ERROR", StatusCode::FAILURE );
        }
      }

      bool PinnedMemoryResource::do_is_equal( const std::pmr::memory_resource& other ) const noexcept {
        return bool( dynamic_cast<const PinnedMemoryResource*>( &other ) );
      }
    } // namespace Detail

    std::pmr::memory_resource* get_pinned_memory_resource() {
      fmt::print( "Initializing pinned memory resource\n" );
      static auto base = std::make_unique<Detail::PinnedMemoryResource>();
      static auto bpmr = std::make_unique<vecmem::binary_page_memory_resource>( *base );
      static auto res  = std::make_unique<std::pmr::synchronized_pool_resource>( bpmr.get() );
      return dynamic_cast<std::pmr::memory_resource*>( res.get() );
    }

    CUDAStream::CUDAStream( const Gaudi::AsynchronousAlgorithm* parent, std::string file, int line )
        : stream( nullptr ), parent( parent ) {
      nth_stream = running_streams.fetch_add( 1 ) + 1;
      fmt::print( "Starting {}th concurrent stream\n", nth_stream );
      if ( !available_streams->pop( stream ) ) {
        cudaError_t err = cudaStreamCreate( &stream );
        if ( err != cudaSuccess ) { parent->print_cuda_error( err_fmt( err, __FILE__, __LINE__ ) ); }
      }
    }

    CUDAStream::~CUDAStream() {
      for ( void* allocation : allocations ) { cudaFreeAsync( allocation, stream ); }
      if ( parent->cuda_stream_await( stream ).isFailure() ) {
        parent->print_cuda_error( "Error freeing CUDA stream memory" );
      }
      running_streams.fetch_sub( 1 );
      available_streams->push( stream );
    }
  } // namespace CUDA
} // namespace Gaudi

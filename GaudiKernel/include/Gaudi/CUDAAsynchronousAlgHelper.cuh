/***********************************************************************************\
* (c) Copyright 2023-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <format>

#include <atomic>
#include <memory_resource>
#include <string>

namespace Gaudi {
  namespace CUDA {
    inline std::string SI( double number, std::string_view unit ) {
      static const std::string prefix[] = { "q", "r", "y", "z", "a", "f", "p", "n", "µ", "m", "",
                                            "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q" };
      int                      idx      = std::floor( ( std::log10( number ) + 30 ) / 3 );
      if ( idx < 0 ) { idx = 0; }
      if ( idx > 20 ) { idx = 20; }
      // splitting 30 into 36 - 6 gives us 6 digits after the decimal point
      double reduced = std::round( number / std::pow( 10, 3 * idx - 36 ) ) / 1e6;
      if ( unit.size() <= 1 ) { return std::format( "{} {}{}", reduced, prefix[idx], unit ); }
      return std::format( "{} {}·{}", reduced, prefix[idx], unit );
    }
    std::string                err_fmt( cudaError_t err, std::string file, int line );
    cudaError_t                cuda_stream_await( cudaStream_t stream );
    std::pmr::memory_resource* get_pinned_memory_resource();
    namespace Detail {
      class PinnedMemoryResource : public std::pmr::memory_resource {
        void*              do_allocate( std::size_t bytes, std::size_t alignment ) override;
        void               do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override;
        bool               do_is_equal( const std::pmr::memory_resource& other ) const noexcept override;
        std::atomic_size_t num_allocs   = 0;
        std::atomic_size_t num_deallocs = 0;

      public:
        ~PinnedMemoryResource() {
#ifndef NDEBUG
          std::cout << std::format( "Allocated {} times and deallocated {} times\n", num_allocs.load(),
                                    num_deallocs.load() )
                    << std::endl;
#endif
        }
      };
    } // namespace Detail

  } // namespace CUDA
} // namespace Gaudi

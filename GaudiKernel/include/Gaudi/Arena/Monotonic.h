/***********************************************************************************\
* (c) Copyright 2019-20 CERN for the benefit of the LHCb and ATLAS collaborations   *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include "Gaudi/Allocator/Arena.h"
#include "GaudiKernel/Kernel.h"

#include <boost/container/small_vector.hpp>

#include <gsl/span>

#include <cstddef>
#include <numeric>
namespace Gaudi::Arena {
  namespace details {
    template <std::size_t Alignment>
    constexpr std::size_t align_up( std::size_t n ) {
      return ( n + ( Alignment - 1 ) ) & ~( Alignment - 1 );
    }
  } // namespace details

  /** @class Monotonic
   *  @brief A fast memory arena that does not track deallocations.
   *
   *  This is a memory arena suitable for use with Gaudi::Allocators::Arena.
   *  It allocates memory from an upstream resource in blocks of geometrically
   *  increasing size and serves allocation requests from those blocks.
   *  Deallocations are not tracked, so the memory footprint of a Monotonic
   *  arena increases monotonically until either it is destroyed or its reset()
   *  method is called.
   *  All requests are served with alignment specified in the template parameter.
   *
   *  @todo Efficiently support stateful upstream allocators, probably by putting an
   *        instance of the upstream allocator in a boost::compressed_pair.
   *  @todo Use the given UpstreamAllocator to serve dynamic allocations required by
   *        boost::container::small_vector.
   */
  template <std::size_t Alignment = alignof( std::max_align_t ), typename UpstreamAllocator = std::allocator<std::byte>>
  class Monotonic {
    // Restriction could be lifted, see @todo above.
    static_assert( std::is_empty_v<UpstreamAllocator>, "Stateful upstream allocators are not yet supported." );

    /// Size (in bytes) of the next block to be allocated.
    std::size_t m_next_block_size{};

    /// Number of allocation requests served by this arena.
    std::size_t m_allocations{0};

    /// Current position in the current block, or nullptr if there is no current block.
    std::byte* m_current{nullptr};

    /// One byte past the end of the current block, or nullptr if it doesn't exist.
    std::byte* m_current_end{nullptr};

    /// All memory blocks owned by this arena.
    boost::container::small_vector<gsl::span<std::byte>, 1> m_all_blocks;

    /// Approximate factor by which each block is larger than its predecessor.
    static constexpr std::size_t growth_factor = 2;

  public:
    static constexpr std::size_t alignment = Alignment;

    /** Construct an arena whose first block have approximately the given size.
     *  This constructor does not trigger any allocation.
     */
    Monotonic( std::size_t next_block_size ) noexcept
        : m_next_block_size{details::align_up<Alignment>( next_block_size )} {}

    ~Monotonic() noexcept {
      for ( auto block : m_all_blocks ) { UpstreamAllocator{}.deallocate( block.data(), block.size() ); }
    }

    // Allocators will hold pointers to instances of this class, deleting these
    // methods makes it harder to accidentally invalidate those pointers...
    Monotonic( Monotonic&& )      = delete;
    Monotonic( Monotonic const& ) = delete;
    Monotonic& operator=( Monotonic&& ) = delete;
    Monotonic& operator=( Monotonic const& ) = delete;

    /** Return an aligned point to n bytes of memory.
     *  This may trigger allocation from the upstream resource.
     */
    template <std::size_t ReqAlign>
    std::byte* allocate( std::size_t n ) {
      // If the requested alignment was larger we would need to round up
      // m_current -- instead of implementing that, just assert it's not
      // the case.
      static_assert( ReqAlign <= alignment, "Requested alignment too large for this Gaudi::Arena::Monotonic!" );
      // Figure out how many bytes we need to allocate
      std::size_t const aligned_n = details::align_up<Alignment>( n );
      // Check that we have a current block and this request fits inside it
      if ( UNLIKELY( !m_current || m_current + aligned_n > m_current_end ) ) {
        // Calculate our next block size
        auto next_block_size = std::max( m_next_block_size, aligned_n );
        // And update the estimate of what comes after that, following a geometric series
        m_next_block_size = details::align_up<Alignment>( growth_factor * next_block_size );
        // Allocate the new block and mark it as the current one
        m_current     = UpstreamAllocator{}.allocate( next_block_size );
        m_current_end = m_current + next_block_size;
        // Add it to the list of blocks that we'll eventually deallocate
        m_all_blocks.emplace_back( m_current, next_block_size );
      }
      m_allocations++;
      return std::exchange( m_current, m_current + aligned_n );
    }

    /** Deallocations are not tracked, so this is a no-op!
     */
    constexpr void deallocate( std::byte*, std::size_t ) noexcept {}

    /** Signal that this arena may start re-using the memory resources.
     *  - If the arena owns zero blocks, there is no change.
     *  - If the arena owns one block, it will reset to serving future requests from the
     *    start of that block.
     *  - If the arena owns more than one block, it will deallocate all but the first one
     *    and serve future requests from the start of the remaining block.
     */
    void reset() noexcept {
      m_allocations = 0;
      if ( !m_all_blocks.empty() ) {
        // Only re-use the first block, deallocate any others
        if ( UNLIKELY( m_all_blocks.size() > 1 ) ) {
          for ( std::size_t i = 1; i < m_all_blocks.size(); ++i ) {
            UpstreamAllocator{}.deallocate( m_all_blocks[i].data(), m_all_blocks[i].size() );
          }
          m_all_blocks.resize( 1 );
        }
        auto reused_block = m_all_blocks.front();
        m_current         = reused_block.data();
        m_current_end     = m_current + reused_block.size();
        m_next_block_size = details::align_up<Alignment>( growth_factor * reused_block.size() );
      }
    }

    /** Query how much memory is owned by this arena, in bytes.
     */
    [[nodiscard]] std::size_t capacity() const noexcept {
      return std::accumulate( m_all_blocks.begin(), m_all_blocks.end(), 0ul,
                              []( std::size_t sum, auto block ) { return sum + block.size(); } );
    }

    /** Query how much memory was *used* from this arena, in bytes.
     */
    [[nodiscard]] std::size_t size() const noexcept { return capacity() - ( m_current_end - m_current ); }

    /** Query how many blocks of memory this arena owns.
     */
    [[nodiscard]] std::size_t num_blocks() const noexcept { return m_all_blocks.size(); }

    /** Query how many allocations this arena has served.
     */
    [[nodiscard]] std::size_t num_allocations() const noexcept { return m_allocations; }
  };
} // namespace Gaudi::Arena

namespace Gaudi::Allocator {
  /** @class MonotonicArena
   *  @brief Shorthand for Gaudi::Allocator::Arena with Gaudi::Arena::Monotonic resource
   */
  template <typename T, typename DefaultResource = void, std::size_t Alignment = alignof( std::max_align_t ),
            typename UpstreamAllocator = std::allocator<std::byte>>
  using MonotonicArena =
      ::Gaudi::Allocator::Arena<::Gaudi::Arena::Monotonic<Alignment, UpstreamAllocator>, T, DefaultResource>;
} // namespace Gaudi::Allocator
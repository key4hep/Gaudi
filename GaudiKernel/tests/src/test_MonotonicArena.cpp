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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_MonotonicArena
#include "Gaudi/Arena/Monotonic.h"
#include <boost/align/align_up.hpp>
#include <boost/test/unit_test.hpp>

std::size_t memory  = 0;
std::size_t alloc   = 0;
std::size_t dealloc = 0;

// Override global operators for testing purposes
void* operator new( std::size_t s ) {
  memory += s;
  ++alloc;
  return std::malloc( s );
}

void operator delete( void* p ) noexcept {
  ++dealloc;
  std::free( p );
}

void operator delete( void* p, std::size_t ) noexcept {
  ++dealloc;
  std::free( p );
}

BOOST_AUTO_TEST_CASE( test_arena ) {
  memory = alloc = dealloc               = 0; // reset counters just in case
  std::size_t           first_block_size = 100;
  constexpr std::size_t alignment        = 2;
  // default upstream allocator
  Gaudi::Arena::Monotonic<alignment> arena{ first_block_size };
  // no requests served yet, everything should be zero
  BOOST_CHECK( alloc == 0 );
  BOOST_CHECK( memory == 0 );
  BOOST_CHECK( dealloc == 0 );
  BOOST_CHECK( arena.size() == 0 );
  BOOST_CHECK( arena.capacity() == 0 );
  BOOST_CHECK( arena.num_blocks() == 0 );
  BOOST_CHECK( arena.num_allocations() == 0 );
  // allocate 11 bytes
  std::size_t nbytes = 11;
  BOOST_CHECK( nbytes <= first_block_size );
  auto ptr = arena.allocate<alignment>( nbytes );
  BOOST_CHECK( ptr != nullptr );
  // check the basics first
  BOOST_CHECK( alloc == 1 );
  BOOST_CHECK( dealloc == 0 );
  BOOST_CHECK( arena.size() >= nbytes );
  BOOST_CHECK( arena.capacity() == memory );
  BOOST_CHECK( arena.capacity() >= first_block_size );
  BOOST_CHECK( arena.num_blocks() == 1 );
  BOOST_CHECK( arena.num_allocations() == 1 );
  // arguably these are implementation details...
  BOOST_CHECK( arena.size() == boost::alignment::align_up( nbytes, alignment ) );
  BOOST_CHECK( arena.capacity() == boost::alignment::align_up( first_block_size, alignment ) );
  // allocate enough more that the arena will have to reallocate
  BOOST_CHECK( arena.allocate<alignment>( first_block_size ) != nullptr );
  BOOST_CHECK( alloc >= 2 ); // maybe more because of small vector
  BOOST_CHECK( dealloc == 0 );
  BOOST_CHECK( arena.size() >= nbytes + first_block_size );
  BOOST_CHECK( arena.capacity() >= 2 * first_block_size );
  BOOST_CHECK( memory >= arena.capacity() );
  BOOST_CHECK( arena.num_blocks() == 2 );
  BOOST_CHECK( arena.num_allocations() == 2 );
  // reset the arena
  auto memory_pre_reset = memory;
  auto allocs_pre_reset = alloc;
  arena.reset();
  BOOST_CHECK( dealloc >= 1 ); // should have freed the second block (small vector?)
  BOOST_CHECK( alloc == allocs_pre_reset );
  BOOST_CHECK( memory == memory_pre_reset );
  BOOST_CHECK( arena.size() == 0 );
  BOOST_CHECK( arena.capacity() >= first_block_size );
  BOOST_CHECK( arena.num_blocks() == 1 );
  BOOST_CHECK( arena.num_allocations() == 0 );
  auto ptr2 = arena.allocate<alignment>( nbytes );
  BOOST_CHECK( ptr == ptr2 );
}

BOOST_AUTO_TEST_CASE( test_allocator ) {
  memory = alloc = dealloc                                           = 0; // reset counters just in case
  auto                                                    N          = 100;
  auto                                                    block_size = N * sizeof( int );
  Gaudi::Arena::Monotonic<>                               arena{ block_size };
  std::vector<int, Gaudi::Allocator::MonotonicArena<int>> vec{ &arena };
  vec.resize( N );
  BOOST_CHECK( alloc == 1 );
  BOOST_CHECK( arena.size() == block_size );
  BOOST_CHECK( arena.capacity() >= block_size );
  BOOST_CHECK( arena.num_blocks() == 1 );
  BOOST_CHECK( arena.num_allocations() == 1 );
}

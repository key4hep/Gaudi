/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/cxx/SynchronizedValue.h>

#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <type_traits>

#if __has_include( <catch2/catch.hpp> )
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

namespace {

  using Gaudi::cxx::SynchronizedValue;

  struct CopyOnly {
    int value = 0;

    CopyOnly() = default;
    explicit CopyOnly( int v ) : value{ v } {}
    CopyOnly( CopyOnly const& )            = default;
    CopyOnly& operator=( CopyOnly const& ) = default;
    CopyOnly( CopyOnly&& )                 = delete;
    CopyOnly& operator=( CopyOnly&& )      = delete;
  };

  struct InitializerListHostile {
    int value = 0;

    InitializerListHostile() = default;
    explicit InitializerListHostile( int v ) : value{ v } {}
    InitializerListHostile( InitializerListHostile const& )                 = default;
    InitializerListHostile& operator=( InitializerListHostile const& )      = default;
    InitializerListHostile( InitializerListHostile&& )                      = default;
    InitializerListHostile& operator=( InitializerListHostile&& )           = default;
    InitializerListHostile( std::initializer_list<InitializerListHostile> ) = delete;
  };

  enum class ConstructionPath { Default, Copied, Greedy };

  struct GreedyFromAnything {
    ConstructionPath path = ConstructionPath::Default;

    GreedyFromAnything() = default;
    GreedyFromAnything( GreedyFromAnything const& ) : path{ ConstructionPath::Copied } {}
    GreedyFromAnything& operator=( GreedyFromAnything const& ) = default;

    template <typename T>
    explicit GreedyFromAnything( T&& ) : path{ ConstructionPath::Greedy } {}
  };

  struct CountingMutex {
    static inline int locks        = 0;
    static inline int shared_locks = 0;

    static void reset() {
      locks        = 0;
      shared_locks = 0;
    }

    void lock() {
      ++locks;
      m.lock();
    }
    bool try_lock() {
      ++locks;
      return m.try_lock();
    }
    void unlock() { m.unlock(); }

    void lock_shared() {
      ++shared_locks;
      m.lock_shared();
    }
    void unlock_shared() { m.unlock_shared(); }

  private:
    std::shared_mutex m;
  };

  struct CountingReadLock {
    explicit CountingReadLock( CountingMutex& mutex ) : m_mutex{ &mutex } { m_mutex->lock_shared(); }
    CountingReadLock( CountingReadLock const& )            = delete;
    CountingReadLock& operator=( CountingReadLock const& ) = delete;
    ~CountingReadLock() { m_mutex->unlock_shared(); }

  private:
    CountingMutex* m_mutex = nullptr;
  };

  struct CountingWriteLock {
    explicit CountingWriteLock( CountingMutex& mutex ) : m_mutex{ &mutex } { m_mutex->lock(); }
    CountingWriteLock( CountingWriteLock const& )            = delete;
    CountingWriteLock& operator=( CountingWriteLock const& ) = delete;
    ~CountingWriteLock() { m_mutex->unlock(); }

  private:
    CountingMutex* m_mutex = nullptr;
  };

  template <typename Value>
  using CountingSynchronizedValue = SynchronizedValue<Value, CountingMutex, CountingReadLock, CountingWriteLock>;

  template <typename T>
  concept ValidSynchronizedValue = requires { sizeof( SynchronizedValue<T> ); };

  static_assert( !ValidSynchronizedValue<int&> );
  static_assert( std::is_constructible_v<SynchronizedValue<int>, int> );
  static_assert( !std::is_convertible_v<int, SynchronizedValue<int>> );
  static_assert( std::is_constructible_v<SynchronizedValue<CopyOnly>, SynchronizedValue<CopyOnly> const&> );
  static_assert( std::is_constructible_v<SynchronizedValue<InitializerListHostile>,
                                         SynchronizedValue<InitializerListHostile> const&> );
  static_assert(
      std::is_constructible_v<SynchronizedValue<GreedyFromAnything>, SynchronizedValue<GreedyFromAnything>&> );

} // namespace

TEST_CASE( "SynchronizedValue basic construction and assignment", "[SynchronizedValue]" ) {
  SynchronizedValue<int> value{ 1 };

  CHECK( value.with_lock( []( int const& v ) { return v; } ) == 1 );

  value.with_lock( []( int& v ) { v = 2; } );
  CHECK( value.with_lock( []( int const& v ) { return v; } ) == 2 );

  SynchronizedValue<int> copy{ value };
  CHECK( copy.with_lock( []( int const& v ) { return v; } ) == 2 );

  SynchronizedValue<int> assigned{ 0 };
  assigned = value;
  CHECK( assigned.with_lock( []( int const& v ) { return v; } ) == 2 );

  SynchronizedValue<int> moved{ SynchronizedValue<int>{ 3 } };
  CHECK( moved.with_lock( []( int const& v ) { return v; } ) == 3 );

  assigned = SynchronizedValue<int>{ 4 };
  CHECK( assigned.with_lock( []( int const& v ) { return v; } ) == 4 );
}

TEST_CASE( "SynchronizedValue supports non-default and copy-only values", "[SynchronizedValue]" ) {
  SynchronizedValue<CopyOnly> value{ 5 };
  SynchronizedValue<CopyOnly> copy{ value };

  CHECK( copy.with_lock( []( CopyOnly const& v ) { return v.value; } ) == 5 );

  SynchronizedValue<CopyOnly> assigned{ 0 };
  assigned = value;
  CHECK( assigned.with_lock( []( CopyOnly const& v ) { return v.value; } ) == 5 );
}

TEST_CASE( "SynchronizedValue copy construction uses direct initialization", "[SynchronizedValue]" ) {
  SynchronizedValue<InitializerListHostile> value{ 7 };
  SynchronizedValue<InitializerListHostile> copy{ value };

  CHECK( copy.with_lock( []( InitializerListHostile const& v ) { return v.value; } ) == 7 );
}

TEST_CASE( "SynchronizedValue copy construction is not hijacked by the forwarding constructor",
           "[SynchronizedValue]" ) {
  SynchronizedValue<GreedyFromAnything> value;
  SynchronizedValue<GreedyFromAnything> copy{ value };

  CHECK( copy.with_lock( []( GreedyFromAnything const& v ) { return v.path; } ) == ConstructionPath::Copied );
}

TEST_CASE( "SynchronizedValue selects read and write lock paths", "[SynchronizedValue]" ) {
  CountingMutex::reset();

  CountingSynchronizedValue<int> value{ 1 };

  CHECK( value.with_lock( []( int const& v ) { return v; } ) == 1 );
  CHECK( CountingMutex::shared_locks == 1 );
  CHECK( CountingMutex::locks == 0 );

  value.with_lock( []( int& v ) { ++v; } );
  CHECK( CountingMutex::shared_locks == 1 );
  CHECK( CountingMutex::locks == 1 );

  CountingSynchronizedValue<int> copy{ value };
  CHECK( copy.with_lock( []( int const& v ) { return v; } ) == 2 );
  CHECK( CountingMutex::shared_locks == 3 );

  CountingSynchronizedValue<int> assigned{ 0 };
  assigned = value;
  CHECK( assigned.with_lock( []( int const& v ) { return v; } ) == 2 );
  CHECK( CountingMutex::locks >= 3 );
}

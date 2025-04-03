/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <algorithm>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Gaudi::cxx {

  // C++23: replace with http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0290r2.html
  //         http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4033.html

  template <typename Value, typename Mutex = std::mutex,
            typename ReadLock  = std::conditional_t<std::is_same_v<std::shared_mutex, Mutex>, std::shared_lock<Mutex>,
                                                   std::lock_guard<Mutex>>,
            typename WriteLock = std::lock_guard<Mutex>>
  class SynchronizedValue {
    static_assert( !std::is_reference_v<Value>, "Value must not be a reference" );
    Value         m_obj;
    mutable Mutex m_mtx;

  public:
    template <typename... Args>
      requires( std::is_constructible_v<Value, Args...> )
    SynchronizedValue( Args&&... args ) : m_obj{ std::forward<Args>( args )... } {}

    SynchronizedValue( const SynchronizedValue& rhs ) {
      static_assert( std::is_default_constructible_v<Value> ); // bound to hold, as otherwise we wouldn't get this
                                                               // far... so for 'documnentation purpose' really (C++20:
                                                               // turn into a `requires` clause )...
      static_assert( std::is_copy_assignable_v<Value> );
      auto lock = std::scoped_lock{ rhs.m_mtx, m_mtx };
      m_obj     = rhs.m_obj;
    }

    SynchronizedValue& operator=( const SynchronizedValue& rhs ) {
      static_assert( std::is_copy_assignable_v<Value> );
      if ( this != &rhs ) {
        auto lock = std::scoped_lock{ rhs.m_mtx, m_mtx };
        m_obj     = rhs.m_obj;
      }
      return *this;
    }

    SynchronizedValue( SynchronizedValue&& rhs ) {
      static_assert( std::is_default_constructible_v<Value> ); // bound to hold, as otherwise we wouldn't get this
                                                               // far... so for 'documnentation purpose' really (C++20:
                                                               // turn into a `requires` clause )...
      static_assert( std::is_move_assignable_v<Value> );
      auto lock = std::scoped_lock{ rhs.m_mtx, m_mtx };
      m_obj     = std::move( rhs.m_obj );
    }

    SynchronizedValue& operator=( SynchronizedValue&& rhs ) {
      static_assert( std::is_move_assignable_v<Value> );
      if ( this != &rhs ) {
        auto lock = std::scoped_lock{ rhs.m_mtx, m_mtx };
        m_obj     = std::move( rhs.m_obj );
      }
      return *this;
    }

    template <typename... Args, std::invocable<Value&, Args...> F>
      requires( !std::is_invocable_v<F, const Value&, Args...> )
    decltype( auto ) with_lock( F&& f, Args&&... args ) {
      WriteLock _{ m_mtx };
      return std::invoke( std::forward<F>( f ), m_obj, std::forward<Args>( args )... );
    }

    template <typename... Args, std::invocable<const Value&, Args...> F>
    decltype( auto ) with_lock( F&& f, Args&&... args ) const {
      ReadLock _{ m_mtx };
      return std::invoke( std::forward<F>( f ), m_obj, std::forward<Args>( args )... );
    }
  };

  // transform an f(T,...) into an f(SynchronizedValue<T>,...)
  template <typename Fun>
  auto with_lock( Fun&& f ) {
    return [f = std::forward<Fun>( f )]( auto& p, auto&&... args ) -> decltype( auto ) {
      return p.with_lock( f, std::forward<decltype( args )>( args )... );
    };
  }
  // call f(T) for each element in a container of Synced<T>
  template <typename ContainerOfSynced, typename Fun>
  void for_each( ContainerOfSynced& c, Fun&& f ) {
    std::for_each( begin( c ), end( c ), with_lock( std::forward<Fun>( f ) ) );
  }

} // namespace Gaudi::cxx

/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_LOCKEDHANDLE
#define GAUDIKERNEL_LOCKEDHANDLE 1

#include <mutex>

// =======================================================================
/** @class LockedHandle
 *
 *  Provides automatic lock/unlock access to a class upon deref of ptr.
 *  default type of mutex to use is a std::mutex, but this can be
 *  overridden with a second template parameter, as long as the type
 *  provides the lock() and unlock() functions
 *
 *  @code
 *
 *  std::mutex the_mutex;
 *  Obj *o = new Obj();
 *  LockedHandle<Obj> lh(o, &the_mutex);
 *  o->doSomething();
 *
 *  @endcode
 *
 *  @author Charles Leggett / Goetz Gaycken
 *  @date   2016-07-01
 */

template <class T, class MutexType = std::mutex>
class LockedHandle {
public:
  LockedHandle( T* ptr, MutexType& mut ) : m_ptr( ptr ), m_mutex( &mut ) {}
  LockedHandle( T* ptr, MutexType* mut ) : m_ptr( ptr ), m_mutex( mut ) {}
  LockedHandle( T* ptr ) : m_ptr( ptr ), m_mutex( 0 ) {}
  LockedHandle() : m_ptr( nullptr ), m_mutex( nullptr ) {}

  void set( T* ptr, MutexType* mut ) {
    m_ptr   = ptr;
    m_mutex = mut;
  }

  void setMutex( MutexType* mut ) { m_mutex = mut; }

  T* get() const { return m_ptr; }

  class Guard {
    Guard( const Guard& a ) = delete;

  public:
    Guard( Guard&& a ) : m_ptr( a.m_ptr ), m_mutex( a.m_mutex ) { a.m_mutex = nullptr; }

    Guard( T* ptr, MutexType& mutex ) : m_ptr( ptr ), m_mutex( &mutex ) { m_mutex->lock(); }
    ~Guard() { m_mutex->unlock(); }
    T& operator*() { return *m_ptr; }
    T* operator->() { return m_ptr; }

    operator T&() { return *m_ptr; }

  private:
    T*         m_ptr{nullptr};
    MutexType* m_mutex{nullptr};
  };

  class ConstGuard {
    ConstGuard( const ConstGuard& a ) = delete;

  public:
    ConstGuard( ConstGuard&& a ) : m_ptr( a.m_ptr ), m_mutex( a.m_mutex ) { a.m_mutex = nullptr; }

    ConstGuard( const T* ptr, MutexType& mutex ) : m_ptr( ptr ), m_mutex( &mutex ) { m_mutex->lock(); }
    ~ConstGuard() { m_mutex->unlock(); }
    const T& operator*() const { return *m_ptr; }
    const T* operator->() const { return m_ptr; }

    operator const T&() const { return *m_ptr; }

  private:
    const T*   m_ptr;
    MutexType* m_mutex;
  };

  /// Aquire and release the lock before and after the object is accessed.
  Guard operator*() { return Guard( m_ptr, *m_mutex ); }

  /// Aquire and release the lock before and after the object is accessed.
  Guard operator->() { return Guard( m_ptr, *m_mutex ); }

  /// Aquire and release the lock before and after the const object is accessed.
  ConstGuard operator*() const { return ConstGuard( m_ptr, *m_mutex ); }

  /// Aquire and release the lock before and after the const object is accessed.
  ConstGuard operator->() const { return ConstGuard( m_ptr, *m_mutex ); }

  operator bool() const { return m_ptr; }

private:
  T*                 m_ptr;
  mutable MutexType* m_mutex;
};

#endif

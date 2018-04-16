#ifndef _GAUDI_CONTEXTSPECIFICPTR_H_
#define _GAUDI_CONTEXTSPECIFICPTR_H_

#include <functional>
#include <mutex>
#include <numeric>
#include <type_traits>
#include <unordered_map>

// For the definition of GAUDI_API
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ThreadLocalContext.h"

class EventContext;

namespace Gaudi
{
  namespace Hive
  {
    /**
     *  Simple implementation of a smart pointer with different values for
     *  different event contexts (slots).
     *
     *  When the copy for a new context is requested, the returned pointer is
     *  null.
     *
     *  The interface is meant to allow for a drop-in replacement for regular
     *  pointers. It's still responsibility of the user to delete the memory
     *  associated to the pointers.
     */
    template <typename T>
    class ContextSpecificPtr
    {
    private:
      /// Type used for the internal storage.
      typedef std::unordered_map<ContextIdType, T*> StorageType;

    public:
      /// Return the pointer for the current context (null for a new context).
      inline T* get() const
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        return m_ptrs[currentContextId()];
      }
      /// Set the pointer for the current context.
      inline T*& set( T* ptr )
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        return m_ptrs[currentContextId()] = ptr;
      }

      /// Assignment operator (@see set).
      inline T*& operator=( T* ptr ) { return set( ptr ); }

      /// Return true if the pointer is not null.
      inline bool isValid() const { return get(); }

      /// Conversion to boolean (@see isValid).
      inline operator bool() const { return isValid(); }

      /// Comparison with another pointer.
      inline bool operator==( T* rhs ) const { return get() == rhs; }

      /// @{ Dereference operators.
      inline T& operator*() { return *get(); }
      inline const T& operator*() const { return *get(); }
      inline T* operator->() { return get(); }
      inline const T* operator->() const { return get(); }
      /// @}

      /// @{Non thread-safe methods.

      /// Set to null all the used pointers.
      void clear() { m_ptrs.clear(); }

      /// Taking a function f that from a T* produces a value, return the sum of
      /// all the values corresponding to the contained pointers using init as
      /// first value.
      template <class Mapper>
      typename std::result_of<Mapper( const T* )>::type
      accumulate( Mapper f, typename std::result_of<Mapper( const T* )>::type init ) const
      {
        typedef typename std::result_of<Mapper( const T* )>::type R;
        return accumulate( f, init, std::plus<R>() );
      }

      /// Taking a function f that from a T* produces a value, return the
      /// accumulated  result, through the operation 'op', of all the values
      /// corresponding to the contained pointers using init as first value.
      template <class Mapper, class BinaryOperation>
      typename std::result_of<Mapper( const T* )>::type
      accumulate( Mapper f, typename std::result_of<Mapper( const T* )>::type init, BinaryOperation op ) const
      {
        typedef typename std::result_of<Mapper( const T* )>::type R;
        typedef typename StorageType::value_type                  V;
        std::lock_guard<std::mutex>                               lock( m_ptrs_lock );
        return std::accumulate( m_ptrs.begin(), m_ptrs.end(), init, [&f, &op]( const R& partial, const V& p ) -> R {
          return op( partial, f( p.second ) );
        } );
      }

      /// Call a function on each contained pointer.
      template <class F>
      void for_each( F f ) const
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        for ( auto& i : m_ptrs ) f( i.second );
      }

      /// Call a function on each contained pointer. (non-const version)
      template <class F>
      void for_each( F f )
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        for ( auto& i : m_ptrs ) f( i.second );
      }

      /// Call a function on each element, passing slot# as well
      template <class F>
      void for_all( F f ) const
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        for ( auto& i : m_ptrs ) f( i.first, i.second );
      }
      template <class F>
      void for_all( F f )
      {
        std::lock_guard<std::mutex> lock( m_ptrs_lock );
        for ( auto& i : m_ptrs ) f( i.first, i.second );
      }

      void deleteAll()
      {
        for_each( []( T*& p ) {
          delete p;
          p = nullptr;
        } );
      }

    private:
      /// Internal storage for the different internal pointers.
      mutable StorageType m_ptrs;
      /// Mutex for the m_ptrs container.
      mutable std::mutex m_ptrs_lock;
    };

    /**
     *  Implementation of a context specific storage accessible as a sort of
     *  smart reference class.
     *
     *  New values are created from the prototype passed to the constructor.
     */
    template <typename T>
    class ContextSpecificData
    {
    public:
      /// Constructor with default initialization.
      ContextSpecificData() : m_proto() {}
      /// Constructor with prototype value.
      ContextSpecificData( const T& proto ) : m_proto( proto ) {}

      /// Destructor.
      ~ContextSpecificData() { m_ptr.deleteAll(); }

      inline operator T&()
      {
        if ( m_ptr )
          return *m_ptr;
        else
          return *( m_ptr = new T( m_proto ) );
      }

      inline operator T&() const
      {
        if ( m_ptr )
          return *m_ptr;
        else
          return *( m_ptr = new T( m_proto ) );
      }

      /// Assignment operator.
      inline T& operator=( const T& other ) { return (T&)( *this ) = other; }

      /// Return the sum of all the contained values using init as first value.
      inline T accumulate( T init ) const { return accumulate( init, std::plus<T>() ); }

      /// Return the accumulated result, through the operation 'op', of all the
      /// contained values using init as first value.
      template <class T1, class BinaryOperation>
      T1 accumulate( T1 init, BinaryOperation op ) const
      {
        return m_ptr.accumulate( []( const T* p ) -> T { return *p; }, init, op );
      }

      /// Call a function on each contained value.
      template <class F>
      void for_each( F f ) const
      {
        m_ptr.for_each( [&f]( const T* p ) { f( *p ); } );
      }

      /// Call a function on each contained value. (non-const version)
      template <class F>
      void for_each( F f )
      {
        m_ptr.for_each( [&f]( T* p ) { f( *p ); } );
      }

      /// Call a function on each element, passing slot# as well
      template <class F>
      void for_all( F f ) const
      {
        m_ptr.for_all( [&f]( size_t s, const T* p ) { f( s, *p ); } );
      }
      template <class F>
      void for_all( F f )
      {
        m_ptr.for_all( [&f]( size_t s, T* p ) { f( s, *p ); } );
      }

    private:
      // FIXME: implement a proper copy constructor
      ContextSpecificData( const ContextSpecificData& ) = delete;

      /// Prototype value.
      T m_proto;
      /// Internal implementation.
      mutable ContextSpecificPtr<T> m_ptr;
    };
  }
}

#endif // _GAUDI_CONTEXTSPECIFICPTR_H_

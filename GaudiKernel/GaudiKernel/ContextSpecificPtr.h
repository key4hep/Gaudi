#ifndef _GAUDI_CONTEXTSPECIFICPTR_H_
#define _GAUDI_CONTEXTSPECIFICPTR_H_

#include <unordered_map>
#include <mutex>
#include <numeric>
#include <functional>
#include <type_traits>

// For the definition of GAUDI_API
#include "Kernel.h"

namespace Gaudi {
  namespace Hive {
    /// Internal type used for the ContextId.
    typedef size_t ContextIdType;

    /// Return the current context id.
    /// The returned id is valid only within the (sys)Execute method of
    /// algorithms.
    GAUDI_API ContextIdType currentContextId();

    /// Used by the framework to change the value of the current context id.
    GAUDI_API void setCurrentContextId(ContextIdType newId);

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
    template< typename T >
    class ContextSpecificPtr {
    private:
      /// Type used for the internal storage.
      typedef std::unordered_map<ContextIdType, T*> StorageType;
    public:

      /// Return the pointer for the current context (null for a new context).
      inline T* get() const {
        std::lock_guard<std::mutex> lock(m_ptrs_lock);
        return m_ptrs[currentContextId()];
      }
      /// Set the pointer for the current context.
      inline T*& set(T* ptr) {
        std::lock_guard<std::mutex> lock(m_ptrs_lock);
        return m_ptrs[currentContextId()] = ptr;
      }

      /// Assignment operator (@see set).
      inline T*& operator= (T* ptr) { return set(ptr); }

      /// Return true if the pointer is not null.
      inline bool isValid() const { return get(); }

      /// Conversion to boolean (@see isValid).
      inline operator bool () const { return isValid(); }

      /// Comparison with another pointer.
      inline bool operator==(T* rhs) const { return get() == rhs; }

      /// @{ Dereference operators.
      inline T& operator* () { return *get(); }
      inline const T& operator* () const { return *get(); }
      inline T* operator-> () { return get(); }
      inline const T* operator-> () const { return get(); }
      /// @}

      /// @{Non thread-safe methods.

      /// Set to null all the used pointers.
      void clear() { m_ptrs.clear(); }

      /// Taking a function f that from a T* produces a value, return the sum
      /// of all the values corresponding to the contained pointers using init
      /// as first value.
      template< class Mapper >
      typename std::result_of<Mapper(const T*)>::type
      accumulate(Mapper f, typename std::result_of<Mapper(const T*)>::type init) const {
        typedef typename std::result_of<Mapper(const T*)>::type R;
        return accumulate(f, init, std::plus<R>());
      }

      /// Taking a function f that from a T* produces a value, return the sum
      /// of all the values using init as first value.
      template< class Mapper, class BinaryOperation >
      typename std::result_of<Mapper(const T*)>::type
      accumulate(Mapper f, typename std::result_of<Mapper(const T*)>::type init,
                 BinaryOperation op) const {
        typedef typename std::result_of<Mapper(const T*)>::type R;
        typedef typename StorageType::value_type V;
        std::lock_guard<std::mutex> lock(m_ptrs_lock);
        return std::accumulate(m_ptrs.begin(), m_ptrs.end(), init,
          [&f, &op](const R& partial, const V& p) -> R {
          return op(partial, f(p.second));
        });
      }

      template< class F >
      void for_each(F f) const {
        std::lock_guard<std::mutex> lock(m_ptrs_lock);
        for(auto& i: m_ptrs) f(i.second);
      }

      template< class F >
      void for_each(F f) {
        std::lock_guard<std::mutex> lock(m_ptrs_lock);
        for(auto& i: m_ptrs) f(i.second);
      }

      void deleteAll() {
        for_each([](T*& p) {delete p; p = nullptr;} );
      }


    private:
      /// Internal storage for the different internal pointers.
      mutable StorageType m_ptrs;
      /// Mutex for the m_ptrs container.
      mutable std::mutex m_ptrs_lock;
    };

  }
}

#endif // _GAUDI_CONTEXTSPECIFICPTR_H_

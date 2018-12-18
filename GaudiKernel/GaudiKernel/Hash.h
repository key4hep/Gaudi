#ifndef GAUDIKERNEL_HASH_H
#define GAUDIKERNEL_HASH_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <cstddef>
#include <numeric>
// ============================================================================
// Boost
// ============================================================================
#include "boost/functional/hash.hpp"
// ============================================================================
namespace GaudiUtils
{
  /// Generic hash implementation (for easy migration to the new Hash class).
  //  To enable the generic hash for a user defined class, it is enough to add
  //  few lines to the .cpp files that needs them
  //  @code
  //  #include "GaudiKernel/Hash.h"
  //  class MyType;
  //  namespace GaudiUtils {
  //    template <>
  //    struct Hash<MyType>: public GenericHash<MyType> {};
  //  }
  //  @endcode
  template <class T>
  struct GenericHash {
    // ========================================================================
    /// the generic hash function
    inline std::size_t operator()( const T& key ) const
    {
      const char* p = reinterpret_cast<const char*>( &key );
      return std::accumulate( p, p + sizeof( T ), std::size_t{0},
                              []( std::size_t res, const char& c ) { return ( res << 1 ) ^ c; } );
    }
    // ========================================================================
  };

  // ==========================================================================
  /** @class Hash Hash.h GaudiKernel/Hash.h
   *
   * Simple hash function.
   * The default implementation is based on boost::hash
   *
   * @attention There is no generic a'la Stroustrup implementation
   *            valid for "any" objects. @see GaudiUtils::GenericHash
   * @attention The hashing of pointers does not rely on hash for objects
   *
   * The usage *requires* to provide the specialization
   * The default specialization are provided by boost::bash_value for
   *  - all primitive fundamental types
   *  - strings and wide strings
   *  - std::complex
   *  - std::pair
   *  - std::vector, std::list, std::deque
   *  - std::set, std::multiset, std::map, std::multimap
   *  - C-arrays, including C-strings
   *  - generic pointer types
   *
   * To provide own specialization for type TYPE or TYPE* there are four ways:
   *  - provide the specialization of
   *         Gaudi::Utils::Hash<TYPE>::operator()  ( const TYPE& val ) const
   *     or  Gaudi::Utils::Hash<TYPE*>::operator() ( const TYPE* val ) const
   *  - provide the full specialization of  Gaudi::Utils::Hash<TYPE>
   *  - provide the function  std::size_t hash_value ( const TYPE& val ) ;
   *
   * The first and the last ways are recommended.
   * The second way can be needed if e.g. TYPE has non trivial properties,
   * e.g. one need to use non-const or non-ref  forms of hashing value
   * For the last way the function must reside in the same namespace, where
   * TYPE is defines, therefore if TYPE is in a global namespace it is
   * better to avoid this way. The most natural place for this function is
   * a "friend function" of the class. In this way it can have access to some
   * useful internals of the class. However this is a bit intrusive way.
   * The third way is not recommended, since sooner or later we'll need to
   * migrate from boost::hash to std::tr1::hash.
   *
   * Comparing the previous Gaudi implementation of GaudiUtils::Hash and
   * boost::bash specializations, one can expect:
   *  - some (tiny?) CPU penalty for floating point types
   *  - some (tiny?) CPU penalty for long-long integer types
   *  - better CPU performance for generic pointer types
   *  - guaranteed coherence of hash for std::string & C-string
   *
   * Probably the first two items are compensated by better
   *  "hashing"-performance
   *
   * @author Marco Clemencic
   * @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   * @date 2005-10-07
   */
  template <class T>
  struct Hash {
    // ========================================================================
    /// the hash-function
    inline std::size_t operator()( const T& key ) const;
    // ========================================================================
  };
  // ==========================================================================
  /// the partial specialization for pointers
  template <class T>
  struct Hash<T*> {
    // ========================================================================
    /// the hash-function
    inline std::size_t operator()( const T* key ) const;
    // ========================================================================
  };
  // ==========================================================================
  /// generic specialization for arrays
  template <class T, unsigned N>
  struct Hash<T ( & )[N]> {
    // ========================================================================
    /// the hash-function
    inline std::size_t operator()( T ( &key )[N] ) const { return boost::hash_range( key, key + N ); }
    // ========================================================================
  };
  /// generic specialization for arrays
  template <class T, unsigned N>
  struct Hash<const T ( & )[N]> {
    // ========================================================================
    /// the hash-function
    inline std::size_t operator()( const T ( &key )[N] ) const { return boost::hash_range( key, key + N ); }
    // ========================================================================
  };
  // ==========================================================================
  /// remove extra qualifiers:
  template <class T>
  struct Hash<const T> : public Hash<T> {
  };
  /// remove extra qualifiers:
  template <class T>
  struct Hash<const T*> : public Hash<T*> {
  };
  /// remove extra qualifiers:
  template <class T>
  struct Hash<T&> : public Hash<T> {
  };
  /// remove extra qualifiers:
  template <class T>
  struct Hash<const T&> : public Hash<T> {
  };
  // ==========================================================================
  /// the generic implementations of hash-function
  template <class T>
  inline std::size_t Hash<T>::operator()( const T& key ) const
  {
    using namespace boost;
    return hash_value( key );
  }
  /// the generic implementation of hash for pointers
  template <class T>
  inline std::size_t Hash<T*>::operator()( const T* key ) const
  {
    using namespace boost;
    return hash_value( key );
  }
  /// (very)specific for C-strings
  template <>
  inline std::size_t Hash<char*>::operator()( const char* key ) const
  {
    std::size_t seed = 0;
    while ( key ) {
      boost::hash_combine( seed, *key );
      ++key;
    }
    return seed;
  }

  // ==========================================================================
} // end of namespace GaudiUtils
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_HASH_H

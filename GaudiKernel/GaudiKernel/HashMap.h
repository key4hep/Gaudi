// $Id: HashMap.h,v 1.7 2008/11/01 14:30:21 marcocle Exp $
#ifndef GAUDIKERNEL_HASHMAP_H
#define GAUDIKERNEL_HASHMAP_H 1

// Include files
#include "GaudiKernel/Map.h"
#include "GaudiKernel/Hash.h"

#if defined(_WIN32) || defined(__ECC)
#include <hash_map>
#elif defined(__ICC)
// icc uses the headers from GCC...
#include <ext/hash_map>
#elif __GNUC__ < 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ < 3 )
#include <ext/hash_map>
#elif __GNUC__ >= 4
// Marco Cl.: in gcc >= 4.3 the hash_map has been replaced by unordered_map
//#include <tr1/unordered_map>
//            but gccxml has problems with it
#include <ext/hash_map>
#endif

namespace GaudiUtils 
{
  // ==========================================================================
  /** @class HashMap HashMap.h GaudiKernel/HashMap.h
   *
   * Common class providing an architecture-independent hash map.
   *
   * 
    *  Due to helper base class Gaudi::Utils::MapBase, this class 
   *  is "python-friendly", and one can perform all python 
   *  manipulaitons 
   *  in intuitive way:
   *  @code 
   *  
   *    >>> m = ...        ## get the map 
   *    >>> print m        ## print the map a'la python class dict 
   *    ...   
   *    >>> for key in m :  print key, m[key]   ## iteration over the map
   *    ...
   *    >>> for key,value in m.iteritems() : print key, value 
   *    ...
   *    >>> keys   = m.keys()                     ## get the list of keys 
   *    >>> values = m.values ()                  ## get the list of values 
   *    >>  items  = m.items  ()                  ## get the list of items 
   *
   *    >>> if 'one' in m           ## check the presence of the key in map
   * 
   *    >>>  v = m.get(key', None) ## return m[key] for existing key, else None
   *
   *    >>>  del m[key]      ## erase the key form the map 
   *
   *    >>> value m[key]     ## unchecked access through the key
   *    ...
   *    >>> m.update( key, value ) ## update/insert key/value pair 
   *    
   *   @endcode 
   *  
   *   @attention One needs to redfine  <c>__setitem__</c> and <c>__getitem__</c>
   *              methods in python:
   *
   *   @code 
   *   
   *    >>> type(m).__setitem__ = Gaudi.Utils.MapBase.__setitem__ 
   *    >>> type(m).__getitem__ = Gaudi.Utils.MapBase.__getitem__ 
   *
   *    >>> m[key] = value  ## much more intuitive semantics for key insertion
   *
   *   @endcode 
   *   In a similar way <c>__delitem__</c> methods can be redefined.
   *   @warning
   *   To bypass some compilation problme (and to avoid the unnesessary 
   *   expansion of dictionaries) it is nesessary to exclude from 
   *   dictionary the following methods:
   *     - lower_bound 
   *     - upper_bound 
   *     - equal_range 
   *     - insert 
   *   
   *   @see Gaudi::Utils::MapBase 
   * 
   * @author Marco Clemencic
   * @date 2005-10-06
   */

  template <typename K, typename T,
            typename H = Hash<K>,
#if defined(_WIN32)
// Marco Cl.: (work-around) it seems that Windows implementation of hash map is extremely slow
// we use a normal map instead.
//            typename M = stdext::hash_map<K,T,H>
            typename M = std::map<K,T>
#elif defined(__ECC)
            typename M = std::hash_map<K,T,H>
#elif defined(__ICC)
            // icc uses the headers from GCC...
            typename M = __gnu_cxx::hash_map<K,T,H>
#elif __GNUC__ < 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ < 3 )
            typename M = __gnu_cxx::hash_map<K,T,H>
#elif __GNUC__ >= 4
// Marco Cl.: in gcc >= 4.3 the hash_map has been replaced by unordered_map
//          typename M = std::tr1::unordered_map<K,T,H>
            typename M = __gnu_cxx::hash_map<K,T,H>
//            but gccxml has problems with it
#endif
            >
  class HashMap: public Map<K,T,M> {

  public:
    typedef H hasher;

    inline hasher hash_funct() const {
// Marco Cl.: since on windows we are using a std::map, we have to provide a different implementation
// for GaudiUtils::HashMap::hash_funct(). (std::map::hash_funct does not exist)
#ifdef _WIN32
      return hasher();
#else
      return this->m_map.hash_funct();
#endif
    }
  };
} // namespace GaudiUtils

#endif // GAUDIKERNEL_GAUDIHASHMAP_H

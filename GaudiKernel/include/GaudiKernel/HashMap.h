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

// Include files
#include <GaudiKernel/Hash.h>
#include <GaudiKernel/Map.h>
#include <unordered_map>

namespace GaudiUtils {
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

  template <typename K, typename T, typename H = Hash<K>, typename M = std::unordered_map<K, T, H>>
  class HashMap : public Map<K, T, M> {
  public:
    typedef H     hasher;
    inline hasher hash_funct() const { return this->m_map.hash_funct(); }
  };
} // namespace GaudiUtils

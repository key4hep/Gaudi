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
/** @file GaudiKernel/SerializeSTL.h
 *
 * Provide serialization function (output only) for some common STL classes
 * (vectors, lists, pairs, maps) plus GaudiUtils::Map and GaudiUtils::HashMap.
 *
 * To use the serializer provided by this file, one should add
 * "using namespace GaudiUtils" (possibly inside the function (scope) calling "<<").
 *
 * @author Marco Clemencic
 * (adapted from the code found in LHCbKernel, original author unknown)
 */
#ifndef GAUDIKERNEL_SERIALIZESTL_H_
#define GAUDIKERNEL_SERIALIZESTL_H_

#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/Map.h"
#include <array>
#include <list>
#include <map>
#include <ostream>
#include <utility>
#include <vector>

#include "GaudiKernel/SerializeSTLFwd.h"

namespace GaudiUtils {
  namespace details {

    struct IdentityOutputter {
      template <typename T>
      std::ostream& operator()( std::ostream& os, T&& t ) const {
        return os << std::forward<T>( t );
      }
    };

    template <typename Stream, typename Iterator, typename Separator, typename OutputElement = IdentityOutputter>
    Stream& ostream_joiner( Stream& os, Iterator first, Iterator last, Separator sep,
                            OutputElement output = OutputElement{} ) {
      if ( first != last ) {
        output( os, *first );
        ++first;
      }
      for ( ; first != last; ++first ) { output( os << sep, *first ); }
      return os;
    }

    template <typename Stream, typename Container, typename Separator, typename OutputElement = IdentityOutputter>
    Stream& ostream_joiner( Stream& os, const Container& c, Separator sep, OutputElement output = OutputElement{} ) {
      return ostream_joiner( os, std::begin( c ), std::end( c ), sep, output );
    }
  } // namespace details

  /// Serialize an std::vector in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::vector<T, ALLOC>& v ) {
    return details::ostream_joiner( s << '[', v, ", " ) << ']';
  }

  /// Serialize an std::array in a python like format. E.g. "[1, 2, 3]".
  template <class T, std::size_t N>
  inline std::ostream& operator<<( std::ostream& s, const std::array<T, N>& v ) {
    return details::ostream_joiner( s << '[', v, ", " ) << ']';
  }

  /// Serialize an std::list in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::list<T, ALLOC>& l ) {
    return details::ostream_joiner( s << '[', l, ", " ) << ']';
  }

  /// Serialize an std::list in a python like format. E.g. "(1, 2)".
  template <class T1, class T2>
  inline std::ostream& operator<<( std::ostream& s, const std::pair<T1, T2>& p ) {
    return s << '(' << p.first << ", " << p.second << ')';
  }

  /// Serialize an std::map in a python like format. E.g. "{a: 1, b: 2}".
  template <class T1, class T2, class COMP, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::map<T1, T2, COMP, ALLOC>& m ) {
    return details::ostream_joiner( s << "{", m, ", ",
                                    []( std::ostream& os, const std::pair<const T1, T2>& p ) -> std::ostream& {
                                      return os << p.first << ": " << p.second;
                                    } )
           << "}";
  }

  /// Serialize a GaudiUtils::Map in a python like format. E.g. "{a: 1, b: 2}".
  template <class K, class T, class M>
  inline std::ostream& operator<<( std::ostream& s, const GaudiUtils::Map<K, T, M>& m ) {
    // Serialize the internal map.
    return s << static_cast<const M&>( m );
  }

  /// Serialize a GaudiUtils::HashMap in a python like format. E.g. "{a: 1, b: 2}".
  /// This implementation is not efficient, but very simple. Anyway a print-out
  /// of a hash map is not something that we do every second.
  template <class K, class T, class H, class M>
  inline std::ostream& operator<<( std::ostream& s, const GaudiUtils::HashMap<K, T, H, M>& m ) {
    // Copy the hash map into a map to have it ordered by key.
    return s << GaudiUtils::Map<K, T>( m.begin(), m.end() );
  }

} // namespace GaudiUtils

#endif /*GAUDIKERNEL_SERIALIZESTL_H_*/

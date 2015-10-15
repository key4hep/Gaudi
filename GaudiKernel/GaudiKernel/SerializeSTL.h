/** @file GaudiKernel/SerializeSTL.h
 *
 * Provide serialization function (output only) for some common STL classes
 * (vectors, lists, pairs, maps) plus GaudiUtils::Map and GaudiUtils::HashMap.
 *
 * To use the serializer provided by this file, one should add
 * "using namespace GaudiUtils" (possibly inside the function calling "<<").
 *
 * @author Marco Clemencic
 * (adapted from the code found in LHCbKernel, original author unknown)
 */
#ifndef GAUDIKERNEL_SERIALIZESTL_H_
#define GAUDIKERNEL_SERIALIZESTL_H_

#include <ostream>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include "GaudiKernel/Map.h"
#include "GaudiKernel/HashMap.h"

#include "GaudiKernel/SerializeSTLFwd.h"

namespace GaudiUtils {
  namespace detail {

  struct Identity {
    template <typename T>
    std::ostream& operator()(std::ostream& os, T&& t) const {
      return os << std::forward<T>(t);
    }
  };

  template <typename Stream, typename Container, typename Separator, typename OutputElement = Identity>
  Stream& ostream_joiner(Stream& os, const Container& c, Separator sep, OutputElement output = OutputElement{}) {
    auto first = std::begin(c); auto last = std::end(c);
    if (first!=last) { output(os,*first); ++first; }
    for (;first!=last;++first) { output(os << sep,*first); }
    return os;
  }

  }

  /// Serialize an std::vector in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<< ( std::ostream& s, const std::vector<T,ALLOC>& v )
  {
    return detail::ostream_joiner( s << '[', v, ", " ) << ']';
  }

  /// Serialize an std::list in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<< ( std::ostream& s, const std::list<T,ALLOC>& l )
  {
    return detail::ostream_joiner( s << '[', l, ", " ) << ']';
  }

  /// Serialize an std::list in a python like format. E.g. "(1, 2)".
  template <class T1, class T2>
  inline std::ostream& operator<< ( std::ostream& s, const std::pair<T1,T2>& p )
  {
    return s << '(' << p.first << ", " << p.second << ')';
  }

  /// Serialize an std::map in a python like format. E.g. "{a: 1, b: 2}".
  template <class T1, class T2, class COMP, class ALLOC>
  inline std::ostream& operator << ( std::ostream& s,
                                     const std::map<T1,T2,COMP,ALLOC>& m )
  {
    return detail::ostream_joiner( s << "{", m,  ", ",
                                   [](std::ostream& os, const std::pair<const T1,T2>& p)
                                   -> std::ostream&
                                   { return os << p.first << ": " << p.second; } )
           << "}";
  }

  /// Serialize a GaudiUtils::Map in a python like format. E.g. "{a: 1, b: 2}".
  template <class K, class T, class M>
  inline std::ostream& operator << ( std::ostream& s,
                                     const GaudiUtils::Map<K,T,M>& m )
  {
    // Serialize the internal map.
    return s << (M)m;
  }

  /// Serialize a GaudiUtils::HashMap in a python like format. E.g. "{a: 1, b: 2}".
  /// This implementation is not efficient, but very simple. Anyway a print-out
  /// of a hash map is not something that we do every second.
  template <class K, class T, class H, class M>
  inline std::ostream& operator << ( std::ostream& s,
                                     const GaudiUtils::HashMap<K,T,H,M>& m )
  {
    // Copy the hash map into a map to have it ordered by key.
    return s << GaudiUtils::Map<K,T>(m.begin(),m.end());
  }

} // namespace GaudiUtils

#endif /*GAUDIKERNEL_SERIALIZESTL_H_*/

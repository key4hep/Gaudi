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
/** Forward declarations for the functions in SerializeSTL.h.
 *
 * @author Marco Clemencic
 *
 * @see SerializeSTL.h
 */
#ifndef GAUDIKERNEL_SERIALIZESTLFWD_H_
#define GAUDIKERNEL_SERIALIZESTLFWD_H_

#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/Map.h"
#include <list>
#include <map>
#include <ostream>
#include <utility>
#include <vector>

namespace GaudiUtils {
  /// Serialize an std::vector in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::vector<T, ALLOC>& v );

  /// Serialize an std::list in a python like format. E.g. "[1, 2, 3]".
  template <class T, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::list<T, ALLOC>& l );

  /// Serialize an std::list in a python like format. E.g. "(1, 2)".
  template <class T1, class T2>
  inline std::ostream& operator<<( std::ostream& s, const std::pair<T1, T2>& p );

  /// Serialize an std::map in a python like format. E.g. "{a: 1, b: 2}".
  template <class T1, class T2, class COMP, class ALLOC>
  inline std::ostream& operator<<( std::ostream& s, const std::map<T1, T2, COMP, ALLOC>& m );

  /// Serialize a GaudiUtils::Map in a python like format. E.g. "{a: 1, b: 2}".
  template <class K, class T, class M>
  inline std::ostream& operator<<( std::ostream& s, const GaudiUtils::Map<K, T, M>& m );

  /// Serialize a GaudiUtils::HashMap in a python like format. E.g. "{a: 1, b: 2}".
  /// This implementation is not efficient, but very simple. Anyway a print-out
  /// of a hash map is not something that we do every second.
  template <class K, class T, class H, class M>
  inline std::ostream& operator<<( std::ostream& s, const GaudiUtils::HashMap<K, T, H, M>& m );
} // namespace GaudiUtils

#endif /*GAUDIKERNEL_SERIALIZESTLFWD_H_*/

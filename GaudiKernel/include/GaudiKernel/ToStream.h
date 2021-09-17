/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
#ifndef GAUDIPROPERTYPARSERS_PARSERVALUETOSTREAM_H
#define GAUDIPROPERTYPARSERS_PARSERVALUETOSTREAM_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <array>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/Map.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/VectorMap.h"
// ============================================================================
/** @file GaudiKernel/ToStream.h
 *  implementation of various functions for streaming.
 *  this functionality is essential for usage of various types as property for
 *  the various Gaudi components
 *  @attention the implementation of the specific specializations must be done
 *                    before the inclusion of this file
 *  @todo ToStream.h : reimplement in terms of functors, to allow
 *                     easier specializations
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Utils {
    // ========================================================================
    /** the generic implementation of the printout to the std::ostream
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class TYPE>
    std::ostream& toStream( const TYPE& obj, std::ostream& s );
    // ========================================================================
    /** the helper function to print the sequence
     *  @param first (INPUT)  begin-iterator for the sequence
     *  @param last  (INPUT)  end-iterator for the sequence
     *  @param s     (UPDATE) the stream itself
     *  @param open  (INPUT)  "open"-symbol
     *  @param close (INPUT)  "close"-symbol
     *  @param delim (INPUT)  "delimiter"-symbol
     *  @return the stream
     *  @author Vanya BELYAEV Ivan.BElyaev@nikhef.nl
     *  @date 2009-09-15
     */
    template <class ITERATOR>
    inline std::ostream& toStream( ITERATOR           first,   // begin of the sequence
                                   ITERATOR           last,    //   end of the sequence
                                   std::ostream&      s,       //            the stream
                                   const std::string& open,    //               opening
                                   const std::string& close,   //               closing
                                   const std::string& delim ); //             delimiter
    // ========================================================================
    /** the printtout of the strings.
     *  the string is printed a'la Python using the quotes
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    inline std::ostream& toStream( const std::string& obj, std::ostream& s ) { return s << std::quoted( obj, '\'' ); }
    /** the printout of boolean values "a'la Python"
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream( const bool obj, std::ostream& s ) { return s << ( obj ? "True" : "False" ); }
    /** the printout of float values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream( const float obj, std::ostream& s, const int prec = 6 ) {
      const int p = static_cast<int>( s.precision() );
      return s << std::setprecision( prec ) << obj << std::setprecision( p );
    }
    /** the printout of double values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream( const double obj, std::ostream& s, const int prec = 8 ) {
      const int p = static_cast<int>( s.precision() );
      return s << std::setprecision( prec ) << obj << std::setprecision( p );
    }
    /** the printout of long double values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream( const long double obj, std::ostream& s, const int prec = 10 ) {
      const int p = static_cast<int>( s.precision() );
      return s << std::setprecision( prec ) << obj << std::setprecision( p );
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>std::pair<KTYPE,VTYPE></c> printout
     *  the pair is printed a'la Python tuple: " ( a , b )"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class KTYPE, class VTYPE>
    inline std::ostream& toStream( const std::pair<KTYPE, VTYPE>& obj, std::ostream& s ) {
      return toStream( obj.second, toStream( obj.first, s << "( " ) << " , " ) << " )";
    }
    // ========================================================================
    /** the partial template specialization of <c>std::vector<TYPE,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class TYPE, class ALLOCATOR>
    inline std::ostream& toStream( const std::vector<TYPE, ALLOCATOR>& obj, std::ostream& s ) {
      return toStream( obj.begin(), obj.end(), s, "[ ", " ]", " , " );
    }
    // ========================================================================
    /** the partial template specialization of <c>std::list<TYPE,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-04-08
     */
    template <class TYPE, class ALLOCATOR>
    inline std::ostream& toStream( const std::list<TYPE, ALLOCATOR>& obj, std::ostream& s ) {
      return toStream( obj.begin(), obj.end(), s, "[ ", " ]", " , " );
    }
    // ========================================================================
    /** the partial template specialization of <c>std::set<TYPE,CMP,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class TYPE, class CMP, class ALLOCATOR>
    inline std::ostream& toStream( const std::set<TYPE, CMP, ALLOCATOR>& obj, std::ostream& s ) {
      return toStream( obj.begin(), obj.end(), s, "[ ", " ]", " , " );
    }
    // ========================================================================
    /** the partial template specialization of <c>std::unordered_set<TYPE,HASH,CMP,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     */
    template <class TYPE, class HASH, class CMP, class ALLOCATOR>
    inline std::ostream& toStream( const std::unordered_set<TYPE, HASH, CMP, ALLOCATOR>& obj, std::ostream& s ) {
      return toStream( obj.begin(), obj.end(), s, "[", "]", " , " );
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>std::map<KTYPE,VTYPE,CMP,ALLOCATOR></c> printout
     *  the map is printed a'la Python dict: " ( a : b , c: d , e : f )"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class KTYPE, class VTYPE, class CMP, class ALLOCATOR>
    inline std::ostream& toStream( const std::map<KTYPE, VTYPE, CMP, ALLOCATOR>& obj, std::ostream& s ) {
      using GaudiUtils::details::ostream_joiner;
      return ostream_joiner( s << "{ ", obj, " , ",
                             []( std::ostream& os, const std::pair<const KTYPE, VTYPE>& i ) -> std::ostream& {
                               return toStream( i.second, toStream( i.first, os ) << " : " );
                             } )
             << " }";
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>GaudiUtils::VectorMap<KTYPE,VTYPE,CMP,ALLOCATOR></c> printout
     *  the map is printed a'la Python dict: " ( a : b , c: d , e : f )"
     *  @see GaudiUtils::VectorMap
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class KTYPE, class VTYPE, class CMP, class ALLOCATOR>
    inline std::ostream& toStream( const GaudiUtils::VectorMap<KTYPE, VTYPE, CMP, ALLOCATOR>& obj, std::ostream& s ) {
      using GaudiUtils::details::ostream_joiner;
      return ostream_joiner( s << "{ ", obj, " , ",
                             []( std::ostream& os, const std::pair<const KTYPE, VTYPE>& i ) -> std::ostream& {
                               return toStream( i.second, toStream( i.first, os ) << " : " );
                             } )
             << " }";
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>GaudiUtils::Map<KTYPE,VTYPE,MAP></c> printout
     *  the map is printed a'la Python dict: " ( a : b , c: d , e : f )"
     *  @see GaudiUtils::VectorMap
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class KTYPE, class VTYPE, class MAP>
    inline std::ostream& toStream( const GaudiUtils::Map<KTYPE, VTYPE, MAP>& obj, std::ostream& s ) {
      using GaudiUtils::details::ostream_joiner;
      return ostream_joiner( s << "{ ", obj, " , ",
                             []( std::ostream& os, const std::pair<const KTYPE, VTYPE>& i ) -> std::ostream& {
                               return toStream( i.second, toStream( i.first, os ) << " : " );
                             } )
             << " }";
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>GaudiUtils::HashMap<KTYPE,VTYPE,HASH,MAP></c> printout
     *  the map is printed a'la Python dict: " ( a : b , c: d , e : f )"
     *  @see GaudiUtils::VectorMap
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class KTYPE, class VTYPE, class HASH, class MAP>
    inline std::ostream& toStream( const GaudiUtils::HashMap<KTYPE, VTYPE, HASH, MAP>& obj, std::ostream& s ) {
      // Copy the hash map into a map to have it ordered by key.
      return toStream( GaudiUtils::Map<KTYPE, VTYPE>{obj.begin(), obj.end()}, s );
    }
    // ========================================================================
    /** the specialization for C-arrays, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-05
     */
    template <class TYPE, unsigned int N>
    std::ostream& toStream( const TYPE ( &obj )[N], std::ostream& s ) {
      if constexpr ( N == 1 ) {
        return toStream( obj[0], s << "( " ) << " , )";
      } else {
        return toStream( obj, obj + N, s, "( ", " )", " , " );
      }
    }
    // ========================================================================
    /** the specialization for std::array, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-16
     */
    template <class TYPE, std::size_t N>
    std::ostream& toStream( const std::array<TYPE, N>& obj, std::ostream& s ) {
      if constexpr ( N == 1 ) {
        return toStream( obj[0], s << "( " ) << " , )";
      } else {
        return toStream( begin( obj ), end( obj ), s, "( ", " )", " , " );
      }
    }
    // ========================================================================
    /** the specialization for C-string, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-05
     */
    template <unsigned int N>
    std::ostream& toStream( const char ( &obj )[N], std::ostream& s ) {
      return toStream( std::string( obj, obj + N ), s );
    }
    // ========================================================================
    /** the specialization for C-string, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-05
     */
    inline std::ostream& toStream( const char* obj, std::ostream& s ) { return toStream( std::string( obj ), s ); }
    // ========================================================================
    /** the generic implementation of the printout to the std::ostream
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template <class TYPE>
    inline std::ostream& toStream( const TYPE& obj, std::ostream& s ) {
      return s << obj;
    }
    // ========================================================================
    /** the helper function to print the sequence
     *  @param first (INPUT)  begin-iterator for the sequence
     *  @param last  (INPUT)  end-iterator for the sequence
     *  @param s     (UPDATE) the stream itself
     *  @param open  (INPUT)  "open"-symbol
     *  @param close (INPUT)  "close"-symbol
     *  @param delim (INPUT)  "delimiter"-symbol
     *  @return the stream
     *  @author Vanya BELYAEV Ivan.BElyaev@nikhef.nl
     *  @date 2009-09-15
     */
    template <class ITERATOR>
    inline std::ostream& toStream( ITERATOR           first,  // begin of the sequence
                                   ITERATOR           last,   //   end of the sequence
                                   std::ostream&      s,      //            the stream
                                   const std::string& open,   //               opening
                                   const std::string& close,  //               closing
                                   const std::string& delim ) //             delimiter
    {
      using ref_t = typename std::iterator_traits<ITERATOR>::reference;
      using GaudiUtils::details::ostream_joiner;
      return ostream_joiner( s << open, first, last, delim,
                             []( std::ostream& os, ref_t i ) -> std::ostream& { return toStream( i, os ); } )
             << close;
    }
    // ========================================================================
    // helper function to print a tuple of any size
    template <class Tuple, std::size_t N>
    struct TuplePrinter {
      static std::ostream& toStream( const Tuple& t, std::ostream& s ) {
        TuplePrinter<Tuple, N - 1>::toStream( t, s ) << " , ";
        return Gaudi::Utils::toStream( std::get<N - 1>( t ), s );
      }
    };

    template <class Tuple>
    struct TuplePrinter<Tuple, 1> {
      static std::ostream& toStream( const Tuple& t, std::ostream& s ) {
        return Gaudi::Utils::toStream( std::get<0>( t ), s );
      }
    };

    /** the helper function to print the tuple
     *  @param tuple (INPUT)  tuple
     *  @return the stream
     *  @author Aleander Mazurov alexander.mazurov@cern.ch
     *  @date 2015-03-21
     */
    template <typename... Args>
    inline std::ostream& toStream( const std::tuple<Args...>& tuple, std::ostream& s ) {
      auto& out = TuplePrinter<decltype( tuple ), sizeof...( Args )>::toStream( tuple, s << " ( " );
      if constexpr ( std::tuple_size_v<std::tuple<Args...>> == 1 ) { // this is a special case in Python
        out << " ,";
      }
      return out << " ) ";
    }

    // ========================================================================
    /** the generic implementation of the type conversion to the string
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     *  @see Gaudi::Utils::toStream
     *  @todo need to be compared with boost::lexical_cast
     */
    template <class TYPE>
    inline std::string toString( const TYPE& obj ) {
      std::ostringstream s;
      std::ios::fmtflags orig_flags = s.flags();
      s.setf( std::ios::showpoint ); // to display correctly floats
      toStream( obj, s );
      s.flags( orig_flags );
      return s.str();
    }
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif

// $Id: ToStream.h,v 1.8 2008/10/30 23:38:46 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.8 $
// ============================================================================
#ifndef GAUDIPROPERTYPARSERS_PARSERVALUETOSTREAM_H
#define GAUDIPROPERTYPARSERS_PARSERVALUETOSTREAM_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <string>
#include <sstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Map.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/VectorMap.h"
// ============================================================================
/** @file GaudiKernel/ToStream.h
 *  implemenattiono fvarioud functions for streaming.
 *  this functionality is essenital for usag eof varuodu types as property for
 *  the various Gaudi components
 *  @attention the implemenation of the specific specializations must be done
 *                    before the inclusion of this file
 *  @todo ToStream.h : reimplement in terms of functors, to allow
 *                     easier especializations
 */
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils
  {
    // ========================================================================
    /** the generic implementation of the printout to the std::ostream
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class TYPE>
    std::ostream& toStream ( const TYPE& obj, std::ostream& s ) ;
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
    inline std::ostream& toStream
    ( ITERATOR           first ,                       // begin of the sequence
      ITERATOR           last  ,                       //   end of the sequence
      std::ostream&      s     ,                       //            the stream
      const std::string& open  ,                       //               opening
      const std::string& close ,                       //               closing
      const std::string& delim ) ;                     //             delimiter
    // ========================================================================
    /** the printtout of the strings.
     *  the string is printed a'la Python using the quotes
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    inline std::ostream& toStream
    ( const std::string& obj , std::ostream& s )
    {
      if ( std::string::npos == obj.find('\'') )
      { s << "\'" << obj << "\'" ; }
      else
      { s << "\"" << obj << "\"" ; }
      return s ;
    }
    /** the printout of boolean values "a'la Python"
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream
    ( const bool         obj , std::ostream& s )
    { return s << ( obj ? "True" : "False" ) ; }
    /** the printout of float values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream
    ( const float        obj , std::ostream& s , const int prec = 6 )
    {
      const int  p = s.precision() ;
      return s << std::setprecision (  prec ) << obj << std::setprecision ( p ) ;
    }
    /** the printout of double values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream
    ( const double       obj , std::ostream& s , const int prec = 8 )
    {
      const int p = s.precision() ;
      return s << std::setprecision ( prec ) << obj << std::setprecision ( p ) ;
    }
    /** the printout of long double values with the reasonable precision
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-09-09
     */
    inline std::ostream& toStream
    ( const long double  obj , std::ostream& s , const int prec = 10 )
    {
      const int p = s.precision() ;
      return s << std::setprecision ( prec ) << obj << std::setprecision ( p ) ;
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>std::pair<KTYPE,VTYPE></c> printout
     *  the pair is printed a'la Python tuple: " ( a , b )"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class KTYPE, class VTYPE>
    inline std::ostream& toStream
    ( const std::pair<KTYPE,VTYPE>& obj, std::ostream& s)
    {
      s << "( "  ;
      toStream ( obj.first  , s ) ;
      s << " , " ;
      toStream ( obj.second , s ) ;
      return s << " )" ;
    }
    // ========================================================================
    /** the partial template specialization of <c>std::vector<TYPE,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class TYPE,class ALLOCATOR>
    inline std::ostream& toStream
    ( const std::vector<TYPE,ALLOCATOR>& obj, std::ostream& s)
    {
      return toStream ( obj.begin() , obj.end () , s , "[ " , " ]" , " , " ) ;
    }
    // ========================================================================
    /** the partial template specialization of <c>std::list<TYPE,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-04-08
     */
    template<class TYPE,class ALLOCATOR>
    inline std::ostream& toStream
    ( const std::list<TYPE,ALLOCATOR>& obj, std::ostream& s)
    {
      return toStream ( obj.begin() , obj.end () , s , "[ " , " ]" , " , " ) ;
    }
    // ========================================================================
    /** the partial template specialization of <c>std::set<TYPE,CMP,ALLOCATOR></c>
     *  printout. The vector is printed a'la Python list: "[ a, b, c ]"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class TYPE,class CMP,class ALLOCATOR>
    inline std::ostream& toStream
    ( const std::set<TYPE,CMP,ALLOCATOR>& obj, std::ostream& s)
    {
      return toStream ( obj.begin() , obj.end () , s , "[ " , " ]" , " , " ) ;
    }
    // ========================================================================
    /** the partial template specialization of
     *  <c>std::map<KTYPE,VTYPE,CMP,ALLOCATOR></c> printout
     *  the map is printed a'la Python dict: " ( a : b , c: d , e : f )"
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class KTYPE, class VTYPE,class CMP,class ALLOCATOR>
    inline std::ostream& toStream
    ( const std::map<KTYPE,VTYPE,CMP,ALLOCATOR>& obj, std::ostream& s )
    {
      s << "{ ";
      for ( typename std::map<KTYPE,VTYPE,CMP,ALLOCATOR>::const_iterator cur =
              obj.begin() ; obj.end() != cur ; ++cur )
      {
        if ( obj.begin() != cur ) { s << " , " ; }
        toStream ( cur -> first  , s ) ;
        s << " : " ;
        toStream ( cur -> second , s ) ;
      }
      return s << " }";
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
    template<class KTYPE, class VTYPE,class CMP,class ALLOCATOR>
    inline std::ostream& toStream
    ( const GaudiUtils::VectorMap<KTYPE,VTYPE,CMP,ALLOCATOR>& obj, std::ostream& s )
    {
      s << "{ ";
      for ( typename GaudiUtils::VectorMap<KTYPE,VTYPE,CMP,ALLOCATOR>::const_iterator cur = obj.begin() ;
            obj.end() != cur ; ++cur )
      {
        if ( obj.begin() != cur ) { s << " , " ; }
        toStream ( cur -> first  , s ) ;
        s << " : " ;
        toStream ( cur -> second , s ) ;
      }
      return s << " }";
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
    template<class KTYPE, class VTYPE,class MAP>
    inline std::ostream& toStream
    ( const GaudiUtils::Map<KTYPE,VTYPE,MAP>& obj, std::ostream& s)
    {
      s << "{ ";
      for ( typename GaudiUtils::Map<KTYPE,VTYPE,MAP>::const_iterator cur = obj.begin() ;
            obj.end() != cur ; ++cur )
      {
        if ( obj.begin() != cur ) { s << " , " ; }
        toStream ( cur -> first  , s ) ;
        s << " : " ;
        toStream ( cur -> second , s ) ;
      }
      return s << " }";
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
    template<class KTYPE, class VTYPE,class HASH,class MAP>
    inline std::ostream& toStream
    ( const GaudiUtils::HashMap<KTYPE,VTYPE,HASH,MAP>& obj, std::ostream& s)
    {
      s << "{ ";
      for ( typename GaudiUtils::HashMap<KTYPE,VTYPE,HASH,MAP>::const_iterator cur = obj.begin() ;
            obj.end() != cur ; ++cur )
      {
        if ( obj.begin() != cur ) { s << " , " ; }
        toStream ( cur -> first  , s ) ;
        s << " : " ;
        toStream ( cur -> second , s ) ;
      }
      return s << " }";
    }
    // ========================================================================
    /** the specialization for C-arrays, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhenf.nl
     *  @date 2009-10-05
     */
    template <class TYPE, unsigned int N>
    std::ostream& toStream (       TYPE(&obj)[N] , std::ostream& s )
    {
      return toStream ( obj , obj + N , s , "( " , " )" , " , " ) ;
    }
    // ========================================================================
    /** the specialization for C-arrays, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhenf.nl
     *  @date 2009-10-05
     */
    template <class TYPE, unsigned int N>
    std::ostream& toStream ( const TYPE(&obj)[N] , std::ostream& s )
    {
      return toStream ( obj , obj + N , s , "( " , " )" , " , " ) ;
    }
    // ========================================================================
    /** the specialization for C-string, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhenf.nl
     *  @date 2009-10-05
     */
    template <unsigned int N>
    std::ostream& toStream (       char (&obj)[N] , std::ostream& s )
    { return toStream ( std::string ( obj , obj+N ) , s ) ; }
    // ========================================================================
    /** the specialization for C-string, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhenf.nl
     *  @date 2009-10-05
     */
    template <unsigned int N>
    std::ostream& toStream ( const char (&obj)[N] , std::ostream& s )
    { return toStream ( std::string ( obj , obj+N ) , s ) ; }
    // ========================================================================
    /** the specialization for C-string, a'la python tuple
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhenf.nl
     *  @date 2009-10-05
     */
    inline std::ostream& toStream ( const char* obj , std::ostream& s )
    { return toStream ( std::string ( obj ) , s ) ; }
    // ========================================================================
    /** the generic implementation of the printout to the std::ostream
     *  @author Alexander MAZUROV Alexander.Mazurov@gmail.com
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-05-12
     */
    template<class TYPE>
    inline std::ostream& toStream ( const TYPE& obj, std::ostream& s )
    { return s << obj ; }
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
    inline std::ostream& toStream
    ( ITERATOR           first ,                       // begin of the sequence
      ITERATOR           last  ,                       //   end of the sequence
      std::ostream&      s     ,                       //            the stream
      const std::string& open  ,                       //               opening
      const std::string& close ,                       //               closing
      const std::string& delim )                       //             delimiter
    {
      s << open ;
      for ( ITERATOR curr = first ; curr != last ; ++curr )
      {
        if ( first != curr ) { s << delim ; }
        toStream ( *curr , s ) ;
      }
      s << close ;
      //
      return s ;
    }
    // ========================================================================
     // helper function to print a tuple of any size
    template<class Tuple, std::size_t N>
    struct TuplePrinter {
        static std::ostream& toStream(const Tuple& t, std::ostream& s) 
        {
            TuplePrinter<Tuple, N-1>::toStream(t, s);
            s << " , ";
            Gaudi::Utils::toStream(std::get<N-1>(t), s);
            return s;
        }
    };
     
    template<class Tuple>
    struct TuplePrinter<Tuple, 1>{
        static std::ostream& toStream(const Tuple& t, std::ostream& s) 
        {
            Gaudi::Utils::toStream(std::get<0>(t), s);
            return s;
        }
    };  

    /** the helper function to print the tuple
     *  @param tulpe (INPUT)  tuple
     *  @return the stream
     *  @author Aleander Mazurov alexander.mazurov@cern.ch
     *  @date 2015-03-21
     */
     template<typename... Args>
     inline std::ostream& toStream(const std::tuple<Args...>& tuple, std::ostream& s) {
        s << " ( ";
        TuplePrinter<decltype(tuple), sizeof...(Args)>::toStream(tuple, s);
        s << " ) ";
        return s;
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
    inline std::string   toString ( const TYPE& obj )
    {
      std::ostringstream s;
      std::ios::fmtflags orig_flags = s.flags();
      s.setf(std::ios::showpoint); // to display correctly floats
      toStream ( obj , s);
      s.flags(orig_flags);
      return s.str();
    }
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif
// ============================================================================


/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Kernel.h>

namespace Gaudi {
  namespace Math {
    /** equality comparison of float numbers using as the metric the maximal
     *  number of Units in the Last Place (ULP).
     *  It is a slightly modified version of very efficient implementation
     *  of the initial Bruce Dawson's algorithm by Chris Lomont.
     *
     *  @see www.lomont.org
     *  @see http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
     *
     *  C.Lomont claims the algorithm is factor 2-10 more efficient
     *  with respect to classical Knuth's algorithm for comparison
     *  of the floating number using the relative precision.
     *
     *  The effective relative difference depends on the choice of
     *   <c>maxULPS</c>:
     *  - For the case of maxULPs=1, (of course it is totally unphysical case!!!)
     *  the effective relative precision r = |a-b|/(|a|+|b|)is
     *  between 3.5e-8 and 5.5e-8 for |a|,|b|>1.e-37, and
     *  then it quickly goes to ~1
     *  - For the case of maxULPS=10
     *  the effective relative precision is
     *  between 3e-8 and 6e-7 for |a|,|b|>1.e-37, and
     *  then it quickly goes to ~1
     *  - For the case of maxULPS=100
     *  the effective relative precision is
     *  around ~6e-6 for |a|,|b|>1.e-37, and
     *  then it quickly goes to ~1
     *  - For the case of maxULPS=1000
     *  the effective relative precision is
     *  around ~6e-5 for |a|,|b|>1.e-37, and
     *  then it quickly goes to ~1
     *
     *  @code
     *
     *  const float a = ... ;
     *  const float b = ... ;
     *
     *  const bool equal = Gaudi::Math::lomont_compare_float ( a , b ) ;
     *
     *  @endcode
     *
     *  @param  af the first number
     *  @param  bf the second number
     *  @param  maxULPs the maximal metric deviation in the terms of
     *                 maximal number of units in the last place
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2008-11-08
     */
    GAUDI_API
    bool lomont_compare_float( const float af, const float bf, const unsigned short maxULPs );
    /** equality comparison of double numbers using as the metric the maximal
     *  number of Units in the Last Place (ULP).
     *  It is a slightly modified version of very efficient implementation
     *  of the initial Bruce Dawson's algorithm by Chris Lomont.
     *
     *  @see www.lomont.org
     *  @see http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
     *
     *  C.Lomont claims the algorithm is factor 2-10 more efficient
     *  with respect to  classical Knuth's algorithm for comparison of
     *  the floating number using the relative precision.
     *
     *  The effective relative difference depends on the choice of
     *   <c>maxULPS</c>:
     *  - For the case of maxULPs=1, (of course it is totally unphysical case!!!)
     *  the effective relative precision r = |a-b|/(|a|+|b|)is
     *  ~6e-16 for |a|,|b|>1.e-304, and
     *  then it quickly goes to ~1
     *
     *  @code
     *
     *  const double a = ... ;
     *  const double b = ... ;
     *
     *  const bool equal = Gaudi::Math::lomont_compare_double ( a , b ) ;
     *
     *  @endcode
     *
     *  @param  af the first number
     *  @param  bf the second number
     *  @param  maxULPs the maximal metric deviation in the terms of
     *                 maximal number of units in the last place
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2008-11-08
     */
    GAUDI_API
    bool lomont_compare_double( const double af, const double bf, const unsigned int maxULPs );

    /** @class Lomont
     *  The equality comparison of double numbers using as the metric the maximal
     *  number of Units in the Last Place (ULP).
     *  It is a slightly modified version of very efficient implementation
     *  of the initial Bruce Dawson's algorithm by Chris Lomont.
     *
     *  @see www.lomont.org
     *  @see http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
     *
     *  C.Lomont claims the algorithm is factor 2-10 more efficient
     *  with respect to classical Knuth's algorithm for comparison of
     *  the floating number using the relative precision.
     *
     *  @attention Only the specializations of this class has sense!
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-22
     */
    template <class TYPE>
    class Lomont;

    /** the specialization for float numbers
     *
     *  @code
     *
     *  Gaudi::Math::Lomont<float> compare ( 100 ) ;
     *
     *  const float a = ... ;
     *  const float b = ... ;
     *
     *  const bool equal = compare ( a , b ) ;
     *
     *  @endcode
     *
     *  @see class Gaudi::Math::Lomont
     *  @attention The default precision is not specified!
     *  @see Gaudi::Math::lomont_compare_float
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-22
     */
    template <>
    class Lomont<float> {
    public:
      /// constructor from ULPS:
      constexpr Lomont( const unsigned short ulps ) : m_ulps( ulps ) {}

      /// the only one important method:
      inline bool operator()( const float a, const float b ) const {
        return Gaudi::Math::lomont_compare_float( a, b, m_ulps );
      }

    private:
      /// the precision in "units in last place"
      unsigned short m_ulps;
    };

    /** the specialization for double numbers
     *  @see class Gaudi::Math::Lomont
     *  @attention The default precision is not specified!
     *  @see Gaudi::Math::lomont_compare_float
     *
     *  @code
     *
     *  Gaudi::Math::Lomont<double> compare ( 500 ) ;
     *
     *  const double a = ... ;
     *  const double b = ... ;
     *
     *  const bool equal = compare ( a , b ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2009-10-22
     */
    template <>
    class Lomont<double> {
    public:
      /// constructor from ULPS:
      constexpr Lomont( const unsigned int ulps ) : m_ulps( ulps ) {}

      /// the only one important method:
      inline bool operator()( const double a, const double b ) const {
        return Gaudi::Math::lomont_compare_double( a, b, m_ulps );
      }

    private:
      /// the precision in "units in last place"
      unsigned int m_ulps;
    };

    /** Get the floating number that representation
     *  is different with respect  to the argument for
     *  the certain number of "Units in the Last Position".
     *  For ulps=1, it is just next float number, for ulps=-1 is is the
     *  previous one.
     *
     *  This routine is very convenient to test the parameter maxULPS for
     *  the routine Gaudi::Math::lomont_compare_float
     *
     *  @see Gaudi::Math::lomont_compare_float
     *  @param af the reference number
     *  @param ulps the bias
     *  @return the biased float number (on distance "ulps")
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2008-11-08
     */
    GAUDI_API
    float next_float( const float af, const short ulps );

    /** Get the floating number that representation
     *  is different with respect  to the argument for
     *  the certain number of "Units in the Last Position".
     *  For ulps=1, it is just next float number, for ulps=-1 is is the
     *  previous one.
     *
     *  This routine is very convenient to test the parameter maxULPS for
     *  the routine Gaudi::Math::lomont_compare_double
     *
     *  @see Gaudi::Math::lomont_compare_double
     *  @param af the reference number
     *  @param ulps the bias
     *  @return the biased float number (on distance "ulps")
     *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
     *  @date 2008-11-08
     */
    GAUDI_API
    double next_double( const double af, const short ulps );

    /** "distance" in ULPS between two float values
     *   @param a (INPUT) the first  number
     *   @param b (INPUT) the second number
     *   @param "distance" in ULPs
     */
    GAUDI_API
    long ulps_distance_float( const float a, const float b );

    /** "distance" in ULPS between two double values
     *   @param a (INPUT) the first  number
     *   @param b (INPUT) the second number
     *   @param "distance" in ULPs
     */
    GAUDI_API
    long ulps_distance_double( const double a, const double b );
  } // namespace Math
} // namespace Gaudi

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

#include <cmath>

/** @file
 *
 *  This file was originally taken from the LoKi project -
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2001-01-23
 */
namespace Gaudi {
  namespace Math {
    /** Simple utility for efficient "pow".
     *  It works only for positive integer powers.
     *
     *  @code
     *
     *   const double result = Gaudi::Math::pow ( value , 10 ) ;
     *
     *  @endcode
     *
     *  The actual code is copied from
     *     std::__cmath_power bits/cmath.tcc
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-04-09
     */
    template <typename TYPE>
    inline TYPE pow( TYPE __x, unsigned long __n ) {
      //
      TYPE __y = __n % 2 ? __x : 1;
      //
      while ( __n >>= 1 ) {
        __x = __x * __x;
        if ( __n % 2 ) { __y = __y * __x; }
      }
      //
      return __y;
    }
  } // namespace Math
} // namespace Gaudi

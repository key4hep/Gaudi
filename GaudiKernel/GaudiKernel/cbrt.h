// $Id: cbrt.h,v 1.1 2007/07/27 12:59:52 marcocle Exp $
#ifndef GAUDIKERNEL_CBRT_H
#define GAUDIKERNEL_CBRT_H

// ============================================================================
// Include files
// ============================================================================

#include <cmath>

/** @file cbrt.h
 *
 *  Provide the function cbrt (gcc specific) to all platforms.
 *
 *  @author Marco Clemencic (form an idea of Chris Jones)
 *  @date   27/07/2007
 *
 */

// double cbrt(double) is a gcc built-in
#ifndef __GNUC__
inline double cbrt( double __x ){
  return ::pow( __x, static_cast<double>(1.0/3.0) );
}
#endif

// we need cbrtf for floats
inline float cbrt( float __x ) {
#ifdef __GNUC__
  return ::cbrtf( __x );
#else
  return ::pow( __x, static_cast<float>(1.0/3.0) );
#endif
}

// we need cbrtl for long doubles
inline long double cbrt( long double __x ) {
#ifdef __GNUC__
  return ::cbrtl( __x );
#else
  return ::pow( __x, static_cast<long double>(1.0/3.0) );
#endif
}

#ifdef __INTEL_COMPILER        // Disable ICC remark
  #pragma warning(push)
  #pragma warning(disable:2259) // non-pointer conversion may lose significant bits
#endif

// use cbrt(double) for integers
#define cbrt_for_int_type(t) \
inline double cbrt( t __x ) { return cbrt ( static_cast<double>(__x) ); }

cbrt_for_int_type(int)
cbrt_for_int_type(long)
cbrt_for_int_type(long long)
cbrt_for_int_type(unsigned int)
cbrt_for_int_type(unsigned long)
cbrt_for_int_type(unsigned long long)

#ifdef __INTEL_COMPILER        // End disable ICC remark
  #pragma warning(pop)
#endif

#undef cbrt_for_int_type

#endif
// ============================================================================

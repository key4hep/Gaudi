// $Id: CheckForNaN.h,v 1.5 2007/10/19 10:18:23 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_CHECKFORNAN_H 
#define GAUDIALG_CHECKFORNAN_H 1
// ============================================================================
// Include files
// ============================================================================
#ifndef _WIN32
#include <cmath>
namespace 
{
  inline int lfin ( double x ) { return  std::isfinite ( x ) ; }
  inline int lnan ( double x ) { return  std::isnan  ( x ) ; }
}
#else
#include <float.h>
namespace 
{
  inline int lfin ( double x ) { return  _finite ( x ) ; }
  inline int lnan ( double x ) { return  _isnan  ( x ) ; }
}
#endif
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIALG_CHECKFORNAN_H
// ============================================================================

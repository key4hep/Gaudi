// $Id: Interpolation.h,v 1.2 2005/11/25 10:27:03 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_INTERPOLATION_H 
#define GAUDIMATH_INTERPOLATION_H 1
// ============================================================================
// incldue files 
// ============================================================================

namespace GaudiMath 
{
  namespace Interpolation
  {
    enum Type
      {
        Linear     = 0   ,  // linear interpolation 
        Polynomial       ,  // polinomial interpolation 
        Cspline          ,  // Cubic spline with natural  boundary conditions 
        Cspline_Periodic ,  // Cubic spline with periodic boundary conditions 
        Akima            ,  // Akima spline with natural  boundary conditions 
        Akima_Periodic      // Akima spline with periodic boundary conditions 
      };
  }; // end of namespace Interpolation
}; //end of namespace GaudiMath

#endif // GAUDIMATH_INTERPOLATION_H
// ============================================================================

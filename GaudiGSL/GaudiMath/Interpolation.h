#ifndef GAUDIMATH_INTERPOLATION_H
#define GAUDIMATH_INTERPOLATION_H 1
// ============================================================================
// include files
// ============================================================================
#include <iosfwd>
#include <string>

class StatusCode;

namespace GaudiMath {
  namespace Interpolation {
    enum Type {
      Linear = 0,       // linear interpolation
      Polynomial,       // polinomial interpolation
      Cspline,          // Cubic spline with natural  boundary conditions
      Cspline_Periodic, // Cubic spline with periodic boundary conditions
      Akima,            // Akima spline with natural  boundary conditions
      Akima_Periodic    // Akima spline with periodic boundary conditions
    };

    StatusCode           parse( Type&, const std::string& );
    std::string          toString( const Type& );
    std::ostream&        toStream( const Type&, std::ostream& );
    inline std::ostream& operator<<( std::ostream& os, const Type& t ) { return toStream( t, os ); }

  } // end of namespace Interpolation
} // end of namespace GaudiMath

#endif // GAUDIMATH_INTERPOLATION_H

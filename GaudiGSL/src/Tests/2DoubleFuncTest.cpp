// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <cmath>
#include <cstdio>
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Adapters.h"
#include "GaudiMath/GaudiMath.h"
// ============================================================================

// ============================================================================
/** @file
 *
 *  Test file for the class Adapter2DoubleFunction
 *
 *  @date 2003-09-03
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 */
// ============================================================================

int main()
{
  const GaudiMath::Function& mypow = GaudiMath::adapter( pow );
  Genfun::Argument           arg( 2 );
  for ( double x = 0; x < 10; ++x ) {
    for ( double y = 0; y < 10; ++y ) {
      arg[0] = x;
      arg[1] = y;
      printf( "x=%6.5f y=%6.5f mypow=%+.10f pow=%+.10f pow-mypow=%+.10f \n", x, y, mypow( arg ), pow( x, y ),
              pow( x, y ) - mypow( arg ) );
    }
  }
}

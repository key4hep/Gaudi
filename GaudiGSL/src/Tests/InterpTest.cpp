// ============================================================================
// Include files
// ============================================================================
// STD& STL
// ============================================================================
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiMath/GaudiMath.h"
#include <cmath>
#include <cstdio>
#include <iostream>

// ============================================================================
/** @file
 *
 *  Test file for the interpolations
 *
 *  @date 2003-08-31
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

int main() {

  const GaudiMath::Function& mysin = GaudiMath::SimpleFun( sin );
  const GaudiMath::Function& mycos = GaudiMath::SimpleFun( cos );

  std::vector<double> vctx;
  std::vector<double> vcty;

  for ( int i = 0; i <= 20; ++i ) {
    double x = i * 3.141592 / 20.0;
    double y = mysin( x );
    vctx.push_back( x );
    vcty.push_back( y );
  }

  using GaudiMath::Interpolation::Cspline_Periodic;

  const GaudiMath::Constant minusOne( -1.0, 1 );
  const GaudiMath::Constant plusOne( 1.0, 1 );

  /// make interpolation of "sin", 10 points from 0 to pi
  const GaudiMath::Spline i1( vctx, vcty, Cspline_Periodic );

  /// the first derivative
  const GaudiMath::SplineDeriv i2( vctx, vcty, Cspline_Periodic );

  /// the second derivative. multiplied by -1
  const GaudiMath::Function& i3 = GaudiMath::SplineDeriv2( vctx, vcty, Cspline_Periodic ) * minusOne;

  /// -1*integral(0.0)
  const GaudiMath::Function& i4 = plusOne + GaudiMath::SplineInteg( vctx, vcty, Cspline_Periodic, 0.0 ) * minusOne;

  for ( int i = 0; i < 30; ++i ) {
    double x = i * 3.141592 / 30.0;
    printf( "x=%+7.1f; Sin-i1=%+14.9f; Cos-i2=%+14.9f; Sin-i3=%+14.9f; Cos-i4=%+14.9f;\n", x / Gaudi::Units::degree,
            mysin( x ) - i1( x ), mycos( x ) - i2( x ), mysin( x ) - i3( x ), mycos( x ) - i4( x ) );
  }
}

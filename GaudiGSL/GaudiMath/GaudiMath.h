#ifndef GAUDIMATH_GAUDIMATH_H
#define GAUDIMATH_GAUDIMATH_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Adapters.h"
#include "GaudiMath/Constant.h"
#include "GaudiMath/NumericalDefiniteIntegral.h"
#include "GaudiMath/NumericalDerivative.h"
#include "GaudiMath/NumericalIndefiniteIntegral.h"
#include "GaudiMath/Splines.h"
// ============================================================================

/** @namespace GaudiMath GaudiMath.h GaudiMath/GaudiMath.h
 *
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2003-08-31
 */
namespace GaudiMath {
  typedef Genfun::AbsFunction Function;

  typedef Genfun::GaudiMathImplementation::Constant                    Constant;
  typedef Genfun::GaudiMathImplementation::NumericalDerivative         Derivative;
  typedef Genfun::GaudiMathImplementation::SimpleFunction              SimpleFun;
  typedef Genfun::GaudiMathImplementation::NumericalIndefiniteIntegral IndIntegral;
  typedef Genfun::GaudiMathImplementation::NumericalDefiniteIntegral   DefIntegral;

  typedef Genfun::GaudiMathImplementation::SplineBase      SimpleSpline;
  typedef Genfun::GaudiMathImplementation::GSLSpline       Spline;
  typedef Genfun::GaudiMathImplementation::GSLSplineDeriv  SplineDeriv;
  typedef Genfun::GaudiMathImplementation::GSLSplineDeriv2 SplineDeriv2;
  typedef Genfun::GaudiMathImplementation::GSLSplineInteg  SplineInteg;

} // namespace GaudiMath

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_GAUDIMATH_H

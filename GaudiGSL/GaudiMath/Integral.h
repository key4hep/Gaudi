// $Id: Integral.h,v 1.2 2004/03/28 18:50:36 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_INTEGRAL_H
#define GAUDIMATH_INTEGRAL_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Integration.h"
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
#include "CLHEP/GenericFunctions/AbsFunctional.hh"
// ============================================================================

namespace GaudiMath
{

  GAUDI_API double Integral
  ( const Genfun::AbsFunction&                 function            ,
    const double                               a                   ,
    const double                               b                   ,
    const GaudiMath::Integration::Type         type     =
    GaudiMath::Integration::Adaptive                               ,
    const GaudiMath::Integration::KronrodRule  rule     =
    GaudiMath::Integration::Default                                ,
    const double                               epsabs   = 1.e-10   ,
    const double                               epsrel   = 1.e-7    ,
    const size_t                               size     = 1000     );

  GAUDI_API double Integral
  ( const Genfun::AbsFunction&                 function            ,
    const double                               a                   ,
    const double                               b                   ,
    const std::vector<double>&                 points              ,
    const double                               epsabs    = 1e-9    ,
    const double                               epsrel    = 1.e-6   ,
    const size_t                               size      = 1000    ) ;

  GAUDI_API double Integral
  ( const Genfun::AbsFunction&                 function            ,
    const double                               a                   ,
    const GaudiMath::Integration::Inf          b         =
    GaudiMath::Integration::Infinity                               ,
    const double                               epsabs    = 1e-9    ,
    const double                               epsrel    = 1.e-6   ,
    const size_t                               size      = 1000    ) ;

  GAUDI_API double Integral
  ( const Genfun::AbsFunction&                 function            ,
    const GaudiMath::Integration::Inf          a                   ,
    const double                               b                   ,
    const double                               epsabs    = 1e-9    ,
    const double                               epsrel    = 1.e-6   ,
    const size_t                               size      = 1000    ) ;

  GAUDI_API double Integral
  ( const Genfun::AbsFunction&                 function            ,
    const GaudiMath::Integration::Inf          a         =
    GaudiMath::Integration::Infinity                               ,
    const GaudiMath::Integration::Inf          b         =
    GaudiMath::Integration::Infinity                               ,
    const double                               epsabs    = 1e-9    ,
    const double                               epsrel    = 1.e-6   ,
    const size_t                               size      = 1000    ) ;

}

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_INTEGRAL_H
// ============================================================================

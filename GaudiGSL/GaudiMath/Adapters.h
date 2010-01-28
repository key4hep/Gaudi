// $Id: Adapters.h,v 1.2 2005/11/25 10:27:03 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_ADAPTERS_H
#define GAUDIMATH_ADAPTERS_H 1
// ============================================================================
// Include files
// ============================================================================
// Local
// ============================================================================
#include "GaudiMath/FunAdapters.h"
#include "GaudiMath/GSLFunAdapters.h"
// ============================================================================
#include "GaudiKernel/Kernel.h"

namespace GaudiMath
{
  typedef Genfun::GaudiMathImplementation::AdapterIFunction
  AIDAFunction   ;
  typedef Genfun::GaudiMathImplementation::Adapter2DoubleFunction
  Function2D     ;
  typedef Genfun::GaudiMathImplementation::Adapter3DoubleFunction
  Function3D     ;
  typedef Genfun::GaudiMathImplementation::SimpleFunction
  SimpleFunction ;

  typedef Genfun::GaudiMathImplementation::GSLFunctionWithMode
  GSLFunctionWithMode ;
  typedef Genfun::GaudiMathImplementation::GSLFunctionWithError
  GSLFunctionWithError ;
  typedef Genfun::GaudiMathImplementation::GSLFunctionWithModeAndError
  GSLFunctionWithModeAndError ;

  GAUDI_API AIDAFunction                adapter
  ( const      AIDA::IFunction&           function ) ;
  GAUDI_API Function2D                  adapter
  (       Function2D::Function            function ) ;
  GAUDI_API Function3D                  adapter
  (       Function3D::Function            function ) ;
  GAUDI_API SimpleFunction              adapter
  (   SimpleFunction::Function1           function ) ;
  GAUDI_API SimpleFunction              adapter
  ( const size_t                          dim      ,
      SimpleFunction::Function2           function ) ;
  GAUDI_API SimpleFunction              adapter
  ( const size_t                          dim      ,
      SimpleFunction::Function3           function ) ;
  GAUDI_API GSLFunctionWithMode         adapter
  ( GSLFunctionWithMode::Function         function ,
    const gsl_mode_t&                     mode     ) ;
  GAUDI_API GSLFunctionWithError        adapter
  ( GSLFunctionWithError::Function        function ) ;
  GAUDI_API GSLFunctionWithModeAndError adapter
  ( GSLFunctionWithModeAndError::Function function ,
    const gsl_mode_t&                     mode     ) ;

} // end of namespace GaudiMath

#endif // GAUDIMATH_ADAPTERS_H
// ============================================================================

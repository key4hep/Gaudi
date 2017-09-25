#ifndef GAUDIMATH_CONSTANT_H
#define GAUDIMATH_CONSTANT_H 1
// ============================================================================
// Include files
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================
#include "GaudiKernel/Kernel.h"

#if defined( __clang__ ) || defined( __CLING__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined( __GNUC__ ) && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

namespace Genfun // due to CLHEP
{
  namespace GaudiMathImplementation
  {
    /** @class Constant Constant.h GaudiMath/Constant.h
     *
     *  Fixed constant function
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date   2003-08-31
     */
    class GAUDI_API Constant : public AbsFunction
    {
    public:
      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( Constant )

    public:
      /** Standard constructor
       *  @param value of the function
       *  @param dim   dimensionality
       */
      Constant( const double value, const size_t dim );

      /// dimensionality of the problem
      unsigned int dimensionality() const override { return m_DIM; }
      /// Does this function have an analytic derivative?
      bool hasAnalyticDerivative() const override { return true; }
      /// Function value
      double operator()( double ) const override { return m_value; }
      /// Function value
      double operator()( const Argument& ) const override { return m_value; }
      /// Derivatives
      Derivative partial( unsigned int i ) const override;

      // the assignement operator is disabled
      Constant& operator=( const Constant& ) = delete;

    private:
      double m_value;
      size_t m_DIM;
    };

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun

#if defined( __clang__ ) || defined( __CLING__ )
#pragma clang diagnostic pop
#elif defined( __GNUC__ ) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_CONSTANT_H
// ============================================================================

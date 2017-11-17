#ifndef GAUDIMATH_GSLFUNADAPTERS_H
#define GAUDIMATH_GSLFUNADAPTERS_H 1
// ============================================================================
// Include files
// ============================================================================
#include <memory>
// ============================================================================
// from CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
// ============================================================================
// GaudiGSL/GaudiMath
// ============================================================================
#include "GaudiMath/FunAdapters.h"
// ============================================================================
// GSL
// ============================================================================
#include "gsl/gsl_mode.h"
#include "gsl/gsl_sf_result.h"
// ============================================================================
#include "GaudiKernel/Kernel.h"

#if defined( __clang__ ) || defined( __CLING__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined( __GNUC__ ) && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /// the actual type for primitive adaptor
    typedef Genfun::GaudiMathImplementation::SimpleFunction GSLFunction;

    class GAUDI_API GSLFunctionWithError : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef int ( *Function )( double, gsl_sf_result* );

    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithError )
    public:
      /// constructor with pointer to function
      GSLFunctionWithError( Function function );
      /// copy constructor
      GSLFunctionWithError( const GSLFunctionWithError& );
      /// the main method
      double operator()( double x ) const override;
      double operator()( const Argument& x ) const override;
      bool hasAnalyticDerivative() const override { return true; }
      unsigned int dimensionality() const override { return 1; }
      Genfun::Derivative partial( unsigned int i ) const override;

      /// get the function itself
      Function function() const;
      /// get the last evaluated result
      const gsl_sf_result& result() const;
      /// get the error in the last estimate of the result
      double error() const;

    private:
      // default constructor is disabled
      GSLFunctionWithError();
      // assignement operator is disabled
      GSLFunctionWithError& operator=( const GSLFunctionWithError& );

    private:
      Function m_function;
      std::unique_ptr<gsl_sf_result> m_result;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( GSLFunctionWithError )

    class GAUDI_API GSLFunctionWithMode : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef double ( *Function )( double, gsl_mode_t );

    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithMode )
    public:
      /// constructor with pointer to function
      GSLFunctionWithMode( Function fun, const gsl_mode_t& mode );
      /// copy constructor
      GSLFunctionWithMode( const GSLFunctionWithMode& );
      /// the main method
      double operator()( double x ) const override;
      double operator()( const Argument& x ) const override;
      bool hasAnalyticDerivative() const override { return true; }
      unsigned int dimensionality() const override { return 1; }
      Genfun::Derivative partial( unsigned int i ) const override;
      /// get the function itself
      Function function() const;
      /// get the current mode
      const gsl_mode_t& mode() const;

    private:
      // default constructor is disabled
      GSLFunctionWithMode();
      // assignement operator is disabled
      GSLFunctionWithMode& operator=( const GSLFunctionWithError& );

    private:
      Function m_function;
      std::unique_ptr<gsl_mode_t> m_mode;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( GSLFunctionWithMode )

    class GAUDI_API GSLFunctionWithModeAndError : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef int ( *Function )( double, gsl_mode_t, gsl_sf_result* );

    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithModeAndError )
    public:
      /// constructor with pointer to function
      GSLFunctionWithModeAndError( Function fun, const gsl_mode_t& mode );
      /// copy constructor
      GSLFunctionWithModeAndError( const GSLFunctionWithModeAndError& );
      /// the main method
      double operator()( double x ) const override;
      double operator()( const Argument& x ) const override;
      bool hasAnalyticDerivative() const override { return true; }
      unsigned int dimensionality() const override { return 1; }
      Genfun::Derivative partial( unsigned int i ) const override;
      /// get the function itself
      Function function() const;
      /// get the current mode
      const gsl_mode_t& mode() const;
      /// get the last evaluated result
      const gsl_sf_result& result() const;
      /// get the error in the last estimate of the result
      double error() const;

    private:
      // default constructor is disabled
      GSLFunctionWithModeAndError();
      // assignement operator is disabled
      GSLFunctionWithModeAndError& operator=( const GSLFunctionWithModeAndError& );

    private:
      Function m_function;
      std::unique_ptr<gsl_mode_t> m_mode;
      std::unique_ptr<gsl_sf_result> m_result;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( GSLFunctionWithModeAndError )

  } // end of namespace GaudiMath Implemnentation
} // end of namespace Genfun

#if defined( __clang__ ) || defined( __CLING__ )
#pragma clang diagnostic pop
#elif defined( __GNUC__ ) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

#endif // GAUDIMATH_GSLFUNADAPTERS_H
// ============================================================================

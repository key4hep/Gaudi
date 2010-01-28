// $Id: GSLFunAdapters.h,v 1.2 2005/11/25 10:27:03 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_GSLFUNADAPTERS_H
#define GAUDIMATH_GSLFUNADAPTERS_H 1
// ============================================================================
// Include files
// ============================================================================
// from CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
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

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /// the actual type for primitive adaptor
    typedef Genfun::GaudiMathImplementation::SimpleFunction GSLFunction ;

    class GAUDI_API GSLFunctionWithError : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef int (*Function)( double , gsl_sf_result* ) ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithError )
    public:
      /// constructor with pointer to function
      GSLFunctionWithError ( Function function ) ;
      /// copy constructor
      GSLFunctionWithError ( const GSLFunctionWithError& ) ;
      /// destructor
      virtual ~GSLFunctionWithError () ;
      /// the main method
      virtual double operator() (       double    x ) const ;
      virtual double operator() ( const Argument& x ) const ;
      virtual bool         hasAnalyticDerivative () const { return true ; }
      virtual unsigned int dimensionality        () const { return    1 ; }
      Genfun::Derivative partial ( unsigned int i ) const ;

      /// get the function itself
      Function             function () const ;
      /// get the last evaluated result
      const gsl_sf_result& result   () const ;
      /// get the error in the last estimate of the result
      double               error    () const ;
    private:
      // default constructor is disabled
      GSLFunctionWithError () ;
      // assignement operator is disabled
      GSLFunctionWithError& operator=( const GSLFunctionWithError & ) ;
    private:
      Function        m_function ;
      gsl_sf_result*  m_result   ;
    };

    class GAUDI_API GSLFunctionWithMode : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef double (*Function)( double , gsl_mode_t ) ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithMode )
    public:
      /// constructor with pointer to function
      GSLFunctionWithMode ( Function          fun  ,
                            const gsl_mode_t& mode ) ;
      /// copy constructor
      GSLFunctionWithMode ( const GSLFunctionWithMode& ) ;
      /// destructor
      virtual ~GSLFunctionWithMode () ;
      /// the main method
      virtual double operator() (       double    x ) const ;
      virtual double operator() ( const Argument& x ) const ;
      virtual bool         hasAnalyticDerivative () const { return true ; }
      virtual unsigned int dimensionality        () const { return    1 ; }
      Genfun::Derivative partial ( unsigned int i ) const ;
      /// get the function itself
      Function             function () const ;
      /// get the current mode
      const gsl_mode_t&    mode     () const ;
    private:
      // default constructor is disabled
      GSLFunctionWithMode () ;
      // assignement operator is disabled
      GSLFunctionWithMode& operator=( const GSLFunctionWithError & ) ;
    private:
      Function        m_function ;
      gsl_mode_t*     m_mode     ;
    };

    class GAUDI_API GSLFunctionWithModeAndError : public AbsFunction
    {
    public:
      /// the actual type of the function
      typedef int (*Function)( double          ,
                               gsl_mode_t      ,
                               gsl_sf_result*  ) ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLFunctionWithModeAndError )
    public:
      /// constructor with pointer to function
      GSLFunctionWithModeAndError ( Function          fun  ,
                                    const gsl_mode_t& mode ) ;
      /// copy constructor
      GSLFunctionWithModeAndError ( const GSLFunctionWithModeAndError& ) ;
      /// destructor
      virtual ~GSLFunctionWithModeAndError () ;
      /// the main method
      virtual double operator() (       double    x ) const ;
      virtual double operator() ( const Argument& x ) const ;
      virtual bool         hasAnalyticDerivative () const { return true ; }
      virtual unsigned int dimensionality        () const { return    1 ; }
      Genfun::Derivative partial ( unsigned int i ) const ;
      /// get the function itself
      Function             function () const ;
      /// get the current mode
      const gsl_mode_t&    mode     () const ;
      /// get the last evaluated result
      const gsl_sf_result& result   () const ;
      /// get the error in the last estimate of the result
      double               error    () const ;
    private:
      // default constructor is disabled
      GSLFunctionWithModeAndError () ;
      // assignement operator is disabled
      GSLFunctionWithModeAndError& operator=
      ( const GSLFunctionWithModeAndError & ) ;
    private:
      Function        m_function ;
      gsl_mode_t*     m_mode     ;
      gsl_sf_result*  m_result   ;
    };

  } // end of namespace GaudiMath Implemnentation
} // end of namespace Genfun

#endif // GAUDIMATH_GSLFUNADAPTERS_H
// ============================================================================

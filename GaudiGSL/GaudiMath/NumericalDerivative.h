// $Id: NumericalDerivative.h,v 1.2 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
#ifndef GAUDIMATH_NUMERICALDERIVATIVE_H
#define GAUDIMATH_NUMERICALDERIVATIVE_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /** @class  NumericalDerivative GaudiMath/NumericalDerivative.h
     *
     *  Numerical derivative  (using GSL adaptive numerical differentiation)
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
     =
     \frac{\partial}{\partial x_i} f
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1},  \dots , x_n \right)
     \f]
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2003-08-27
     */
    class GAUDI_API NumericalDerivative : public AbsFunction
    {
    public:
      /// the type of numerical differentiation
      enum Type
        { Central  ,
          Forward  ,
          Backward } ;
    public:
      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( NumericalDerivative )

    public:

      /** The standard constructor from
       *  AbsFunction and the index of the variable
       *
       *  The function, created with this constructor compute the
       *  numerical derivatived by finite differencing,.
       *  An adaptive algorithm is used  to find teh best
       *  choice  of finite difference and to estimate the error
       *  in derivative
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \frac{\partial}{\partial x_i} f
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1},  \dots , x_n \right)
       \f]
       *
       * - The central difference algorithm <tt>gsl_diff_central</tt>
       * is used for <tt>type</tt> = <tt>Type::Central</tt>
       * - The forward difference algorithm <tt>gsl_diff_forward</tt>
       * is used for <tt>type</tt> = <tt>Type::Forward</tt>. The function
       * is evaluated only at points greater than \c x_i and \c x_i itself.
       * This function should be used  if f(x) has a singularity or
       * if it is undefined for values less than \c x_i
       * - The backward difference algorithm <tt>gsl_diff_backward</tt>
       * is used for <tt>type</tt> = <tt>Type::Backward</tt>. The function
       * is evaluated only at points smaller than \c x_i and \c x_i itself.
       * This function should be used  if f(x) has a singularity or
       * if it is undefined for values greater than \c x_i
       *
       *
       * The adaptime numerical differentiation is used
       *  @param function the function
       *  @param index    the variable index
       */
      NumericalDerivative
      ( const AbsFunction&   function          ,
        const size_t         index             ,
        const Type&          type    = Central ) ;

      /// copy constructor
      NumericalDerivative
      ( const NumericalDerivative&   right    ) ;

      /// virtual destructor
      virtual ~NumericalDerivative () ;

      /// dimensionality of the problem
      virtual unsigned int dimensionality() const { return m_DIM ; }

      /// Function value
      virtual double operator() ( double          argument ) const ;
      /// Function value
      virtual double operator() ( const Argument& argument ) const ;

      /// Does this function have an analytic derivative?
      virtual bool hasAnalyticDerivative() const { return true ;}

      /// Derivatives
      virtual Derivative partial ( unsigned int index ) const;

    public:

      /// access to the function itself
      const AbsFunction&  function () const { return *m_function ; }
      /// index
      size_t              index    () const { return  m_index    ; }
      /// type
      const Type&         type     () const { return  m_type     ; }

      /// change the type of the adaptive differentiation
      const Type&         setType  ( const Type& value );

      /// the result of the last call
      double              result   () const { return m_result    ; }
      /// the absolute error estimate for the last call
      double              error    () const { return m_error     ; }

    protected:

      StatusCode Exception
      ( const std::string& message                       ,
        const StatusCode&  sc      = StatusCode::FAILURE ) const ;

    private:

      // default constructor  is disabled
      NumericalDerivative ();
      // assignement operator is disabled
      NumericalDerivative& operator=( const NumericalDerivative& ) ;

    private:
      const AbsFunction* m_function  ;
      size_t             m_index     ;
      size_t             m_DIM       ;
      Type               m_type      ;
      //
      mutable Argument   m_argument  ;
      mutable double     m_result    ;
      mutable double     m_error     ;
    };

  } // end of namespace GaudiMathImplementation
} // end of namespace GenFun

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_NUMERICALDERIVATIVE_H
// ============================================================================

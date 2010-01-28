// $Id: NumericalIndefiniteIntegral.h,v 1.2 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
#ifndef GAUDIMATH_NUMERICALINDEFINITEINTEGRAL_H
#define GAUDIMATH_NUMERICALINDEFINITEINTEGRAL_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Integration.h"
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================


/// forward declaration

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /** @class NumericalIndefiniteIntegral NumericalIndefiniteIntegral.h
     *
     *  The simple class for numerical integrations.
     *  It allows to evaluate following indefinite integrals:
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{a}^{x_i} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{x_i}^{a} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{-\infty}^{x_i} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{x_i}^{+\infty} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2003-08-31
     */
    class GAUDI_API NumericalIndefiniteIntegral : public AbsFunction
    {
    public:
      struct _Workspace ;
      struct _Function  ;
    public:

      /// typedef for vector of singular points
      typedef std::vector<double>   Points ;

    public:

      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( NumericalIndefiniteIntegral )

    public:

      /** Standard constructor
       *  The function, created with this constructor
       *  evaluates following indefinite integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{a}^{x_i} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       * for value of \c limit = \c VariableHighLimit
       * and the integral
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{x_i}^{a} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  for value of \c limit = \c VariableLowLimit
       *
       *  If function contains singularities, the
       *  \c type = <tt>Type::AdaptiveSingular</tt> need to be used
       *
       *  For faster integration of smooth function
       *  non-adaptive integration can be used:
       *  \c type = <tt>Type::NonAdaptive</tt> need to be used
       *
       *  For adaptive integration one can specify the
       *  order of Gauss-Kronrad integration rule
       *  \c rule = <tt>KronrodRule::Gauss15</tt>
       *  The higher-order rules give better accuracy
       *  for smooth functions, while lower-order rules save
       *  the time when the function contains local difficulties,
       *  such as discontinuites.
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qng</tt> is used for
       *   \c type = <tt>Type:NonAdaptive</tt> :
       *  - The GSL routine
       *  <tt>gsl_integration_qag</tt> is used for
       *   \c type = <tt>Type:Adaptive</tt> :
       *  - The GSL routine
       *  <tt>gsl_integration_qags</tt> is used for
       *   \c type = <tt>Type:AdaptiveSingular</tt> :
       *
       *  @param function the base function
       *  @param index    the variable index
       *  @param a        integration limit
       *  @param limit flag to distinguisch low variable limit
       *  from high variable limit
       *  @param type     the integration type (adaptive,
       *                  non-adaptive or adaptive for singular functions
       *  @param key      Gauss-Kronrad integration rule
       *  @param epsabs   absolute precision for integration
       *  @param epsrel   relative precision for integration
       *  @param lim      bisection limit
       */
      NumericalIndefiniteIntegral
      ( const AbsFunction&                         function                   ,
        const size_t                               index                      ,
        const double                               a                          ,
        const GaudiMath::Integration::Limit        limit  =
        GaudiMath::Integration::VariableHighLimit ,
        const GaudiMath::Integration::Type         type   =
        GaudiMath::Integration::Adaptive          ,
        const GaudiMath::Integration::KronrodRule  rule   =
        GaudiMath::Integration::Default           ,
        const double                               epsabs = 1e-10             ,
        const double                               epsrel = 1.e-7             ,
        const size_t                               size   = 1000              );

      /** standard constructor
       *
       *  The function, created with this constructor
       *  evaluates following indefinite integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{a}^{x_i} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       * for value of \c limit = \c VariableHighLimit
       * and the integral
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{x_i}^{a} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  for value of \c limit = \c VariableLowLimit
       *
       *  The integrand is assumed to have a \b known discontinuities
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qagp</tt> is used for integration

       *  @param function the base function
       *  @param index    the variable index
       *  @param a        integration limit
       *  @param limit flag to distinguisch low variable limit
       *               from high variable limit
       *  @param points   list of known function singularities
       *  @param epsabs   absolute precision for integration
       *  @param epsrel   relative precision for integration
       */
      NumericalIndefiniteIntegral
      ( const AbsFunction&                  function                      ,
        const size_t                        index                         ,
        const double                        a                             ,
        const Points&                       points                        ,
        const GaudiMath::Integration::Limit limit     =
        GaudiMath::Integration::VariableHighLimit ,
        const double                        epsabs    = 1e-9              ,
        const double                        epsrel    = 1.e-6             ,
        const size_t                        size      = 1000              ) ;

      /** standard constructor
       *  The function, created with this constructor
       *  evaluates following indefinite integral:
       *
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{-\infty}^{x_i} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       * for value of \c limit = \c VariableHighLimit
       * and the integral
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_i , x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{x_i}^{+\infty} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  for value of \c limit = \c VariableLowLimit
       *
       *  - The GSL routines
       *  <tt>gsl_integration_qagil</tt> and
       *  <tt>gsl_integration_qagiu</tt> are used for adapive integration
       *
       *  @param function the base function
       *  @param index    the variable index
       *  @param limit flag to distinguisch low variable limit
       *               from high variable limit
       *  @param singularities list of known function singularities
       */
      NumericalIndefiniteIntegral
      ( const AbsFunction&                  function                      ,
        const size_t                        index                         ,
        const GaudiMath::Integration::Limit limit     =
        GaudiMath::Integration::VariableHighLimit ,
        const double                        epsabs    = 1e-9              ,
        const double                        epsrel    = 1.e-6             ,
        const size_t                        size      = 1000              ) ;

      /// copy constructor
      NumericalIndefiniteIntegral
      ( const NumericalIndefiniteIntegral& ) ;

      /// destructor
      virtual ~NumericalIndefiniteIntegral() ;

    public:

      /// dimensionality of the problem
      virtual unsigned int dimensionality() const { return m_DIM ; }

      /// Function value
      virtual double operator() ( double          argument ) const ;
      /// Function value
      virtual double operator() ( const Argument& argument ) const ;

      /// Does this function have an analytic derivative?
      virtual bool hasAnalyticDerivative() const { return true ;}

      /// Derivatives
      virtual Genfun::Derivative partial ( unsigned int index ) const;

    public:

      /// accessor to the function itself
      const AbsFunction& function () const { return *m_function ; }
      /// integration limit
      double             a        () const { return         m_a ; }
      /// known singularities
      const Points&      points   () const { return    m_points ; }
      /// absolute precision
      double             epsabs   () const { return    m_epsabs ; }
      /// relatiove precision
      double             epsrel   () const { return    m_epsrel ; }

      /// previous result
      double             result   () const { return    m_result ; }
      /// evaluate of previous error
      double             error    () const { return     m_error ; }

      // maximal number of bisection integvals for adaptive algorithms
      size_t             size     () const { return      m_size ; }

      /// integration limit
      GaudiMath::Integration::Limit
      limit    () const { return     m_limit ; }
      /// integration type
      GaudiMath::Integration::Type
      type     () const { return      m_type ; }
      /// integration category
      GaudiMath::Integration::Category
      category () const { return  m_category ; }
      /// integration rule
      GaudiMath::Integration::KronrodRule
      rule     () const { return      m_rule ; }

    protected:

      // adaptive integration on infinite intervals
      double                     QAGI ( _Function* fun ) const ;
      // adaptive integration  with known singular points
      double                     QAGP ( _Function* fun ) const ;
      // non-adaptive integration
      double                     QNG  ( _Function* fun ) const ;
      // adaptive integration
      double                     QAG  ( _Function* fun ) const ;
      // adaptive integral with singularities
      double                     QAGS ( _Function* fun ) const ;

      // allocate the integration workspace
      _Workspace*                allocate                () const ;
      // the integration workspace
      _Workspace*                ws                      () const
      { return m_ws ; };

      // throw the exception
      StatusCode Exception
      ( const std::string& message ,
        const StatusCode&  sc = StatusCode::FAILURE ) const ;

   private:

      // default constructor is disabled
      NumericalIndefiniteIntegral() ;
      // assignement operator is disabled
      NumericalIndefiniteIntegral& operator=
      ( const NumericalIndefiniteIntegral& ) ;

    private:

      const AbsFunction*                  m_function ;
      size_t                              m_DIM      ;
      size_t                              m_index    ;

      double                              m_a        ;

      GaudiMath::Integration::Limit       m_limit    ;
      GaudiMath::Integration::Type        m_type     ;
      GaudiMath::Integration::Category    m_category ;
      GaudiMath::Integration::KronrodRule m_rule     ;

      Points                              m_points   ;
      double*                             m_pdata    ;

      double                              m_epsabs   ;
      double                              m_epsrel   ;

      mutable double                      m_result   ;
      mutable double                      m_error    ;

      size_t                              m_size     ;
      mutable _Workspace*                 m_ws       ;

      mutable  Argument                   m_argument ;

    };

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_NUMERICALINDEFINITEINTEGRAL_H
// ============================================================================

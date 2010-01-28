// $Id: NumericalDefiniteIntegral.h,v 1.3 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
#ifndef GAUDIMATH_NUMERICALDEFINITEINTEGRAL_H
#define GAUDIMATH_NUMERICALDEFINITEINTEGRAL_H 1
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

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /** @class NumericalDefiniteIntegral GaudiMath/NumericalDefiniteIntegral.h
     *
     *  This class allows the numerical evaluation of the following functions:
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{a}^{b} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{a}^{+\infty} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{-\infty}^{b} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     \f[
     {\mathcal{F}}_i
     \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
     =
     \int\limits_{-\infty}^{+\infty} f
     \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
     \, {\mathrm{d}} \hat{x}_i
     \f]
     *
     *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2003-08-31
     */
    class GAUDI_API NumericalDefiniteIntegral : public AbsFunction
    {
    public:
      struct _Workspace ;
      struct _Function  ;
    public:

      /// typedef for vector of singular points
      typedef std::vector<double>   Points ;

    public:

      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( NumericalDefiniteIntegral )

    public:

      /** Standard constructor
       *  The function created with this constructor compute
       *  the following integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{a}^{b} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *
       *  If function contains singularities, the
       *  \c type = <tt>Type::AdaptiveSingular</tt> need to be used
       *
       *  For faster integration of smooth function
       *  non-adaptive integration can be used:
       *  \c type = <tt>Type::NonAdaptive</tt> need to be used
       *
       *  For adaptive integration
       *  \c type = <tt>Type>>Adaptive</tt> one can specify the
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
       *
       *  @param function funtion to be integrated
       *  @param index    variable index
       *  @param a        lower intgeration limit
       *  @param b        high  integration limit
       *  @param type     integration type
       *  @param rule     integration rule (for adaptive integration)
       *  @param epsabs   required absolute precision
       *  @param epsrel   required relative precision
       *  @param size     maximal number of bisections for adaptive integration
       */
      NumericalDefiniteIntegral
      ( const AbsFunction&                        function            ,
        const size_t                              index               ,
        const double                              a                   ,
        const double                              b                   ,
        const GaudiMath::Integration::Type        type     =
        GaudiMath::Integration::Adaptive                              ,
        const GaudiMath::Integration::KronrodRule rule     =
        GaudiMath::Integration::Default                               ,
        const double                              epsabs   = 1.e-10   ,
        const double                              epsrel   = 1.e-7    ,
        const size_t                              size     = 1000     );


      /** Standard constructor
       *  The function created with this constructor compute
       *  the following integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{a}^{b} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       * where function have known singulatities
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qagp</tt> is used for integration
       *
       *
       *  @param function funtion to be integrated
       *  @param index    variable index
       *  @param a        lower intgeration limit
       *  @param b        high  integration limit
       *  @param pointns  vector of know singular points
       *  @param epsabs   required absolute precision
       *  @param epsrel   required relative precision
       *  @param size     maximal number of bisections for adaptive integration
       */
      NumericalDefiniteIntegral
      ( const AbsFunction&                  function           ,
        const size_t                        index              ,
        const double                        a                  ,
        const double                        b                  ,
        const Points&                       points             ,
        const double                        epsabs    = 1e-9   ,
        const double                        epsrel    = 1.e-6  ,
        const size_t                        size      = 1000   ) ;


      /** Standard constructor
       *  The function created with this constructor compute
       *  the following integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{a}^{+\infty} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qagiu</tt> is used for integration
       *
       *
       *  @param function funtion to be integrated
       *  @param index    variable index
       *  @param a        lower intgeration limit
       *  @param b        indication for infinity
       *  @param epsabs   required absolute precision
       *  @param epsrel   required relative precision
       *  @param size     maximal number of bisections for adaptive integration
       */
      NumericalDefiniteIntegral
      ( const AbsFunction&                      function           ,
        const size_t                            index              ,
        const double                            a                  ,
        const GaudiMath::Integration::Inf       b         =
        GaudiMath::Integration::Infinity                           ,
        const double                            epsabs    = 1e-9   ,
        const double                            epsrel    = 1.e-6  ,
        const size_t                            size      = 1000   ) ;

      /** Standard constructor
       *  The function created with this constructor compute
       *  the following integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{-\infty}^{b} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qagiu</tt> is used for integration
       *
       *
       *  @param function funtion to be integrated
       *  @param index    variable index
       *  @param a        lower intgeration limit
       *  @param b        indication for infinity
       *  @param epsabs   required absolute precision
       *  @param epsrel   required relative precision
       *  @param size     maximal number of bisections for adaptive integration
       */
      NumericalDefiniteIntegral
      ( const AbsFunction&                      function           ,
        const size_t                            index              ,
        const GaudiMath::Integration::Inf       a                  ,
        const double                            b                  ,
        const double                            epsabs    = 1e-9   ,
        const double                            epsrel    = 1.e-6  ,
        const size_t                            size      = 1000   ) ;

      /** Standard constructor
       *  The function created with this constructor compute
       *  the following integral:
       *
       \f[
       {\mathcal{F}}_i
       \left(x_1, \dots , x_{i-1}, x_{i+1}, \dots , x_n \right)
       =
       \int\limits_{-\infty}^{+\infty} f
       \left(x_1, \dots , x_{i-1}, \hat{x}_i , x_{i+1},  \dots , x_n \right)
       \, {\mathrm{d}} \hat{x}_i
       \f]
       *
       *  - The GSL routine
       *  <tt>gsl_integration_qagi</tt> is used for integration
       *
       *
       *  @param function funtion to be integrated
       *  @param index    variable index
       *  @param epsabs   required absolute precision
       *  @param epsrel   required relative precision
       *  @param size     maximal number of bisections for adaptive integration
       */
      NumericalDefiniteIntegral
      ( const AbsFunction&                      function           ,
        const size_t                            index              ,
        // FIXME: The next two arguments should be "double" but are "float" to resolve the
        // ambiguity with the first constructor when providing
        // '(const Genfun::AbsFunction, const unsigned int, const double, const double)'
        const float                             epsabs    = 1e-9   ,
        const float                             epsrel    = 1.e-6  ,
        const size_t                            size      = 1000   ) ;

      /// copy constructor
      NumericalDefiniteIntegral ( const NumericalDefiniteIntegral& ) ;

      /// destructor
      virtual ~NumericalDefiniteIntegral() ;

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
      double             b        () const { return         m_b ; }
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

      // defautl constructor is disabled
      NumericalDefiniteIntegral();
      // assignement is disabled
      NumericalDefiniteIntegral& operator=( const NumericalDefiniteIntegral& );

    private:

      const AbsFunction*                  m_function ;
      size_t                              m_DIM      ;
      size_t                              m_index    ;

      double                              m_a        ;
      double                              m_b        ;
      bool                                m_ia       ;
      bool                                m_ib       ;

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
      mutable  Argument                   m_argF     ;

    };

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_NUMERICALDEFINITEINTEGRAL_H
// ============================================================================

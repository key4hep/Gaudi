// ============================================================================
// Include files
// ============================================================================
// GSL
// ============================================================================
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Splines.h"
#include "GaudiMath/GaudiMath.h"
// ============================================================================
#include <cstring>


/** @file
 *  Implementation file for class GSLSplines
 *  @date 2004-03-06
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    /** constructor from vectors and type
     *  @param x    vector of x
     *  @param y    vector of y(x)
     *  @param type interpolation type
     */
    SplineBase::SplineBase
    ( const SplineBase::Data1D&            x    ,
      const SplineBase::Data1D&            y    ,
      const GaudiMath::Interpolation::Type type )
      : m_x         ( x )
      , m_y         ( y )
      , m_type      ( type )
    {

    }
    // ========================================================================

    // ========================================================================
    /** constructor from vector of (x,y(x)) pairs
     *  @param data vector of (x,y(x)) pairs
     *  @param type interpolaiton type
     */
    // ========================================================================
    SplineBase::SplineBase
    ( const SplineBase::Data2D&            data ,
      const GaudiMath::Interpolation::Type type )
      : m_type      ( type )
    {
      m_x.reserve(data.size());
      m_y.reserve(data.size());
      for( const auto& i : data ) {
        m_x.push_back( i.first  );
        m_y.push_back( i.second );
      }
    }
    // ========================================================================


    // ========================================================================
    void SplineBase::initialize() const
    {
      if ( m_spline ) { return ; }                                 // RETURN

      const gsl_interp_type* T = nullptr ;

      switch ( m_type )
      {
      case GaudiMath::Interpolation::Linear           :
        T = gsl_interp_linear            ; break ;
      case GaudiMath::Interpolation::Polynomial       :
        T = gsl_interp_polynomial        ; break ;
      case GaudiMath::Interpolation::Cspline          :
        T = gsl_interp_cspline           ; break ;
      case GaudiMath::Interpolation::Cspline_Periodic :
        T = gsl_interp_cspline_periodic  ; break ;
      case GaudiMath::Interpolation::Akima            :
        T = gsl_interp_akima             ; break ;
      case GaudiMath::Interpolation::Akima_Periodic   :
        T = gsl_interp_akima_periodic    ; break ;
      default :
        T = gsl_interp_cspline           ; break ;
      };

      m_spline.reset( gsl_spline_alloc( T , m_x.size() ) );
      gsl_spline_init( m_spline.get() , m_x.data() , m_y.data() , m_x.size() ) ;
      m_accel.reset( gsl_interp_accel_alloc() );

      if (!m_accel) m_spline.reset();


    }
    // ========================================================================

    // ========================================================================
    double SplineBase::eval  ( const double x ) const
    {
      if ( !m_spline ) { initialize() ; }
      return gsl_spline_eval ( m_spline.get() , x , m_accel.get() );
    }
    // ========================================================================

    // ========================================================================
    double SplineBase::deriv  ( const double x ) const
    {
      if ( !m_spline ) { initialize() ; }
      return gsl_spline_eval_deriv  ( m_spline.get() , x , m_accel.get() );
    }
    // ========================================================================

    // ========================================================================
    double SplineBase::deriv2 ( const double x ) const
    {
      if ( !m_spline ) { initialize() ; }
      return gsl_spline_eval_deriv2 ( m_spline.get() , x , m_accel.get() );
    }
    // ========================================================================

    // ========================================================================
    double SplineBase::integ  ( const double a ,
                                const double b ) const
    {
      if ( !m_spline ) { initialize() ; }
      return gsl_spline_eval_integ ( m_spline.get() , a , b , m_accel.get() ) ;
    }
    // ========================================================================

    // ========================================================================
    /** constructor from vectors and type
     *
     *  @code
     *
     *   Data1D x = .. ;
     *   Data1D y = .. ;
     *
     *   const AbsFunction& func =
     *        GSLSpline( x , y , GaudiMath::Interpolation::Cspline ) ;
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param x    vector of x
     *  @param y    vector of y
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSpline::GSLSpline
    ( const GSLSpline::Data1D&             x    ,
      const GSLSpline::Data1D&             y    ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline ( x , y , type )
    {}
    // ========================================================================

    // ========================================================================
    /** constructor from data vector
     *
     *  @code
     *
     *   Data2D data = ... ;
     *
     *   const AbsFunction& func =
     *        GSLSpline( data , GaudiMath::Interpolation::Cspline );
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param data vector of (x,y) pairs
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSpline::GSLSpline
    ( const GSLSpline::Data2D&             data ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline    ( data , type )
    {}
    // ========================================================================

    // ========================================================================
    /// constructor from SplineBase
    // ========================================================================
    GSLSpline::GSLSpline
    ( const SplineBase& right )
      : AbsFunction ()
      , m_spline    ( right )
    {}
    // ========================================================================

    // ========================================================================
    double GSLSpline::operator() (       double    x ) const
    { return m_spline.eval( x    ) ; }
    // ========================================================================
    double GSLSpline::operator() ( const Argument& x ) const
    { return m_spline.eval( x[0] ) ; }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative GSLSpline::partial( unsigned int i  ) const
    {
      if ( i >= 1 )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0.0 , 1 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      }
      const AbsFunction& aux = GSLSplineDeriv( *this ) ;
      return Genfun::FunctionNoop( &aux ) ;
    }
    // ========================================================================

    // ========================================================================
    /** constructor from vectors and type
     *
     *  @code
     *
     *   Data1D x = .. ;
     *   Data1D y = .. ;
     *
     *   const AbsFunction& func =
     *        GSLSplineDeriv( x , y , GaudiMath::Interpolation::Cspline ) ;
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param x    vector of x
     *  @param y    vector of y
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineDeriv::GSLSplineDeriv
    ( const GSLSplineDeriv::Data1D&        x    ,
      const GSLSplineDeriv::Data1D&        y    ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline ( x , y , type )
    {}
    // ========================================================================

    // ========================================================================
    /** constructor from data vector
     *
     *  @code
     *
     *   Data2D data = ... ;
     *
     *   const AbsFunction& func =
     *        GSLSpline( data , GaudiMath::Interpolation::Cspline );
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param data vector of (x,y) pairs
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineDeriv::GSLSplineDeriv
    ( const GSLSplineDeriv::Data2D&        data ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline    ( data , type )
    {}
    // ========================================================================

    // ========================================================================
    /// constructor from SplineBase
    // ========================================================================
    GSLSplineDeriv::GSLSplineDeriv
    ( const SplineBase& right )
      : AbsFunction ()
      , m_spline    ( right )
    {}
    // ========================================================================

    // ========================================================================
    /// copy constructor
    // ========================================================================
    GSLSplineDeriv::GSLSplineDeriv
    ( const GSLSplineDeriv& right )
      : AbsFunction ()
      , m_spline    ( right  )
    {}
    // ========================================================================

    // ========================================================================
    double GSLSplineDeriv::operator() (       double    x ) const
    { return m_spline.deriv ( x    ) ; }
    // ========================================================================
    double GSLSplineDeriv::operator() ( const Argument& x ) const
    { return m_spline.deriv ( x[0] ) ; }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative GSLSplineDeriv::partial( unsigned int i  ) const
    {
      if ( i >= 1 )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0.0 , 1 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      }
      const AbsFunction& aux = GSLSplineDeriv2( *this ) ;
      return Genfun::FunctionNoop( &aux ) ;
    }
    // ========================================================================

    // ========================================================================
    /** constructor from vectors and type
     *
     *  @code
     *
     *   Data1D x = .. ;
     *   Data1D y = .. ;
     *
     *   const AbsFunction& func =
     *        GSLSplineDeriv( x , y , GaudiMath::Interpolation::Cspline ) ;
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param x    vector of x
     *  @param y    vector of y
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineDeriv2::GSLSplineDeriv2
    ( const GSLSplineDeriv2::Data1D&       x    ,
      const GSLSplineDeriv2::Data1D&       y    ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline ( x , y , type )
    {}
    // ========================================================================

    // ========================================================================
    /** constructor from data vector
     *
     *  @code
     *
     *   Data2D data = ... ;
     *
     *   const AbsFunction& func =
     *        GSLSplineDeriv2( data , GaudiMath::Interpolation::Cspline );
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param data vector of (x,y) pairs
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineDeriv2::GSLSplineDeriv2
    ( const GSLSplineDeriv2::Data2D&       data ,
      const GaudiMath::Interpolation::Type type )
      : AbsFunction ()
      , m_spline    ( data , type )
    {}
    // ========================================================================

    // ========================================================================
    /// constructor from SplineBase
    // ========================================================================
    GSLSplineDeriv2::GSLSplineDeriv2
    ( const SplineBase& right )
      : AbsFunction ()
      , m_spline    ( right )
    {}
    // ========================================================================

    // ========================================================================
    double GSLSplineDeriv2::operator() (       double    x ) const
    { return m_spline.deriv2 ( x    ) ; }
    // ========================================================================
    double GSLSplineDeriv2::operator() ( const Argument& x ) const
    { return m_spline.deriv2 ( x[0] ) ; }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative GSLSplineDeriv2::partial( unsigned int i  ) const
    {
      if ( i >= 1 )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0.0 , 1 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      }
      const AbsFunction& aux = GaudiMath::Derivative( *this , i ) ;
      return Genfun::FunctionNoop( &aux ) ;
    }
    // ========================================================================

    // ========================================================================
    /** constructor from vectors and type
     *
     *  @code
     *
     *   Data1D x = .. ;
     *   Data1D y = .. ;
     *
     *   const AbsFunction& func =
     *        GSLSplineInteg( x , y , GaudiMath::Interpolation::Cspline , -1 ) ;
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param x    vector of x
     *  @param y    vector of y
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineInteg::GSLSplineInteg
    ( const GSLSplineInteg::Data1D&        x     ,
      const GSLSplineInteg::Data1D&        y     ,
      const GaudiMath::Interpolation::Type type  ,
      const double                         low   )
      : AbsFunction (      )
      , m_spline    ( x    , y , type )
      , m_low       ( low  )
    {}
    // ========================================================================

    // ========================================================================
    /** constructor from data vector
     *
     *  @code
     *
     *   Data2D data = ... ;
     *
     *   const AbsFunction& func =
     *        GSLSplineInteg( data , GaudiMath::Interpolation::Cspline , -1 );
     *
     *   const double value = 1 ;
     *   const double result = func( value ) ;
     *
     *  @endcode
     *
     *
     *  @param data vector of (x,y) pairs
     *  @param type interpolation type
     */
    // ========================================================================
    GSLSplineInteg::GSLSplineInteg
    ( const GSLSplineInteg::Data2D&        data  ,
      const GaudiMath::Interpolation::Type type  ,
      const double                         low   )
      : AbsFunction ()
      , m_spline    ( data , type )
      , m_low       ( low  )
    {}
    // ========================================================================

    // ========================================================================
    /// constructor from SplineBase
    // ========================================================================
    GSLSplineInteg::GSLSplineInteg
    ( const SplineBase& right ,
      const double      low   )
      : AbsFunction ()
      , m_spline    ( right )
      , m_low       ( low   )
    {}
    // ========================================================================

    // ========================================================================
    double GSLSplineInteg::operator() (       double    x ) const
    { return m_spline.integ ( m_low , x    ) ; }
    // ========================================================================
    double GSLSplineInteg::operator() ( const Argument& x ) const
    { return m_spline.integ ( m_low , x[0] ) ; }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative GSLSplineInteg::partial( unsigned int i  ) const
    {
      if ( i >= 1 )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0.0 , 1 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      }
      const AbsFunction& aux = GSLSpline( *this ) ;
      return Genfun::FunctionNoop( &aux ) ;
    }
    // ========================================================================

  }
}

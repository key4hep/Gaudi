// $Id: Adapter.cpp,v 1.2 2004/03/28 18:50:36 mato Exp $
// Include files
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/GaudiMath.h"
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IFunction.h"
// ============================================================================
#include <cstring>

namespace Genfun
{
  namespace GaudiMathImplementation
  {

    FUNCTION_OBJECT_IMP( AdapterIFunction )

    AdapterIFunction::AdapterIFunction ( const AIDA::IFunction& fun)
      : AbsFunction ( )
      , m_fun ( &fun  )
      , m_dim ( fun.dimension() )
      , m_arg ( fun.dimension() , 0 )
    {}

    AdapterIFunction:: AdapterIFunction ( const AdapterIFunction& right )
      : AbsFunction (  )
      , m_fun ( right.m_fun )
      , m_dim ( right.m_dim )
      , m_arg ( right.m_arg )
    {}

    AdapterIFunction::~AdapterIFunction(){}

    double AdapterIFunction::operator() ( double x ) const
    {
      for ( size_t i = 0; i < m_dim; ++i ) { m_arg[i] = 0.0 ; }
      m_arg[0] = x ;
      return m_fun -> value ( m_arg ) ;
    }

    double AdapterIFunction::operator() ( const Genfun::Argument& x ) const
    {
      for ( size_t i = 0; i < m_dim; ++i ) { m_arg[i] = x[i] ; }
      return m_fun -> value ( m_arg ) ;
    }

    Genfun::Derivative AdapterIFunction::partial( unsigned int i  ) const
    {
      if ( i >= m_dim )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0 , m_dim  ) ;
        return Genfun::FunctionNoop( &aux ) ;
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this , i ) ;
      return Genfun::FunctionNoop ( &aux ) ;
    }

    FUNCTION_OBJECT_IMP( Adapter2DoubleFunction )

    Adapter2DoubleFunction::Adapter2DoubleFunction
    ( Adapter2DoubleFunction::Function  func )
      : AbsFunction (       )
      , m_func      ( func  )
    {}

    Adapter2DoubleFunction:: Adapter2DoubleFunction
    ( const Adapter2DoubleFunction& right )
      : AbsFunction (              )
      , m_func    ( right.m_func )
    {}

    Adapter2DoubleFunction::~Adapter2DoubleFunction(){}

    double Adapter2DoubleFunction::operator()
      (       double    x ) const
    { return m_func ( x , 0 ) ; }

    double Adapter2DoubleFunction::operator()
      ( const Genfun::Argument& x ) const
    { return m_func ( x[0] , x[1] ) ; }

    double Adapter2DoubleFunction::operator()
      ( const double x ,
        const double y ) const
    { return m_func ( x , y ) ; }

    /// Derivatives
    Genfun::Derivative Adapter2DoubleFunction::partial( unsigned int i  ) const
    {
      if ( i >= 2  )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0 , 2 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this , i ) ;
      return Genfun::FunctionNoop ( &aux ) ;
    }

    FUNCTION_OBJECT_IMP( Adapter3DoubleFunction )

    Adapter3DoubleFunction::Adapter3DoubleFunction
    ( Adapter3DoubleFunction::Function  func )
      : AbsFunction (       )
      , m_func      ( func  )
    {}

    Adapter3DoubleFunction:: Adapter3DoubleFunction
    ( const Adapter3DoubleFunction& right )
      : AbsFunction (              )
      , m_func      ( right.m_func )
    {}

    Adapter3DoubleFunction::~Adapter3DoubleFunction(){}

    double Adapter3DoubleFunction::operator()
      (       double    x ) const
    { return m_func ( x , 0 , 0 ) ; }

    double Adapter3DoubleFunction::operator()
      ( const Genfun::Argument& x ) const
    { return m_func ( x[0] , x[1] , x[2] ) ; }

    double Adapter3DoubleFunction::operator()
      ( const double x ,
        const double y ,
        const double z ) const
    { return m_func ( x , y , z ) ; }

    /// Derivatives
    Genfun::Derivative Adapter3DoubleFunction::partial( unsigned int i  ) const
    {
      if ( i >= 3  )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0 , 3 ) ;
        return Genfun::FunctionNoop( &aux ) ;
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this , i ) ;
      return Genfun::FunctionNoop ( &aux ) ;
    }

    // ========================================================================
    /// from CLHGEP/GenericFunctions
    // ========================================================================
    FUNCTION_OBJECT_IMP( SimpleFunction )
    // ========================================================================

    // =======================================================================
    /** constructor from the trivial function
     *  @param func pointer to trivial function
     */
    // ========================================================================
    SimpleFunction::SimpleFunction
    ( SimpleFunction::Function1 func )
      : AbsFunction ()
      , m_case      ( SimpleFunction::TrivialArg )
      , m_DIM       ( 1    )
      , m_func1     ( func )
      , m_func2     ( 0    )
      , m_arg2      ( 0    )
      , m_func3     ( 0    )
      , m_arg3      (      )
    {}
    // ========================================================================

    // ========================================================================
    /** constructor from the simple function with array-like argument
     *  @param func pointer to trivial function
     *  @param dim  dimension of the argument
     */
    // ========================================================================
    SimpleFunction::SimpleFunction
    ( SimpleFunction::Function2 func ,
      const size_t              dim  )
      : AbsFunction ()
      , m_case      ( SimpleFunction::ArrayArg  )
      , m_DIM       ( dim  )
      , m_func1     ( 0    )
      , m_func2     ( func )
      , m_arg2      ( 0    )
      , m_func3     ( 0    )
      , m_arg3      (      )
    {
      m_arg2 = new double[dim];
    }
    // ========================================================================

    // ========================================================================
    /** constructor from the simple function with array-like argument
     *  @param func pointer to trivial function
     *  @param dim  dimension of the argument
     */
    // ========================================================================
    SimpleFunction::SimpleFunction
    ( SimpleFunction::Function3 func ,
      const size_t              dim  )
      : AbsFunction ()
      , m_case      ( SimpleFunction::VectorArg  )
      , m_DIM       ( dim  )
      , m_func1     ( 0    )
      , m_func2     ( 0    )
      , m_arg2      ( 0    )
      , m_func3     ( func )
      , m_arg3      ( dim  , 0 )
    {}
    // ========================================================================

    // ========================================================================
    /// copy constructor
    // ========================================================================
    SimpleFunction::SimpleFunction
    ( const SimpleFunction& right )
      : AbsFunction ()
      , m_case      ( right.m_case  )
      , m_DIM       ( right.m_DIM   )
      , m_func1     ( right.m_func1 )
      , m_func2     ( right.m_func2 )
      , m_arg2      ( 0             )
      , m_func3     ( right.m_func3 )
      , m_arg3      ( right.m_arg3  )
    {
      if (  0 != right.m_arg2 )
      {
        std::memcpy(m_arg2, right.m_arg2, m_DIM);
      }
    }
    // ========================================================================

    // ========================================================================
    /// destructor
    // ========================================================================
    SimpleFunction::~SimpleFunction()
    { if( 0 != m_arg2 ) { delete m_arg2 ; m_arg2 = 0 ;} }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative SimpleFunction::partial( unsigned int i ) const
    {
      if ( i >= m_DIM )
      {
        const AbsFunction& aux = GaudiMath::Constant( 0 , m_DIM ) ;
        return Genfun::FunctionNoop( &aux ) ;
      }
      const  AbsFunction& aux = GaudiMath::Derivative( *this , i ) ;
      return Genfun::FunctionNoop( &aux );
    }

    // ========================================================================
    /// Function value
    // ========================================================================
    double SimpleFunction::operator()  ( double value ) const
    {
      double result = 0 ;
      switch ( m_case )
      {
      case TrivialArg  :
        result = (*m_func1) ( value  ) ; break ;
      case ArrayArg    :
        std::fill ( m_arg2 , m_arg2 + m_DIM , 0.0 );
        m_arg2[0] = value  ;
        result = (*m_func2) ( m_arg2 ) ; break ;
      case VectorArg   :
        std::fill ( m_arg3.begin () , m_arg3.end () , 0.0 );
        m_arg3[0] = value  ;
        result = (*m_func3) ( m_arg3 ) ; break ;
      default:
        break ;
      };
      return result ;
    }
    // ========================================================================

    // ========================================================================
    /// Function value
    // ========================================================================
    double SimpleFunction::operator()  ( const Argument& argument ) const
    {
      double result = 0 ;
      switch ( m_case )
      {
      case TrivialArg  :
        result = (*m_func1) ( argument[0]  ) ; break ;
      case ArrayArg    :
        for( size_t i = 0 ; i < m_DIM ; ++i ) { m_arg2[i] = argument[i] ; }
        return (*m_func2)( m_arg2 ) ;        ; break ;
      case VectorArg   :
        for( size_t i = 0 ; i < m_DIM ; ++i ) { m_arg3[i] = argument[i] ; }
        result = (*m_func3) ( m_arg3 ) ; break ;
      default:
        break ;
      }
      return result ;
    }

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun


// ============================================================================
// The END
// ============================================================================

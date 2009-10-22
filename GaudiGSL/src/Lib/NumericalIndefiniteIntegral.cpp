// $Id: NumericalIndefiniteIntegral.cpp,v 1.2 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
// Include
// ============================================================================
// STD & STL 
// ============================================================================
#include <vector>
#include <algorithm>
// ============================================================================
// GSL
// ============================================================================
#include "gsl/gsl_errno.h"
#include "gsl/gsl_integration.h"
// ============================================================================
// GaudiMath 
// ============================================================================
#include "GaudiMath/NumericalIndefiniteIntegral.h"
#include "GaudiMath/NumericalDerivative.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// local 
// ============================================================================
#include "Helpers.h"
// ============================================================================

// ============================================================================
/** @file
 *  Implementation of class NumericalIndefiniteIntegral
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

namespace Genfun 
{
  namespace GaudiMathImplementation 
  {
    
    struct NumericalIndefiniteIntegral::_Workspace 
    { gsl_integration_workspace* ws ; };
    struct NumericalIndefiniteIntegral::_Function  
    { gsl_function*              fn ; };
    
    // ========================================================================
    /// from CLHEP/GenericFunctions
    // ========================================================================
    FUNCTION_OBJECT_IMP( NumericalIndefiniteIntegral );
    // ========================================================================

    // ========================================================================
    /** Standard constructor
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
    // ========================================================================
    NumericalIndefiniteIntegral::NumericalIndefiniteIntegral
    ( const AbsFunction&                         function  ,
      const size_t                               index     , 
      const double                               a         , 
      const GaudiMath::Integration::Limit        limit     , 
      const GaudiMath::Integration::Type         type      , 
      const GaudiMath::Integration::KronrodRule  rule      , 
      const double                               epsabs    , 
      const double                               epsrel    , 
      const size_t                               size      ) 
      : AbsFunction () 
      , m_function  ( function.clone()                    )
      , m_DIM       ( function.dimensionality()           )
      , m_index     ( index                               )
      , m_a         ( a                                   ) 
      , m_limit     ( limit                               )
      , m_type      ( type                                ) 
      , m_category  ( GaudiMath::Integration::Finite      ) 
      , m_rule      ( rule                                ) 
      //
      , m_points    (                                     ) 
      , m_pdata     ( 0                                   )
      //
      , m_epsabs    ( epsabs                              ) 
      , m_epsrel    ( epsrel                              ) 
      //
      , m_result    ( GSL_NEGINF                          ) 
      , m_error     ( GSL_POSINF                          ) 
      //
      , m_size      ( size                                ) 
      , m_ws        ( 0                                   ) 
      , m_argument  ( function.dimensionality()           ) 
    {
      if ( GaudiMath::Integration::Fixed == m_rule ) 
        { m_rule = GaudiMath::Integration::Default ; }
      if ( m_index >= m_DIM  ) 
        { Exception("::constructor: invalid variable index") ; }
    };
    // ========================================================================
    
    /** standard constructor 
     *  @param function the base function 
     *  @param index    the variable index 
     *  @param a        integration limit 
     *  @param limit flag to distinguisch low variable limit 
     *               from high variable limit
     *  @param points   list of known function singularities 
     *  @param epsabs   absolute precision for integration 
     *  @param epsrel   relative precision for integration 
     */
    NumericalIndefiniteIntegral::NumericalIndefiniteIntegral 
    ( const AbsFunction&                   function ,
      const size_t                         index    , 
      const double                         a        , 
      const Points&                        points   ,
      const GaudiMath::Integration::Limit  limit    , 
      const double                         epsabs   , 
      const double                         epsrel   ,
      const size_t                         size     ) 
      : AbsFunction () 
      , m_function  ( function.clone()          )
      , m_DIM       ( function.dimensionality() ) 
      , m_index     ( index                     ) 
      , m_a         ( a                         )
      , m_limit     ( limit                     )
      , m_type      ( GaudiMath::Integration::    Other )
      , m_category  ( GaudiMath::Integration:: Singular )
      , m_rule      ( GaudiMath::Integration::    Fixed )
      , m_points    ( points  ) 
      , m_pdata     ( 0       ) 
      , m_epsabs    ( epsabs  )
      , m_epsrel    ( epsrel  ) 
      //
      , m_result    ( GSL_NEGINF                          ) 
      , m_error     ( GSL_POSINF                          ) 
      //
      , m_size      ( size                                ) 
      , m_ws        ( 0                                   ) 
      , m_argument  ( function.dimensionality()           ) 
    {
      if ( m_index >= m_DIM ) 
        { Exception("::constructor: invalid variable index") ; } 
      m_pdata = new double[ 2 + m_points.size() ] ;
      m_points.push_back( a ) ;
      std::sort( m_points.begin() , m_points.end() ) ;
      m_points.erase ( std::unique( m_points.begin () , 
                                    m_points.end   () ) , m_points.end() );
    };
    
    // ========================================================================
    /** standard constructor 
     *  the integral limt is assumed to be infinity 
     *  @param function the base function 
     *  @param index    the variable index 
     *  @param limit flag to distinguisch low variable limit 
     *               from high variable limit
     */
    // ========================================================================
    NumericalIndefiniteIntegral::NumericalIndefiniteIntegral 
    ( const AbsFunction&                  function  ,
      const size_t                        index     , 
      const GaudiMath::Integration::Limit limit     ,
      const double                        epsabs    , 
      const double                        epsrel    , 
      const size_t                        size      ) 
      : AbsFunction () 
      , m_function  ( function.clone() ) 
      , m_DIM       ( function.dimensionality() )
      , m_index     ( index            )
      , m_a         ( GSL_NEGINF       ) // should not be used! 
      , m_limit     ( limit            )
      , m_type      ( GaudiMath::Integration::    Other ) 
      , m_category  ( GaudiMath::Integration:: Infinite )
      , m_rule      ( GaudiMath::Integration::    Fixed )
      , m_points    (            ) 
      , m_pdata     ( 0          )
      , m_epsabs    ( epsabs     ) 
      , m_epsrel    ( epsrel     )
      , m_result    ( GSL_NEGINF ) 
      , m_error     ( GSL_POSINF ) 
      , m_size      ( size       ) 
      , m_ws        ( 0          ) 
      , m_argument  ( function.dimensionality()           ) 
    {
      if ( m_index >= m_DIM ) 
        { Exception("::constructor: invalid variable index") ; }
    };
    // ========================================================================

    
    // ========================================================================
    /// copy constructor 
    // ========================================================================
    NumericalIndefiniteIntegral::
    NumericalIndefiniteIntegral( const NumericalIndefiniteIntegral& right )
      : AbsFunction () 
      , m_function  ( right.m_function->clone() ) 
      , m_DIM       ( right.m_DIM      ) 
      , m_index     ( right.m_index    ) 
      , m_a         ( right.m_a        ) 
      , m_limit     ( right.m_limit    ) 
      , m_type      ( right.m_type     ) 
      , m_category  ( right.m_category )
      , m_rule      ( right.m_rule     )
      , m_points    ( right.m_points   ) 
      , m_pdata     ( 0 )           // attention 
      , m_epsabs    ( right.m_epsabs   ) 
      , m_epsrel    ( right.m_epsrel   ) 
      , m_result    ( GSL_NEGINF       ) 
      , m_error     ( GSL_POSINF       )
      , m_size      ( right.m_size     ) 
      , m_ws        ( 0                )
      , m_argument  ( right.m_argument )
    {
      m_pdata = new double[ 2 + m_points.size() ] ; // attention! 
    };
    // ========================================================================

      
    // ========================================================================
    /// destructor 
    // ========================================================================
    NumericalIndefiniteIntegral::~NumericalIndefiniteIntegral()
    {
      if( 0 != m_ws ) 
        { 
          gsl_integration_workspace_free ( m_ws->ws ) ;
          delete m_ws ;
          m_ws = 0 ;
        }
      if ( 0 != m_pdata    ) { delete m_pdata    ; m_pdata    = 0 ; }
      if ( 0 != m_function ) { delete m_function ; m_function = 0 ; }
    };
    // ========================================================================

    // ========================================================================
    // throw the exception
    // ========================================================================
    StatusCode NumericalIndefiniteIntegral::Exception 
    ( const std::string& message , 
      const StatusCode&  sc      ) const 
    {
      throw GaudiException( "NumericalIndefiniteIntegral::" + message , 
                            "*GaudiMath*" , sc ) ;
      return sc ;
    };
    // ========================================================================
    
    // ========================================================================
    /// evaluate the function 
    // ========================================================================
    double NumericalIndefiniteIntegral::operator() 
      ( const double argument ) const 
    {
      // reset the result and the error  
      m_result = GSL_NEGINF ;
      m_error  = GSL_POSINF ;
      
      // check the argument 
      if( 1 != m_DIM ) { Exception ( "operator(): invalid argument size " ) ; };
      
      m_argument[0] = argument ;
      return (*this) ( m_argument );
    };
    // ========================================================================

    // ========================================================================
    /// Derivatives 
    // ========================================================================
    Genfun::Derivative 
    NumericalIndefiniteIntegral::partial ( unsigned int idx ) const
    {
      if      ( idx >= m_DIM   )
        { Exception ( "::partial(i): invalid variable index " ) ; };
      if      ( idx != m_index )
        {
          const AbsFunction& aux = NumericalDerivative( *this , idx ) ;
          return Genfun::FunctionNoop( &aux ) ;
        }
      else if ( GaudiMath::Integration::VariableLowLimit == limit () ) 
        { 
          const AbsFunction& aux = -1 * function() ;
          return Genfun::FunctionNoop( &aux ) ;
        }
      const AbsFunction& aux = function() ;
      return Genfun::FunctionNoop( &aux ) ;
    };
    
    // ========================================================================
    /// evaluate the function 
    // ========================================================================
    double NumericalIndefiniteIntegral::operator() 
      ( const Argument& argument ) const 
    {
      // reset the result and the error  
      m_result = GSL_NEGINF ;
      m_error  = GSL_POSINF ;
      
      // check the argument 
      if( argument.dimension() != m_DIM ) 
        { Exception ( "operator(): invalid argument size " ) ; };
      
      // copy the argument 
      {for( size_t i  = 0 ; i < m_DIM ; ++i ){ m_argument[i] = argument[i];}}
      
      // create the helper object 
      GSL_Helper helper( *m_function , m_argument , m_index );
      
      // use GSL to evaluate the numerical derivative 
      gsl_function F ;
      F.function = &GSL_Adaptor ;
      F.params   = &helper                 ;
      _Function F1    ;
      F1.fn      = &F ;
      
      if        (  GaudiMath::Integration::Infinite         == category () )
        { return   QAGI ( &F1 ) ; }                                // RETURN
      else if   (  GaudiMath::Integration::Singular         == category () )
        { return   QAGP ( &F1 ) ; }                                // RETURN
      else if   (  GaudiMath::Integration::Finite           == category () )
        if      (  GaudiMath::Integration::NonAdaptive      == type     () ) 
          { return QNG  ( &F1 ) ; }                                // RETURN
        else if (  GaudiMath::Integration::Adaptive         == type     () ) 
          { return QAG  ( &F1 ) ; }                                // RETURN
        else if (  GaudiMath::Integration::AdaptiveSingular == type     () ) 
          { return QAGS ( &F1 ) ; }                                // RETURN
        else 
          { Exception ( "::operator(): invalid type "  ); }
      else 
        { Exception ( "::operator(): invalid category "  ); }
      
      return 0 ;
    };
    // ========================================================================
    
    // ========================================================================
    /// allocate the integration workspace 
    // ======================================================================== 
    NumericalIndefiniteIntegral::_Workspace* 
    NumericalIndefiniteIntegral::allocate() const 
    {
      if ( 0 != m_ws ) { return m_ws; }
      gsl_integration_workspace* aux = 
        gsl_integration_workspace_alloc( size () );
      if ( 0 == aux ) { Exception ( "allocate()::invalid workspace" ) ; };
      m_ws = new _Workspace() ;
      m_ws->ws = aux ;
      return m_ws ;
    };
    // ========================================================================
    
    // ========================================================================
    // adaptive integration on infinite interval
    // ========================================================================
    double NumericalIndefiniteIntegral::QAGI ( _Function* F ) const 
    {
      // check the argument 
      if( 0 == F ) { Exception("::QAGI: invalid function"); }
      
      const double x = m_argument[m_index] ;
      
      // allocate workspace 
      if( 0 == ws() ) { allocate() ; }
      
      int ierror = 0 ;
      switch ( limit() ) 
        {
        case GaudiMath::Integration::VariableLowLimit  : 
          ierror = gsl_integration_qagiu ( F->fn     , x        ,  
                                           m_epsabs  , m_epsrel , 
                                           size ()   , ws()->ws , 
                                           &m_result , &m_error ) ; break ;
        case GaudiMath::Integration::VariableHighLimit :
          ierror = gsl_integration_qagil ( F->fn     , x        ,
                                           m_epsabs  , m_epsrel , 
                                           size ()   , ws()->ws , 
                                           &m_result , &m_error ) ; break ;
        default :
          Exception ( "::QAGI: invalid mode" ) ;
        };
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QAGI" , 
                                __FILE__ , __LINE__ , ierror ) ;}
      
      return m_result ;
    };
    // ========================================================================
    
    // ========================================================================
    // adaptive integration with known singular points 
    // ========================================================================
    double NumericalIndefiniteIntegral::QAGP( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAGP::invalid function") ; }
      
      const double x = m_argument[m_index] ;
      if ( m_a == x  ) 
        { 
          m_result = 0    ;
          m_error  = 0    ;   // EXACT !
          return m_result ; 
        }
      
      // no known singular points ?
      if( points().empty() ) { return QAGS( F ) ; }
      
      // integration limits 
      const double a = std::min ( m_a , x ) ;
      const double b = std::max ( m_a , x ) ;
      
      // "active" singular points
      Points::const_iterator lower = 
        std::lower_bound ( points().begin() , points().end() , a ) ;
      Points::const_iterator upper = 
        std::upper_bound ( points().begin() , points().end() , b ) ;
      
      Points pnts ( upper - lower ) ;
      std::copy( lower , upper , pnts.begin() );
      if ( *lower != a       ) { pnts.insert( pnts.begin () , a ) ; }
      if ( *upper != b       ) { pnts.insert( pnts.end   () , b ) ; }
      std::copy( pnts.begin() , pnts.end() , m_pdata );
      const size_t npts = pnts.size() ;
      
      // use GSL 
      int ierror = 
        gsl_integration_qagp ( F->fn                , 
                               m_pdata   , npts     , 
                               m_epsabs  , m_epsrel ,
                               size ()   , ws()->ws , 
                               &m_result , &m_error ) ;
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QAGI " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if      ( GaudiMath::Integration::VariableHighLimit == limit() 
                &&  x < m_a  ) { m_result *= -1 ; }
      else if ( GaudiMath::Integration::VariableLowLimit  == limit() 
                &&  x > m_a  ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================
    
    // ========================================================================
    // non-adaptive integration 
    // ========================================================================
    double NumericalIndefiniteIntegral::QNG ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QNG::invalid function") ; }
      
      const double x = m_argument[m_index] ;
      if ( m_a == x  ) 
        { 
          m_result = 0    ;
          m_error  = 0    ;   // EXACT !
          return m_result ; 
        }
      
      // integration limits 
      const double a = std::min ( m_a , x ) ;
      const double b = std::max ( m_a , x ) ;
      
      size_t neval = 0 ;
      int ierror = 
        gsl_integration_qng ( F->fn                 ,  
                              a         ,         b , 
                              m_epsabs  ,  m_epsrel , 
                              &m_result , &m_error  , &neval  ) ;
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QNG " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if      ( GaudiMath::Integration::VariableHighLimit == limit() 
                &&  x < m_a  ) { m_result *= -1 ; }
      else if ( GaudiMath::Integration::VariableLowLimit  == limit() 
                &&  x > m_a  ) { m_result *= -1 ; }

      return m_result ;
    };
    
    // ========================================================================
    // simple adaptive integration 
    // ========================================================================
    double NumericalIndefiniteIntegral::QAG ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAG::invalid function") ; }
      
      const double x = m_argument[m_index] ;
      if ( m_a == x  ) 
        { 
          m_result = 0    ;
          m_error  = 0    ;   // EXACT !
          return m_result ; 
        }
      
      // allocate workspace 
      if( 0 == ws () ) { allocate () ; }
      
      // integration limits 
      const double a = std::min ( m_a , x ) ;
      const double b = std::max ( m_a , x ) ;
      
      int ierror = 
        gsl_integration_qag ( F->fn                    , 
                              a         ,            b , 
                              m_epsabs  ,     m_epsrel , 
                              size   () , (int) rule() , ws ()->ws , 
                              &m_result ,     &m_error             );
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QAG " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if      ( GaudiMath::Integration::VariableHighLimit == limit() 
                &&  x < m_a  ) { m_result *= -1 ; }
      else if ( GaudiMath::Integration::VariableLowLimit  == limit() 
                &&  x > m_a  ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================
    
    // ========================================================================
    // adaptive integration with singularities 
    // ========================================================================
    double NumericalIndefiniteIntegral::QAGS ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAG::invalid function") ; }
      
      const double x = m_argument[m_index] ;
      if ( m_a == x  ) 
        { 
          m_result = 0    ;
          m_error  = 0    ;   // EXACT !
          return m_result ; 
        }
      
      // allocate workspace 
      if( 0 == ws () ) { allocate () ; }
      
      // integration limits 
      const double a = std::min ( m_a , x ) ;
      const double b = std::max ( m_a , x ) ;
      
      int ierror = 
        gsl_integration_qags ( F->fn                , 
                               a         , b        , 
                               m_epsabs  , m_epsrel , 
                               size   () , ws()->ws , 
                               &m_result , &m_error );
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QAGS " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if      ( GaudiMath::Integration::VariableHighLimit == limit() 
                &&  x < m_a  ) { m_result *= -1 ; }
      else if ( GaudiMath::Integration::VariableLowLimit  == limit() 
                &&  x > m_a  ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================

  }; // end of namespace GaudiMathImplementation
}; // end of namespace Genfun


// ============================================================================
// The END 
// ============================================================================

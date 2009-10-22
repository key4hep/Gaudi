// $Id: NumericalDefiniteIntegral.cpp,v 1.4 2007/11/20 13:00:17 marcocle Exp $
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
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/NumericalDefiniteIntegral.h" 
#include "GaudiMath/NumericalDerivative.h" 
// ============================================================================
// Local 
// ============================================================================
#include "Helpers.h" 
// ============================================================================



namespace Genfun
{
  namespace GaudiMathImplementation
  {
    
    struct NumericalDefiniteIntegral::_Workspace 
    { gsl_integration_workspace* ws ; };
    struct NumericalDefiniteIntegral::_Function  
    { gsl_function*              fn ; };
    
    // ========================================================================
    /// from CLHEP/GenericFunctions
    // ========================================================================
    FUNCTION_OBJECT_IMP( NumericalDefiniteIntegral );
    // ========================================================================
    
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
    NumericalDefiniteIntegral::NumericalDefiniteIntegral
    ( const AbsFunction&                        function , 
      const size_t                              index    , 
      const double                              a        , 
      const double                              b        , 
      const GaudiMath::Integration::Type        type     ,
      const GaudiMath::Integration::KronrodRule rule     ,
      const double                              epsabs   ,
      const double                              epsrel   ,
      const size_t                              size     )
      : AbsFunction () 
      , m_function  ( function.clone          ()    ) 
      , m_DIM       ( 0                             )
      , m_index     ( index                         )
      , m_a         ( a      ) 
      , m_b         ( b      )
      , m_ia        ( false  ) 
      , m_ib        ( false  )
      , m_type      ( type   ) 
      , m_category  ( GaudiMath::Integration::Finite )
      , m_rule      ( rule       ) 
      , m_points    (            ) 
      , m_pdata     ( 0          ) 
      , m_epsabs    ( epsabs     )
      , m_epsrel    ( epsrel     )
      , m_result    ( GSL_NEGINF ) 
      , m_error     ( GSL_POSINF ) 
      , m_size      ( size       ) 
      , m_ws        ()
      , m_argument  ()
      , m_argF      () 
    {
      if ( GaudiMath::Integration::Fixed == m_rule ) 
        { m_rule = GaudiMath::Integration::Default ; }
      if ( function.dimensionality() < 2          ) 
        { Exception("::constructor: invalid dimensionality ") ; }        
      if ( m_index >= function.dimensionality()   ) 
        { Exception("::constructor: invalid variable index") ; }

      m_DIM = function.dimensionality() - 1 ;
      m_argument = Argument( m_DIM      ) ;
      m_argF     = Argument( m_DIM + 1  ) ;

    };  
                   
                     
    /** standard constructor 
     *  @param function the base function 
     *  @param index    the variable index 
     *  @param a        integration limit 
     *  @param b        integration limit 
     *  @param points   list of known function singularities 
     *  @param epsabs   absolute precision for integration 
     *  @param epsrel   relative precision for integration 
     */
    NumericalDefiniteIntegral::NumericalDefiniteIntegral 
    ( const AbsFunction&                       function ,
      const size_t                             index    , 
      const double                             a        , 
      const double                             b        , 
      const NumericalDefiniteIntegral::Points& points   ,
      const double                             epsabs   , 
      const double                             epsrel   ,
      const size_t                             size     ) 
      : AbsFunction () 
      , m_function  ( function.clone()             )
      , m_DIM       ( 0                            ) 
      , m_index     ( index                        ) 
      , m_a         ( a                            )
      , m_b         ( b                            )
      , m_ia        ( false  ) 
      , m_ib        ( false  )
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
      , m_argument  () 
      , m_argF      () 
    {
      if ( function.dimensionality() < 2          ) 
        { Exception("::constructor: invalid dimensionality ") ; }        
      if ( m_index >= function.dimensionality()   ) 
        { Exception("::constructor: invalid variable index") ; }

      m_DIM = function.dimensionality() - 1 ;
      m_argument = Argument( m_DIM      ) ;
      m_argF     = Argument( m_DIM + 1  ) ;

      const double l1 = std::min ( a , b ) ;
      const double l2 = std::max ( a , b ) ;
      m_points.push_back ( l1 ) ;
      m_points.push_back ( l2 ) ;
      std::sort ( m_points.begin() , m_points.end() ) ;
      m_points.erase( std::unique( m_points.begin () , 
                                   m_points.end   () ) , 
                      m_points.end() );
      
      Points::iterator lower = 
        std::lower_bound ( m_points.begin () , m_points.end () , l1 ) ;
      m_points.erase     ( m_points.begin () , lower                ) ;
      Points::iterator upper = 
        std::upper_bound ( m_points.begin () , m_points.end () , l2 ) ;
      m_points.erase     ( upper             , m_points.end ()      ) ;
      
      m_pdata = new double[ m_points.size() ] ;
      std::copy( m_points.begin() , m_points.end() , m_pdata );
    };
    
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
    NumericalDefiniteIntegral::NumericalDefiniteIntegral 
    ( const AbsFunction&                      function  ,
      const size_t                            index     , 
      const double                            a         ,
      const GaudiMath::Integration::Inf       /* b  */  ,
      const double                            epsabs    , 
      const double                            epsrel    , 
      const size_t                            size      ) 
      : AbsFunction() 
      , m_function  ( function.clone()             )
      , m_DIM       ( 0                            ) 
      , m_index     ( index                        ) 
      , m_a         ( a                            )
      , m_b         ( GSL_POSINF                   )
      , m_ia        ( false  ) 
      , m_ib        ( true   )
      , m_type      ( GaudiMath::Integration::    Other )
      , m_category  ( GaudiMath::Integration:: Infinite )
      , m_rule      ( GaudiMath::Integration::    Fixed )
      , m_points    (         ) 
      , m_pdata     ( 0       ) 
      , m_epsabs    ( epsabs  )
      , m_epsrel    ( epsrel  ) 
      //
      , m_result    ( GSL_NEGINF                          ) 
      , m_error     ( GSL_POSINF                          ) 
      //
      , m_size      ( size                                ) 
      , m_ws        ( 0                                   ) 
      , m_argument  () 
      , m_argF      () 
    {
      if ( function.dimensionality() < 2          ) 
        { Exception("::constructor: invalid dimensionality ") ; }        
      if ( m_index >= function.dimensionality()   ) 
        { Exception("::constructor: invalid variable index") ; }

      m_DIM = function.dimensionality() - 1 ;
      m_argument = Argument( m_DIM      ) ;
      m_argF     = Argument( m_DIM + 1  ) ;

    };
    
    
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
    NumericalDefiniteIntegral::NumericalDefiniteIntegral 
    ( const AbsFunction&                      function  ,
      const size_t                            index     , 
      const GaudiMath::Integration::Inf       /* a  */  ,
      const double                            b         ,
      const double                            epsabs    , 
      const double                            epsrel    , 
      const size_t                            size      ) 
      : AbsFunction() 
      , m_function  ( function.clone()             )
      , m_DIM       ( 0                            ) 
      , m_index     ( index                        ) 
      , m_a         ( GSL_NEGINF                   )
      , m_b         ( b                            )
      , m_ia        ( true   ) 
      , m_ib        ( false  )
      , m_type      ( GaudiMath::Integration::    Other )
      , m_category  ( GaudiMath::Integration:: Infinite )
      , m_rule      ( GaudiMath::Integration::    Fixed )
      , m_points    (         ) 
      , m_pdata     ( 0       ) 
      , m_epsabs    ( epsabs  )
      , m_epsrel    ( epsrel  ) 
      //
      , m_result    ( GSL_NEGINF                          ) 
      , m_error     ( GSL_POSINF                          ) 
      //
      , m_size      ( size                                ) 
      , m_ws        ( 0                                   ) 
      , m_argument  () 
      , m_argF      () 
    {
      if ( function.dimensionality() < 2          ) 
        { Exception("::constructor: invalid dimensionality ") ; }        
      if ( m_index >= function.dimensionality()   ) 
        { Exception("::constructor: invalid variable index") ; }

      m_DIM = function.dimensionality() - 1 ;
      m_argument = Argument( m_DIM      ) ;
      m_argF     = Argument( m_DIM + 1  ) ;
    };

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
    NumericalDefiniteIntegral::NumericalDefiniteIntegral 
    ( const AbsFunction&                      function  ,
      const size_t                            index     , 
      const float                             epsabs    , 
      const float                             epsrel    ,
      const size_t                            size      ) 
      : AbsFunction() 
      , m_function  ( function.clone()             )
      , m_DIM       ( 0      ) 
      , m_index     ( index                        ) 
      , m_a         ( GSL_NEGINF                   )
      , m_b         ( GSL_POSINF                   )
      , m_ia        ( true   ) 
      , m_ib        ( true   )
      , m_type      ( GaudiMath::Integration::    Other )
      , m_category  ( GaudiMath::Integration:: Infinite )
      , m_rule      ( GaudiMath::Integration::    Fixed )
      , m_points    (         ) 
      , m_pdata     ( 0       ) 
      , m_epsabs    ( epsabs  )
      , m_epsrel    ( epsrel  ) 
      //
      , m_result    ( GSL_NEGINF                          ) 
      , m_error     ( GSL_POSINF                          ) 
      //
      , m_size      ( size                                ) 
      , m_ws        ( 0                                   ) 
      , m_argument  () 
      , m_argF      () 
    {
      if ( function.dimensionality() < 2          ) 
        { Exception("::constructor: invalid dimensionality ") ; }        
      if ( m_index >= function.dimensionality()   ) 
        { Exception("::constructor: invalid variable index") ; }
      
      m_DIM = function.dimensionality() - 1 ;
      m_argument = Argument( m_DIM      ) ;
      m_argF     = Argument( m_DIM + 1  ) ;
      
    };
    
    /// copy constructor 
    NumericalDefiniteIntegral::NumericalDefiniteIntegral
    ( const NumericalDefiniteIntegral& right )
      : AbsFunction () 
      , m_function  ( right.m_function->clone() ) 
      , m_DIM       ( right.m_DIM      )
      , m_index     ( right.m_index    )
      , m_a         ( right.m_a        ) 
      , m_b         ( right.m_b        )
      , m_ia        ( right.m_ia       ) 
      , m_ib        ( right.m_ib       )
      , m_type      ( right.m_type     ) 
      , m_category  ( right.m_category )
      , m_rule      ( right.m_rule     ) 
      , m_points    ( right.m_points   ) 
      , m_pdata     ( 0                ) 
      , m_epsabs    ( right.m_epsabs   )
      , m_epsrel    ( right.m_epsrel   )
      , m_result    ( GSL_NEGINF       ) 
      , m_error     ( GSL_POSINF       ) 
      , m_size      ( right.m_size     ) 
      , m_ws        ( 0                )
      , m_argument  ( right.m_argument )
      , m_argF      ( right.m_argF     )
    {
      if( 0 != right.m_pdata ) 
        {
          m_pdata = new double[m_points.size()] ;
          std::copy( m_points.begin() , m_points.end() , m_pdata );
        } 
    };

    NumericalDefiniteIntegral::~NumericalDefiniteIntegral()
    {
      if( 0 != m_function ) { delete m_function ; m_function = 0 ; }
      if( 0 != m_pdata    ) { delete m_pdata    ; m_pdata    = 0 ; }      
    };
    
    // ========================================================================
    // throw the exception
    // ========================================================================
    StatusCode NumericalDefiniteIntegral::Exception 
    ( const std::string& message , 
      const StatusCode&  sc      ) const 
    {
      throw GaudiException( "NumericalDefiniteIntegral::" + message , 
                            "*GaudiMath*" , sc ) ;
      return sc ;
    };
    // ========================================================================
    
    // ========================================================================
    /// evaluate the function 
    // ========================================================================
    double NumericalDefiniteIntegral::operator() 
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
    NumericalDefiniteIntegral::partial ( unsigned int idx ) const
    {
      if      ( idx >= m_DIM   )
        { Exception ( "::partial(i): invalid variable index " ) ; };
      //
      const AbsFunction& aux = NumericalDerivative( *this , idx ) ;
      return Genfun::FunctionNoop( &aux ) ;
    };
    // ========================================================================

    
    // ========================================================================
    /// evaluate the function 
    // ========================================================================
    double NumericalDefiniteIntegral::operator() 
      ( const Argument& argument ) const 
    {
      // reset the result and the error  
      m_result = GSL_NEGINF ;
      m_error  = GSL_POSINF ;
      
      // check the argument 
      if( argument.dimension() != m_DIM ) 
        { Exception ( "operator(): invalid argument size " ) ; };
      
      // copy the argument 
      
      for( size_t i  = 0 ; i < m_DIM ; ++i )
        { 
          m_argument [i] = argument [i] ;
          const size_t j =  i < m_index ? i : i + 1 ;
          m_argF     [j] = argument [i] ;
        };
      
      // create the helper object 
      GSL_Helper helper( *m_function , m_argF , m_index );
      
      // use GSL to evaluate the numerical derivative 
      gsl_function F ;
      F.function = &GSL_Adaptor ;
      F.params   = &helper                 ;
      _Function F1    ;
      F1.fn      = &F ;
      
      if        (  GaudiMath::Integration::Infinite         == category () )
        { return   QAGI ( &F1 ) ; }                                // RETURN
      
      if ( m_a == m_b ) 
        { 
          m_result = 0    ; m_error  = 0    ;                      // EXACT 
          return m_result ;                                       // RETURN 
        }
      
      if        (  GaudiMath::Integration::Singular         == category () )
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
    NumericalDefiniteIntegral::_Workspace* 
    NumericalDefiniteIntegral::allocate() const 
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
    double NumericalDefiniteIntegral::QAGI ( _Function* F ) const 
    {
      // check the argument 
      if ( 0 == F    ) { Exception("::QAGI: invalid function"); }
      
      // allocate workspace 
      if ( 0 == ws() ) { allocate() ; }
      
      int ierror = 0 ;
      
      if ( m_ia && m_ib ) 
        {
          ierror = gsl_integration_qagi  ( F->fn                , 
                                           m_epsabs  , m_epsrel , 
                                           size ()   , ws()->ws , 
                                           &m_result , &m_error ) ;
        }
      else if ( m_ia )
        {
          ierror = gsl_integration_qagil ( F->fn     , m_b      , 
                                           m_epsabs  , m_epsrel , 
                                           size ()   , ws()->ws , 
                                           &m_result , &m_error ) ;
        }
      else if ( m_ib )
        {
          ierror = gsl_integration_qagiu ( F->fn     , m_a      , 
                                           m_epsabs  , m_epsrel , 
                                           size ()   , ws()->ws , 
                                           &m_result , &m_error ) ;
        }
      else 
        { Exception ( "::QAGI: invalid mode" ) ; };
      
      if( ierror ) { gsl_error( "NumericalDefiniteIntegral::QAGI" , 
                                __FILE__ , __LINE__ , ierror ) ;}
      
      return m_result ;
    };
    // ========================================================================
    
    // ========================================================================
    // adaptive integration with known singular points 
    // ========================================================================
    double NumericalDefiniteIntegral::QAGP( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAGP::invalid function") ; }
      
      // no known singular points ?
      if( points().empty() || 0 == m_pdata ) { return QAGS( F ) ; }
      
      const size_t npts = points().size();
      
      // use GSL 
      int ierror =
        gsl_integration_qagp ( F->fn                , 
                               m_pdata   , npts     , 
                               m_epsabs  , m_epsrel ,
                               size ()   , ws()->ws , 
                               &m_result , &m_error ) ;
      
      if( ierror ) { gsl_error( "NumericalDefiniteIntegral::QAGI " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // the sign
      if ( m_a > m_b ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================
 
    // ========================================================================
    // non-adaptive integration 
    // ========================================================================
    double NumericalDefiniteIntegral::QNG ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QNG::invalid function") ; }
      
      // integration limits 
      const double low  = std::min ( m_a , m_b ) ;
      const double high = std::max ( m_a , m_b ) ;
      
      size_t neval = 0 ;
      int ierror = 
        gsl_integration_qng ( F->fn                 ,  
                              low       ,      high , 
                              m_epsabs  ,  m_epsrel , 
                              &m_result , &m_error  , &neval  ) ;
      
      if( ierror ) { gsl_error( "NumericalIndefiniteIntegral::QNG " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if ( m_a > m_b ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================
    
    
    // ========================================================================
    // simple adaptive integration 
    // ========================================================================
    double NumericalDefiniteIntegral::QAG ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAG::invalid function") ; }
      
      // allocate workspace 
      if( 0 == ws () ) { allocate () ; }
      
      // integration limits 
      const double low  = std::min ( m_a , m_b ) ;
      const double high = std::max ( m_a , m_b ) ;
      
      int ierror = 
        gsl_integration_qag ( F->fn                    , 
                              low       ,         high , 
                              m_epsabs  ,     m_epsrel , 
                              size   () , (int) rule() , ws ()->ws , 
                              &m_result ,     &m_error             );
      
      if( ierror ) { gsl_error( "NumericalDefiniteIntegral::QAG " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if ( m_a > m_b ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================
    
    // ========================================================================
    // adaptive integration with singularities 
    // ========================================================================
    double NumericalDefiniteIntegral::QAGS ( _Function* F ) const 
    {
      if( 0 == F ) { Exception("QAG::invalid function") ; }
      
      if ( m_a == m_b ) 
        { 
          m_result = 0    ;
          m_error  = 0    ;   // EXACT !
          return m_result ; 
        }
      
      // allocate workspace 
      if( 0 == ws () ) { allocate () ; }
      
      // integration limits 
      const double low  = std::min ( m_a , m_b ) ;
      const double high = std::max ( m_a , m_b ) ;
      
      int ierror = 
        gsl_integration_qags ( F->fn                , 
                               low       , high     , 
                               m_epsabs  , m_epsrel , 
                               size   () , ws()->ws , 
                               &m_result , &m_error );
      
      if( ierror ) { gsl_error( "NumericalDefiniteIntegral::QAGS " , 
                                __FILE__ , __LINE__ , ierror ) ; }
      
      // sign
      if ( m_a > m_b ) { m_result *= -1 ; }
      
      return m_result ;
    };
    // ========================================================================


  };  // end of namespace GaudiMathImplementation 
}; // end of namespace Genfun 



// ============================================================================
// The END 
// ============================================================================

// $Id: NumericalDerivative.cpp,v 1.1 2003/11/19 16:56:00 mato Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// local
// ============================================================================
#include "GaudiMath/NumericalDerivative.h"
#include "GaudiMath/Constant.h"
// ============================================================================
// GSL 
// ============================================================================
#include "gsl/gsl_errno.h"
#include "gsl/gsl_diff.h"
#include "gsl/gsl_math.h"
// ============================================================================
// local
// ============================================================================
#include "Helpers.h"
// ============================================================================

// ============================================================================
/** @file 
 *  Implementation file for class NumericalDerivative
 *
 *  @date  2003-08-27 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    // ========================================================================
    /// from CLHEP/GenericFunctions
    // ========================================================================
    FUNCTION_OBJECT_IMP( NumericalDerivative )
    // ========================================================================
    
    // ========================================================================
    /** The standard constructor from 
     *  AbsFunction and the index of the variable 
     *
     *  @param function the function
     *  @param index    the variable index 
     */
    // ========================================================================
    NumericalDerivative::NumericalDerivative
    ( const AbsFunction&               function , 
      const size_t                     index    , 
      const NumericalDerivative::Type& type     )
      : AbsFunction () 
      , m_function  ( function.clone()          )  
      , m_index     ( index                     ) 
      , m_DIM       ( function.dimensionality() )
      , m_type      ( type                      )
      , m_argument  ( function.dimensionality() )
      , m_result    ( GSL_NEGINF                )
      , m_error     ( GSL_POSINF                )
    {
      if( m_index >= m_DIM ) 
        { Exception ( "::constructor invalid variable index " ) ; };
    }
    // ========================================================================
    
    // ========================================================================
    /// copy constructor 
    // ========================================================================
    NumericalDerivative::NumericalDerivative 
    ( const NumericalDerivative&   right    ) 
      : AbsFunction () 
      , m_function  ( right.m_function->clone() )  
      , m_index     ( right.m_index             ) 
      , m_DIM       ( right.m_DIM               )
      , m_type      ( right.m_type              ) 
      , m_argument  ( right.m_DIM               )
      , m_result    ( GSL_NEGINF                )
      , m_error     ( GSL_POSINF                )
    {}
    // ========================================================================
    
    // ========================================================================
    /// destructor 
    // ========================================================================
    NumericalDerivative::~NumericalDerivative()
    { if( 0 != m_function ) { delete m_function ; m_function = 0 ; } }
    // ========================================================================
    
    // ========================================================================
    /// change the type of the adaptive differentiation
    // ========================================================================
    const NumericalDerivative::Type&
    NumericalDerivative::setType  ( const NumericalDerivative::Type& value ) 
    { m_type = value ; return type() ; }
    // ========================================================================
    
    // ========================================================================
    /// Derivatives 
    // ========================================================================
    Genfun::Derivative NumericalDerivative::partial( unsigned int idx ) const 
    {
      if( idx >= m_DIM ) 
        { Exception ( "::partial(i): invalid variable index" ) ; }
      const  AbsFunction& aux = 
        NumericalDerivative( *this , idx , type() ) ;
      return FunctionNoop( &aux ) ;
    }
    // ========================================================================
    
    // ========================================================================
    /// Function value
    // ========================================================================
    double NumericalDerivative::operator() 
      ( const Argument& argument ) const
    {
      // reset the result and the error  
      m_result = GSL_NEGINF ;
      m_error  = GSL_POSINF ;
      
      // check the argument 
      if( argument.dimension() != m_DIM ) 
        { Exception ( "::operator():invalid argument size" ) ; };
      
      /// copy the argument 
      {for( size_t i  = 0 ; i < m_DIM ; ++i ){ m_argument[i] = argument[i];}}
      
      // create the helper object 
      GSL_Helper helper( *m_function , m_argument , m_index );
      
      /// use GSL to evaluate the numerical derivative 
      gsl_function F ;
      F.function = &GSL_Adaptor ;
      F.params   = &helper                 ;
      
      double x       = argument[m_index];
      int    ierrno  = 0                   ;
      switch ( type() ) 
        {
        case Central  : 
          ierrno = gsl_diff_central  ( &F , x , &m_result , &m_error ) ; break ;
        case Forward  : 
          ierrno = gsl_diff_forward  ( &F , x , &m_result , &m_error ) ; break ;
        case Backward :
          ierrno = gsl_diff_backward ( &F , x , &m_result , &m_error ) ; break ;
        default:
          Exception ( "::operator(): invalid diffrentiation type " ) ;          
        }
      
      if( ierrno ) 
        { gsl_error ( " NumericalDerivative:: the error from gsl_diff_XXXX" ,
                      __FILE__ , __LINE__ , ierrno ) ; }
      
      return result() ;
    }
    // ========================================================================
    
    // ========================================================================
    /// Function value
    // ========================================================================
    double NumericalDerivative::operator() ( const double  argument ) const
    {
      // reset the result and the error  
      m_result = GSL_NEGINF ;
      m_error  = GSL_POSINF ;
      
      // check the argument 
      if( 1 != m_DIM ) { Exception ( "operator(): invalid argument size " ); }

      Argument arg(1) ;
      arg[0] = argument ;
      return (*this)( arg ) ;
    }
    // ========================================================================
    
    // ========================================================================
    /// Exception
    // ========================================================================
    StatusCode NumericalDerivative::Exception 
    ( const std::string& message ,  
      const StatusCode&  sc      ) const 
    {
      throw GaudiException( "NumericalDerivative" + message , 
                            "*GaudiMath*"         , sc      ) ;
      return sc ;
    }
    // ========================================================================
    
 
  } // end of namespace GaudiMathImplementation 

} // end of namespace Genfun (due to GSL) 
 
// ============================================================================
// The END 
// ============================================================================

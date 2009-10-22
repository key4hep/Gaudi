// $Id: Constant.cpp,v 1.1 2003/11/19 16:55:59 mato Exp $
// ============================================================================
// Include files
// local
#include "GaudiMath/Constant.h"

/** @file 
 *  Implementation file for class GSL_Constant
 *
 *  @date  2003-08-27 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    // ========================================================================
    /// from CLHGEP/GenericFunctions
    // ========================================================================
    FUNCTION_OBJECT_IMP( Constant );
    // ========================================================================
    
    // ========================================================================
    /** Standard constructor
     *  @param value of the function
     *  @param dim   dimensionality 
     */
    // ========================================================================
    Constant::Constant 
    ( const double value , 
      const size_t dim   ) 
      : AbsFunction () 
      , m_value     ( value ) 
      , m_DIM       ( dim   ) 
    {};
    // ========================================================================
    
    // ========================================================================
    /// copy constructor 
    // ========================================================================
    Constant::Constant 
    ( const Constant& right ) 
      : AbsFunction() 
      , m_value ( right.m_value ) 
      , m_DIM   ( right.m_DIM   )
    {};
    // ========================================================================
    
    // ========================================================================
    /// destructor 
    // ========================================================================
    Constant::~Constant(){}
    // ========================================================================
    
    // ========================================================================
    /// Derivatives 
    // ========================================================================
    Derivative Constant::partial ( unsigned int /* index */ ) const
    {
      const AbsFunction& aux = Constant( 0.0 , m_DIM );
      return FunctionNoop( &aux );
    };
    // ========================================================================
    
    
  }; // end of namespace GaudiMathImplementation
  
}; // end of namespace Genfun (due to CLHEP)

// ============================================================================
// The END 
// ============================================================================

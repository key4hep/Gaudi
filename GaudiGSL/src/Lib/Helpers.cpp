// $Id: Helpers.cpp,v 1.1 2003/11/19 16:55:59 mato Exp $
// ============================================================================
// Include files
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================
// local
// ============================================================================
#include "Helpers.h"
// ============================================================================

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    
    // ========================================================================
    GSL_Helper::GSL_Helper
    ( const Genfun::AbsFunction& function , 
      Genfun::Argument&          argument ,
      const size_t               index    )
      : m_function ( function ) , 
        m_argument ( argument ) , 
        m_index    ( index    ) 
    {};
    // ========================================================================
    
    // ========================================================================
    GSL_Helper::~GSL_Helper(){};
    // ========================================================================
    
    // ========================================================================
    double GSL_Adaptor( double x , void* params )
    {
      // get parameters 
      GSL_Helper* aux = (GSL_Helper*) params ;
      // decode parameters 
      const Genfun::AbsFunction& function = aux -> m_function  ;
      const size_t               index    = aux -> m_index     ;
      Genfun::Argument&          argument = aux -> m_argument  ;
      // save the current value of argument 
      const double x_prev = argument[index] ;
      // modify the argument 
      argument[index]= x       ; 
      // evaluate the function
      const double fun = function( argument ) ;
      // restore the argument 
      argument[index] = x_prev  ;
      return fun ;
    };
    // ========================================================================
    
  }; // end of namespace GaudiMathImplementation
}; // end of namespace Genfun

// ============================================================================
// The END 
// ============================================================================


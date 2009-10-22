// $Id: GSLAdaptersTest.cpp,v 1.2 2005/11/25 10:27:03 mato Exp $
// ============================================================================
// Include file
// ============================================================================
// STD & STL 
// ============================================================================
#include <stdio.h>
#include <iostream>
// ============================================================================
// GaudiGSL/GaudiMath
// ============================================================================
#include "GaudiMath/GaudiMath.h"
// ============================================================================
// GSL 
// ============================================================================
#include "gsl/gsl_sf_bessel.h"
// ============================================================================

/** @file 
 *  test file for GSLFunAdapters 
 *  @date 2004-03-06 
 *  @authro Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

int main()
{

  /// regular bessel function I0 with simplest adaptor 
  const GaudiMath::Function& I0 = GaudiMath::adapter( gsl_sf_bessel_I0   ) ;
  /// regular bessel function I1 with 'result' frm of adaptor  
  const GaudiMath::Function& I1 = GaudiMath::adapter( gsl_sf_bessel_I1_e ) ;
  
  for( int i = 0 ; i < 20 ; ++i )
  {
    const double x  = double(i) * 1 / 20.0  ;
    const double x0 = I0( x ) ;
    const double x1 = I1( x ) ;    
    printf( " x = %+8.4f I0=%15.8f I1=%15.8f \n" , x , x0 , x1 );
  };
  
  
  exit(0);
};


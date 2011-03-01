// $Id: IntegralInTest.cpp,v 1.2 2006/12/08 12:15:24 hmd Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <math.h>
#include <stdio.h>
// ============================================================================
// GAUDI
// ============================================================================
#include "GaudiMath/GaudiMath.h"
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/Sin.hh"
#include "CLHEP/GenericFunctions/Cos.hh"
// ============================================================================
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

// ============================================================================
/** @file
 *
 *  Test file for the class NumericalIndefiniteIntregral
 *
 *  @date 2003-08-31
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

int main()
{

  std::cout <<
    " Test for numerical differentiation of Genfun::Cos(x) " << std::endl ;

  const GaudiMath::IndIntegral& mysin =
    GaudiMath::IndIntegral( Genfun::Cos() , 0 , 0.0 ) ;

  for( double x = -90 * Gaudi::Units::degree ;
       x <= 180 * Gaudi::Units::degree ;
       x += 10 * Gaudi::Units::degree ) {
    double value = mysin ( x ) ;
    double error = mysin.error() ;
    printf( "x=%8.3f deg; I(Cos)=%+.19f; ActErr=%+.19f; EstErr=%+.19f;\n",
	    x / Gaudi::Units::degree  , value , value-sin(x)  , error  );
  }

  exit(0);
}

// ============================================================================
// The END
// ============================================================================

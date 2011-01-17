// $Id: Integral1Test.cpp,v 1.2 2006/12/08 12:15:22 hmd Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <math.h>
#include <stdio.h>
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Integral.h"
#include "GaudiMath/GaudiMath.h"
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/Sin.hh"
#include "CLHEP/GenericFunctions/Cos.hh"
#include "CLHEP/GenericFunctions/Gaussian.hh"
// ============================================================================
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

// ============================================================================
/** @file
 *
 *  Test file for the class NumericalDefiniteIntregral  and Integral
 *
 *  @date 2003-08-31
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

int main()
{

  std::cout <<
    " Test for numerical integration of Genfun::Sin(x) " << std::endl ;

  const GaudiMath::Function& mysin = Genfun::Sin();


  for( double x = 0 * Gaudi::Units::degree ;
       x <= 360 * Gaudi::Units::degree ;
       x += 15 * Gaudi::Units::degree ) {
    double value = GaudiMath::Integral ( mysin , 0. , x ) ;
    printf( "x=%8.3f deg; I(Sin)=%+.19f; ActErr=%+.19f;\n",
	    x / Gaudi::Units::degree  , value , value-(1-cos(x)) );
  }


  const GaudiMath::Function& myfun = Genfun::Gaussian();

  std::cout
    << " integral from gauss1 =  "
    << GaudiMath::Integral( myfun , 0.0 , GaudiMath::Integration::Infinity )
    << std::endl << " integral from gauss2 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity ,0.0  )
    << std::endl << " integral from gauss3 =  "
    << GaudiMath::Integral( myfun                             ,
                            GaudiMath::Integration::Infinity  ,
                            GaudiMath::Integration::Infinity  )
    << std::endl << " integral from gauss4 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity , 1.0  )
    << std::endl << " integral from gauss5 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity , 4.0  )
    << std::endl << " integral from gauss6 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity , 9.0  )
    << std::endl << " integral from gauss7 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity , 16.0  )
    << std::endl << " integral from gauss8 =  "
    << GaudiMath::Integral( myfun , GaudiMath::Integration::Infinity , 25.0  )
    << std::endl ;


  exit(0);
}

// ============================================================================
// The END
// ============================================================================

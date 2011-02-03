// $Id: ExceptionsTest.cpp,v 1.2 2006/12/08 12:15:20 hmd Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <exception>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/GaudiMath.h"
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/Sin.hh"
// ============================================================================
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

// ============================================================================
/** @file
 *
 *  Test file for the class NumericalDerivative
 *
 *  @date 2003-08-31
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

int main()
{

  std::cout <<
    " Test for embedded exceptions ( 2 exception shoudl be catched) "
            << std::endl ;

  try {
    const GaudiMath::Function&   mysin  = Genfun::Sin();
    // the exception!!! showl be thrown!
    const GaudiMath::Function&   prim   = GaudiMath::Derivative( mysin , 5 );
    std::cout << "One should never see this line!! " << std::endl ;
    mysin + prim ;
  }
  catch( const std::exception& e )
    {
      std::cout << " OK 1) The exception is catched with 'what'='"
                << e.what() << "'" << std::endl;
    }

  try {
    const GaudiMath::Function&   mysin  = Genfun::Sin();
    // the exception!!! showl be thrown!
    const GaudiMath::Function&   prim   = GaudiMath::Derivative( mysin , 10 );
    std::cout << "One should never see this line!! " << std::endl ;
    mysin + prim ;
  }
  catch( const GaudiException& e )
    {
      std::cout << " OK 2) " << e << std::endl ;
    }

  exit(0);

}

// ============================================================================
// The END
// ============================================================================

// $Id: PFuncTest.cpp,v 1.5 2006/12/08 12:15:24 hmd Exp $
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <math.h>
#include <stdio.h>
#include <vector>
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/GaudiMath.h"
#include "GaudiMath/Adapters.h"
// ============================================================================

// ============================================================================
/** @file 
 *  
 *  Test file for the class AdapterPFunction
 * 
 *  @date 2003-09-03 
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 */
// ============================================================================

double fun ( const std::vector<double>& x)
{
  return 3 * x[0] * x[0] + 5 * x[1] * x[1] + 8;  
}

int main()
{
  const GaudiMath::Function& myfun = GaudiMath::adapter( 2 , &fun ) ;
  Genfun::Argument arg(myfun.dimensionality());
  
  std::string format( "x[0]=%6.5f x[1]=%6.5f myfun=%+.10f");
  format +=" fun=%+.10f fun-myfun=%+.10f \n" ;
  std::vector<double> x = 
    std::vector<double>(myfun.dimensionality());

  for ( double a = 0; a < 3; ++a)
    {
      for (double b = 0; b < 3; ++b)
        {
          arg[0] = x[0] = a;
          arg[1] = x[1] = b;
          
          printf ( format.c_str() ,
                   x[0], x[1], myfun(arg), fun(x), 
                   fun(x) - myfun(arg) );
        }
    }
  
  exit(0);
  
}


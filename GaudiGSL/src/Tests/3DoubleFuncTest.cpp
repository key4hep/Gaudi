// $Id: 3DoubleFuncTest.cpp,v 1.3 2005/11/25 10:27:03 mato Exp $
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
#include "GaudiMath/GaudiMath.h"
#include "GaudiMath/Adapters.h"
// ============================================================================

// ============================================================================
/** @file 
 *  
 *  Test file for the class Adapter3DoubleFunction
 * 
 *  @date 2003-09-03 
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 */
// ============================================================================

double sum ( double x, double y, double z )
{
  return 3 * x * x - 5 * y * y + 3 * z * z;  
}

int main()
{
  const GaudiMath::Function& mysum = GaudiMath::adapter(sum);
  Genfun::Argument arg(3);
  
  std::string format( "x=%6.5f y=%6.5f z=%6.5f mysum=%+.10f");
  format +=" sum=%+.10f sum-mysum=%+.10f \n" ;
  
  for ( double x = 0; x < 3; ++x)
    {
      for (double y = 0; y < 3; ++y)
        {
          for (double z = 0; z < 3; ++z)
            {
              arg[0] = x;
              arg[1] = y;
              arg[2] = z;
              printf ( format.c_str() ,
                       x, y, z, mysum(arg), sum(x, y, z), 
                       sum(x, y, z) - mysum(arg) );
            } 
        }
    }
  
  exit(0);
  
};


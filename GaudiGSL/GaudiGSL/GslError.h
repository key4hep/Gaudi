#ifndef GAUDIGSL_GSLERROR_H
#define GAUDIGSL_GSLERROR_H 1
// Include files
#include "GaudiKernel/Kernel.h"
// STD & STL
#include <string>

/** @struct GslError GslError.h
 *
 *  Helper class to represent GSL errors
 *
 *  @author Vanya Belyaev Ivan.Belyaev
 *  @date   29/04/2002
 */
class GAUDI_API GslError
{
public:
  /// error message ('reason')
  std::string reason ;
  /// file name
  std::string file   ;
  /// line number
  int         line   ;
  /// error code (GSL)
  int         code   ;
  /// constructor
  GslError( std::string  r = "" ,
            std::string  f = "" ,
            const int    l = 0  ,
            const int    c = 0  )
    : reason ( std::move(r) ) , file   ( std::move(f) ) , line   ( l ) , code   ( c ) {};
  /// comparison (ordering) criteria
  bool operator<( const GslError& right ) const
  {
    return
      code         < right.code   ? true  :
      right.code   <       code   ? false :
      reason       < right.reason ? true  :
      right.reason <       reason ? false :
      file         < right.file   ? true  :
      right.file   <       file   ? false : line < right.line ;
  };
};

// ============================================================================
// The END
// ============================================================================
#endif // GSLERROR_H
// ============================================================================

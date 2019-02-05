#ifndef GAUDIGSL_IFUNCMINIMUM_H
#define GAUDIGSL_IFUNCMINIMUM_H 1
// Include files
// from STL
#include <string>
#include <vector>
// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// forward declarations
namespace Genfun /// CLHEP
{
  class AbsFunction;
  class Argument;
} // namespace Genfun
namespace CLHEP {
  class HepSymMatrix; /// CLHEP
}

/** @class IFuncMinimum IFuncMinimum.h GaudiGSL/IFuncMinimum.h
 *  Interface file to find minimum of the function
 *  and compute covariance matrix (matrix of error)
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date   2002-09-14
 */

struct GAUDI_API IFuncMinimum : extend_interfaces<IAlgTool> {
  /// InterfaceID
  DeclareInterfaceID( IFuncMinimum, 3, 0 );

  /// Function which we minimize (@see CLHEP/GenericFunctions/AbsFunction.hh)
  typedef Genfun::AbsFunction GenFunc;
  /// Argument of function "GenFunc" (@see CLHEP/GenericFunctions/Argument.hh)
  typedef Genfun::Argument Arg;
  /// Covariance matrix (matrix of error) (@see CLHEP/Matrix/SymMatrix.h)
  typedef CLHEP::HepSymMatrix Covariance;

  /// Do Multidimensional Minimization

  /** Find minimum and gradient of the function "GenFunc"
   *  @param fun   - function @see CLHEP/GenericFunctions/AbsFunction.hh
   *  @param pars  - argument @see CLHEP/GenericFunctions/Argument.hh
   *  @return StatusCode
   */
  virtual StatusCode minimum( const GenFunc& fun, Arg& pars ) const = 0;

  /** Find minimum and gradient of the function "GenFunc"
   *  @param fun   - function @see CLHEP/GenericFunctions/AbsFunction.hh
   *  @param pars  - argument @see CLHEP/GenericFunctions/Argument.hh
   *  @param covar - covariance matrix (matrix of error)
   *  @see CLHEP/Matrix/SymMatrix.h
   *  @return StatusCode
   */
  virtual StatusCode minimum( const GenFunc& fun, Arg& pars, Covariance& covar ) const = 0;
};
#endif // GAUDIGSL_IFUNCMINIMUM_H

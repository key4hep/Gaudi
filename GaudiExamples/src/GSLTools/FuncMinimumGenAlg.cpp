// Include files
// from Gaudi
#include "GaudiGSL/IFuncMinimum.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiMath/Adapters.h"
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/Matrix/SymMatrix.h"
// local
#include "FuncMinimumGenAlg.h"

using namespace Genfun;
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP {}
using namespace CLHEP;

//-----------------------------------------------------------------------------
/** @file Implementation file for class : FuncMinimumGenAlg
 *  @see FuncMinimumGenAlg.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2002-09-14
 */
//-----------------------------------------------------------------------------

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

//=============================================================================
// Class for the function "GenFunc"
// @see GaudiGSL/IFuncMinimum.h
class Func : public AbsFunction

{
public:
  FUNCTION_OBJECT_DEF( Func )
public:
  Func() = default;
  Func( const Func& ) : AbsFunction() {}
  double operator()( double /* argument */ ) const override { return 0; }
  double operator()( const Argument& x ) const override { return 10 + 4 * x[0] * x[0] + 27 * x[1] * x[1] + 25 * x[1]; }
  unsigned int dimensionality() const override { return 2; }
};
FUNCTION_OBJECT_IMP( Func )

#ifdef __clang__
#  pragma clang diagnostic pop
#endif

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode FuncMinimumGenAlg::initialize() {

  info() << "==> Initialise" << endmsg;

  StatusCode sc;
  sc = toolSvc()->retrieveTool( "FuncMinimum", m_publicTool );
  if ( sc.isFailure() ) { error() << "Error retrieving the public tool" << endmsg; }
  sc = toolSvc()->retrieveTool( "FuncMinimum", m_privateTool, this );
  if ( sc.isFailure() ) { error() << "Error retrieving the private tool" << endmsg; }
  info() << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode FuncMinimumGenAlg::execute() {

  info() << "==> Execute" << endmsg;

  // the object of the class Func
  Func func;

  //=============================================================================
  // Input number and value of the arguments of the function "GenFunc"
  IFuncMinimum::Arg arg( func.dimensionality() );

  arg[0] = 5;
  arg[1] = 10;

  // Matrix of error
  IFuncMinimum::Covariance matrix_error( arg.dimension(), 0 );

  // Call of the method
  m_publicTool->minimum( func, arg );
  info() << endmsg;
  info() << "START OF THR METHOD" << endmsg;
  info() << "MINIMUM FOUND AT: " << endmsg;

  for ( unsigned int i = 0; i < arg.dimension(); i++ ) {

    info() << "Value of argument " << i << " is " << arg[i] << endmsg;
  }

  info() << endmsg;

  //=============================================================================
  // With Covariance matrix (matrix of error)
  arg[0] = 5;
  arg[1] = 10;

  // Call of the method(with covariance matrix (matrix of error))
  m_publicTool->minimum( func, arg, matrix_error );
  info() << endmsg;
  info() << "THE METHOD WITH MATRIX OF ERROR" << endmsg;
  info() << "MINIMUM FOUND AT: " << endmsg;

  for ( unsigned int i = 0; i < arg.dimension(); i++ ) {

    info() << "Value of argument " << i << " is " << arg[i] << endmsg;
  }

  info() << endmsg;
  info() << "MATRIX OF ERROR";

  for ( unsigned int i = 0; i < arg.dimension(); i++ ) {
    info() << endmsg;

    for ( unsigned int j = 0; j < arg.dimension(); j++ ) { info() << matrix_error( i + 1, j + 1 ) << " "; }
  }
  info() << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode FuncMinimumGenAlg::finalize() {

  info() << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Declaration of the Algorithm Factory
DECLARE_COMPONENT( FuncMinimumGenAlg )

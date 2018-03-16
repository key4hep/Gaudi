// Include files

// from Gaudi
#include "GaudiGSL/IEqSolver.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiMath/Adapters.h"
// local
#include "EqSolverGenAlg.h"

//-----------------------------------------------------------------------------
/** @file Implementation file for class : EqSolverGenAlg
 *  @see EqSolverGenAlg.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2003-07-07
 */
//-----------------------------------------------------------------------------

using namespace Genfun;

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( EqSolverGenAlg )

typedef Genfun::AbsFunction GenFunc;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

// Class for the function "GenFunc"
// @see GaudiGSL/IEqSolver.h
class Func1 : public AbsFunction
{
public:
  FUNCTION_OBJECT_DEF( Func1 )
public:
  Func1() {}
  Func1( const Func1& ) : AbsFunction() {}

  double operator()( double /* argument */ ) const override { return 0; }
  double operator()( const Argument& x ) const override { return x[0] - 1; };
  unsigned int dimensionality() const override { return 3; }
};
FUNCTION_OBJECT_IMP( Func1 )

// Class for the function "GenFunc" @see IEqSolver.h
class Func2 : public AbsFunction
{
public:
  FUNCTION_OBJECT_DEF( Func2 )
public:
  Func2() {}
  Func2( const Func2& ) : AbsFunction() {}

  double operator()( double /* argument */ ) const override { return 0; }
  double operator()( const Argument& x ) const override { return x[1] - 1; };
  unsigned int dimensionality() const override { return 3; }
};
FUNCTION_OBJECT_IMP( Func2 )

// Class for the function "GenFunc" @see IEqSolver.h
class Func3 : public AbsFunction
{
public:
  FUNCTION_OBJECT_DEF( Func3 )
public:
  Func3() {}
  Func3( const Func3& ) : AbsFunction() {}

  double operator()( double /* argument */ ) const override { return 0; }
  double operator()( const Argument& x ) const override { return x[2] - 1; };
  unsigned int dimensionality() const override { return 3; }
};
FUNCTION_OBJECT_IMP( Func3 )

#ifdef __clang__
#pragma clang diagnostic pop
#endif

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode EqSolverGenAlg::initialize()
{
  info() << "==> Initialise" << endmsg;

  StatusCode sc;
  sc = toolSvc()->retrieveTool( "EqSolver", m_publicTool );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the public tool" << endmsg;
  }
  sc = toolSvc()->retrieveTool( "EqSolver", m_privateTool, this );
  if ( sc.isFailure() ) {
    error() << "Error retrieving the private tool" << endmsg;
  }
  info() << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode EqSolverGenAlg::execute()
{

  info() << "==> Execute" << endmsg;

  std::vector<const GenFunc*> function;

  function.push_back( new Func1() );
  function.push_back( new Func2() );
  function.push_back( new Func3() );

  //=============================================================================

  // Input number and value of the arguments of the function "GenFunc"
  IEqSolver::Arg arg( function.size() );

  arg[0] = 10;
  arg[1] = 5;
  arg[2] = 29;

  // Call of the method
  m_publicTool->solver( function, arg );
  info() << endmsg;
  info() << "START OF THE METHOD" << endmsg;
  info() << "SOLUTION FOUND AT: " << endmsg;

  for ( unsigned int i = 0; i < arg.dimension(); i++ ) {
    info() << "Value of argument " << i << " is " << arg[i] << endmsg;
  }
  info() << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode EqSolverGenAlg::finalize()
{

  info() << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================

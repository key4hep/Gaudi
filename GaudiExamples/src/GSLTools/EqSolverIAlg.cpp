// Include files

// from Gaudi
#include "GaudiGSL/IEqSolver.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiMath/Adapters.h"
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
// from AIDA
#include "AIDA/IFunction.h"
// local
#include "EqSolverIAlg.h"

// disable warning about nullptr dereferencing (icc and clang)
//   It's a real problem, but here it is used for test purposes
#ifdef __ICC
#pragma warning( disable : 327 )
#elif __clang__
#pragma clang diagnostic ignored "-Wnull-dereference"
#endif

//-----------------------------------------------------------------------------
/** @file Implementation file for class : EqSolverIAlg
 *  @see EqSolverIAlg.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2003-07-07
 */
//-----------------------------------------------------------------------------

using namespace Genfun;

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP
{
}
using namespace CLHEP;

typedef Genfun::AbsFunction GenFunc;

// Class for the function "IFunction"
// @see AIDA/IFunction.h
class Function1 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument;

public:
  Function1() = default;
  double value( const argument& x ) const override { return x[0] - 1; }
  int                           dimension() const override { return 3; }
  bool                          setTitle( const std::string& ) override { return false; }
  std::string                   title( void ) const override { return ""; }
  bool                          isEqual( const AIDA::IFunction& ) const override { return false; }
  const std::vector<double>&    gradient( const std::vector<double>& ) const override { return m_values; }
  bool                          providesGradient( void ) const override { return false; }
  std::string                   variableName( int ) const override { return ""; }
  std::vector<std::string>      variableNames( void ) const override { return m_names; }
  bool                          setParameters( const std::vector<double>& ) override { return true; }
  const std::vector<double>&    parameters( void ) const override { return m_values; }
  int                           numberOfParameters( void ) const override { return 0; }
  std::vector<std::string>      parameterNames() const override { return m_names; }
  bool                          setParameter( const std::string&, double ) override { return false; }
  double                        parameter( const std::string& ) const override { return 0.0; }
  int                           indexOfParameter( const std::string& ) const override { return 0; }
  const AIDA::IAnnotation&      annotation( void ) const override { return *(AIDA::IAnnotation*)0; }
  AIDA::IAnnotation&            annotation( void ) override { return *(AIDA::IAnnotation*)0; }
  std::string                   codeletString( void ) const override { return ""; }
  void*                         cast( const std::string& ) const override { return 0; }

private:
  std::string              m_version;
  std::string              m_label;
  std::vector<std::string> m_names;
  std::vector<double>      m_values;
};

// Class for the function "IFunction"
class Function2 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument;

public:
  Function2() = default;
  double value( const argument& x ) const override { return x[1] - 1; }
  int                           dimension() const override { return 3; }
  bool                          setTitle( const std::string& ) override { return false; }
  std::string                   title( void ) const override { return ""; }
  bool                          isEqual( const AIDA::IFunction& ) const override { return false; }
  const std::vector<double>&    gradient( const std::vector<double>& ) const override { return m_values; }
  bool                          providesGradient( void ) const override { return false; }
  std::string                   variableName( int ) const override { return ""; }
  std::vector<std::string>      variableNames( void ) const override { return m_names; }
  bool                          setParameters( const std::vector<double>& ) override { return true; }
  const std::vector<double>&    parameters( void ) const override { return m_values; }
  int                           numberOfParameters( void ) const override { return 0; }
  std::vector<std::string>      parameterNames() const override { return m_names; }
  bool                          setParameter( const std::string&, double ) override { return false; }
  double                        parameter( const std::string& ) const override { return 0.0; }
  int                           indexOfParameter( const std::string& ) const override { return 0; }
  const AIDA::IAnnotation&      annotation( void ) const override { return *(AIDA::IAnnotation*)0; }
  AIDA::IAnnotation&            annotation( void ) override { return *(AIDA::IAnnotation*)0; }
  std::string                   codeletString( void ) const override { return ""; }
  void*                         cast( const std::string& ) const override { return 0; }

private:
  std::string              m_version;
  std::string              m_label;
  std::vector<std::string> m_names;
  std::vector<double>      m_values;
};

// Class for the function "IFunction"
class Function3 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument;

public:
  Function3() = default;
  double value( const argument& x ) const override { return x[2] - 1; }
  int                           dimension() const override { return 3; }
  bool                          setTitle( const std::string& ) override { return false; }
  std::string                   title( void ) const override { return ""; }
  bool                          isEqual( const AIDA::IFunction& ) const override { return false; }
  const std::vector<double>&    gradient( const std::vector<double>& ) const override { return m_values; }
  bool                          providesGradient( void ) const override { return false; }
  std::string                   variableName( int ) const override { return ""; }
  std::vector<std::string>      variableNames( void ) const override { return m_names; }
  bool                          setParameters( const std::vector<double>& ) override { return true; }
  const std::vector<double>&    parameters( void ) const override { return m_values; }
  int                           numberOfParameters( void ) const override { return 0; }
  std::vector<std::string>      parameterNames() const override { return m_names; }
  bool                          setParameter( const std::string&, double ) override { return false; }
  double                        parameter( const std::string& ) const override { return 0.0; }
  int                           indexOfParameter( const std::string& ) const override { return 0; }
  const AIDA::IAnnotation&      annotation( void ) const override { return *(AIDA::IAnnotation*)0; }
  AIDA::IAnnotation&            annotation( void ) override { return *(AIDA::IAnnotation*)0; }
  std::string                   codeletString( void ) const override { return ""; }
  void*                         cast( const std::string& ) const override { return 0; }

private:
  std::string              m_version;
  std::string              m_label;
  std::vector<std::string> m_names;
  std::vector<double>      m_values;
};

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode EqSolverIAlg::initialize()
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
StatusCode EqSolverIAlg::execute()
{

  info() << "==> Execute" << endmsg;

  // the objects of IFunction's classes
  const Function1 fun1{};
  const Function2 fun2{};
  const Function3 fun3{};

  // the objects of the class AdapterIFunction
  // @see Adapter.h
  const GaudiMath::AIDAFunction& adap1 = GaudiMath::adapter( fun1 );
  const GaudiMath::AIDAFunction& adap2 = GaudiMath::adapter( fun2 );
  const GaudiMath::AIDAFunction& adap3 = GaudiMath::adapter( fun3 );

  std::vector<const GenFunc*> function = {&adap1, &adap2, &adap3};

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
StatusCode EqSolverIAlg::finalize()
{

  info() << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Declaration of the Algorithm Factory
DECLARE_COMPONENT( EqSolverIAlg )

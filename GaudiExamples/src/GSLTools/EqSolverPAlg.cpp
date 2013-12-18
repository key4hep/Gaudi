// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiGSL/IEqSolver.h"
#include "GaudiMath/GaudiMath.h"
#include "GaudiMath/Adapters.h"
// local
#include "EqSolverPAlg.h"

//-----------------------------------------------------------------------------
/** @file Implementation file for class : EqSolverPAlg
 *  @see EqSolverPAlg.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2003-07-07
 */
//-----------------------------------------------------------------------------

using namespace Genfun;

// Declaration of the Algorithm Factory
DECLARE_COMPONENT(EqSolverPAlg)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
EqSolverPAlg::EqSolverPAlg( const std::string& name,
			ISvcLocator* pSvcLocator)
  : Algorithm ( name , pSvcLocator ) {

}

//=============================================================================
// Destructor
//=============================================================================
EqSolverPAlg::~EqSolverPAlg() {}

//=============================================================================
typedef Genfun::AbsFunction GenFunc;

// Our functions
double function1 ( const std::vector<double>& x  )
{
  return x[0] * x[0] + x[1] * x[1]- 1;
}

double function2 ( const std::vector<double>& x  )
{
  return x[0] + x[1] ;
}

double function3 ( const std::vector<double>& x  )
{
  return x[2] * x[2] * x[2] * x[1] - 120 ;
}

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode EqSolverPAlg::initialize() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "==> Initialise" << endmsg;

  StatusCode sc;
  sc = toolSvc()->retrieveTool("EqSolver", m_publicTool );
  if( sc.isFailure() )
    {
      log << MSG::ERROR<< "Error retrieving the public tool" << endmsg;
    }
  sc = toolSvc()->retrieveTool("EqSolver", m_privateTool, this );
  if( sc.isFailure() )
    {
      log << MSG::ERROR<< "Error retrieving the private tool" << endmsg;
    }
  log << MSG::INFO << "....initialization done" << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode EqSolverPAlg::execute() {

  MsgStream  log( msgSvc(), name() );
  log << MSG::INFO << "==> Execute" << endmsg;

  // the objects of the class AdapterPFunction
  // @see Adapters.h
  const GaudiMath::Function& adap1 = GaudiMath::adapter( 3 , &function1 ) ;
  const GaudiMath::Function& adap2 = GaudiMath::adapter( 3 , &function2 ) ;
  const GaudiMath::Function& adap3 = GaudiMath::adapter( 3 , &function3 ) ;

  std::vector<const GenFunc*> function;

  function.push_back(&adap1);
  function.push_back(&adap2);
  function.push_back(&adap3);

//=============================================================================

  // Input number and value of the arguments of the function "GenFunc"
  IEqSolver::Arg arg (function.size ());

  arg[0] = 2;
  arg[1] = 3;
  arg[2] = 5;

  // Call of the method
  m_publicTool->solver( function ,
                        arg      );
  log << endmsg;
  log << "START OF THE METHOD" << endmsg;
  log << "SOLUTION FOUND AT: " << endmsg;

  for (unsigned int i = 0; i < arg.dimension(); i++)
    {
      log << "Value of argument " << i <<" is " << arg[i] << endmsg;
    }
  log << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode EqSolverPAlg::finalize() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool  );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================

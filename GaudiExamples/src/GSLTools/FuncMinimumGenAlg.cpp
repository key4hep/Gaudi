// $Id: FuncMinimumGenAlg.cpp,v 1.3 2006/01/10 19:58:26 hmd Exp $

// Include files
// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiGSL/IFuncMinimum.h"
#include "GaudiMath/Adapters.h"
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/Matrix/SymMatrix.h"
// local
#include "FuncMinimumGenAlg.h"

using namespace Genfun;
// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

//-----------------------------------------------------------------------------
/** @file Implementation file for class : FuncMinimumGenAlg
 *  @see FuncMinimumGenAlg.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2002-09-14
 */
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FuncMinimumGenAlg::FuncMinimumGenAlg( const std::string& name,
                                ISvcLocator* pSvcLocator)
  : Algorithm ( name , pSvcLocator ) {

}

//=============================================================================
// Destructor
//=============================================================================
FuncMinimumGenAlg::~FuncMinimumGenAlg() {}

//=============================================================================
// Class for the function "GenFunc"
// @see GaudiGSL/IFuncMinimum.h
class Func : public AbsFunction

{
public:
  FUNCTION_OBJECT_DEF(Func)
public:
  Func () {};
  Func ( const Func&  )
    : AbsFunction() {};
  virtual ~Func () {};
  virtual double operator() (double /* argument */) const {return 0;};
  virtual double operator() (const Argument& x) const
  {
    return 10 + 4 * x[0] * x[0] + 27 * x[1] * x[1]
      + 25 * x[1];
  }
  virtual unsigned int dimensionality () const {return 2;}
};
FUNCTION_OBJECT_IMP(Func)

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode FuncMinimumGenAlg::initialize() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "==> Initialise" << endmsg;

  StatusCode sc;
  sc = toolSvc()->retrieveTool("FuncMinimum", m_publicTool );
  if( sc.isFailure() )
    {
      log << MSG::ERROR<< "Error retrieving the public tool" << endmsg;
    }
  sc = toolSvc()->retrieveTool("FuncMinimum", m_privateTool, this );
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
StatusCode FuncMinimumGenAlg::execute() {

  MsgStream  log( msgSvc(), name() );
  log << MSG::INFO << "==> Execute" << endmsg;

  // the object of the class Func
  Func func;

//=============================================================================
  // Input number and value of the arguments of the function "GenFunc"
  IFuncMinimum::Arg arg (func.dimensionality ());

  arg[0] = 5;
  arg[1] = 10;

  // Matrix of error
  IFuncMinimum::Covariance matrix_error (arg.dimension(), 0);

  // Call of the method
  m_publicTool->minimum( func ,
                         arg  );
  log << endmsg;
  log << "START OF THR METHOD" << endmsg;
  log << "MINIMUM FOUND AT: " << endmsg;

  for (unsigned int i = 0; i < arg.dimension(); i++)
    {

      log << "Value of argument " << i <<" is " << arg[i] << endmsg;
    }

  log << endmsg;

//=============================================================================
  // With Covariance matrix (matrix of error)
  arg[0] = 5;
  arg[1] = 10;

  // Call of the method(with covariance matrix (matrix of error))
  m_publicTool->minimum( func        ,
                         arg         ,
                         matrix_error);
  log << endmsg;
  log << "THE METHOD WITH MATRIX OF ERROR" << endmsg;
  log << "MINIMUM FOUND AT: " << endmsg;

  for (unsigned int i = 0; i < arg.dimension(); i++)
    {

      log << "Value of argument " << i <<" is " << arg[i] << endmsg;
    }

  log << endmsg;
  log << "MATRIX OF ERROR";

  for (unsigned int i = 0; i < arg.dimension(); i++)
    {
      log << endmsg;

      for (unsigned int j = 0; j < arg.dimension(); j++)
        {
          log << matrix_error (i+1, j+1) << " ";
        }
    }
  log << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode FuncMinimumGenAlg::finalize() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool  );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY(FuncMinimumGenAlg)

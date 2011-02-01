// $Id: EqSolverIAlg.cpp,v 1.5 2006/01/10 19:58:26 hmd Exp $

// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiGSL/IEqSolver.h"
#include "GaudiMath/Adapters.h"
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
// from AIDA
#include "AIDA/IFunction.h"
// local
#include "EqSolverIAlg.h"

#ifdef __ICC
// disable icc warning #327: NULL reference is not allowed
//   It's a real problem, but here it is used for test purposes
#pragma warning(disable:327)
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
namespace CLHEP { }
using namespace CLHEP;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
EqSolverIAlg::EqSolverIAlg( const std::string& name,
			ISvcLocator* pSvcLocator)
  : Algorithm ( name , pSvcLocator ) {

}

//=============================================================================
// Destructor
//=============================================================================
EqSolverIAlg::~EqSolverIAlg() {}

typedef Genfun::AbsFunction GenFunc;

// Class for the function "IFunction"
// @see AIDA/IFunction.h
class Function1 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument ;
public:
  Function1 () {};
  virtual ~Function1 () {};
  double value ( const argument& x ) const { return x[0] - 1; }
  int dimension () const { return 3; }
  bool setTitle(const std::string& ) { return false; }
  std::string title(void) const {return "";}
  bool isEqual(const AIDA::IFunction&) const { return false; }
  const std::vector<double>& gradient(const std::vector<double>&) const { return m_values;}
  bool providesGradient(void) const { return false; }
  std::string variableName(int) const  { return ""; }
  std::vector<std::string> variableNames(void) const {return m_names;}
  bool setParameters(const std::vector<double> &) {return true;}
  const std::vector<double>& parameters(void) const {return m_values;}
  int numberOfParameters(void) const {return 0;}
  std::vector<std::string> parameterNames() const {return m_names;}
  bool setParameter(const std::string&, double){return false;}
  double parameter(const std::string&) const { return 0.0;}
  int indexOfParameter(const std::string&) const {return 0;}
  const AIDA::IAnnotation& annotation(void) const {return *(AIDA::IAnnotation*)0;}
  AIDA::IAnnotation& annotation(void){return *(AIDA::IAnnotation*)0;}
  std::string codeletString(void) const {return "";}
  void* cast(const std::string &) const { return 0; }
private:
  std::string              m_version ;
  std::string              m_label   ;
  std::vector<std::string> m_names   ;
  std::vector<double>      m_values  ;
};

// Class for the function "IFunction"
class Function2 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument ;
public:
  Function2 () {};
  virtual ~Function2 () {};
  double value ( const argument& x ) const { return x[1] - 1; }
  int dimension () const { return 3; }
  bool setTitle(const std::string& ) { return false; }
  std::string title(void) const {return "";}
  bool isEqual(const AIDA::IFunction&) const { return false; }
  const std::vector<double>& gradient(const std::vector<double>&) const { return m_values;}
  bool providesGradient(void) const { return false; }
  std::string variableName(int) const  { return ""; }
  std::vector<std::string> variableNames(void) const {return m_names;}
  bool setParameters(const std::vector<double> &) {return true;}
  const std::vector<double>& parameters(void) const {return m_values;}
  int numberOfParameters(void) const {return 0;}
  std::vector<std::string> parameterNames() const {return m_names;}
  bool setParameter(const std::string&, double){return false;}
  double parameter(const std::string&) const { return 0.0;}
  int indexOfParameter(const std::string&) const {return 0;}
  const AIDA::IAnnotation& annotation(void) const {return *(AIDA::IAnnotation*)0;}
  AIDA::IAnnotation& annotation(void){return *(AIDA::IAnnotation*)0;}
  std::string codeletString(void) const {return "";}
  void* cast(const std::string &) const { return 0; }
private:
  std::string              m_version ;
  std::string              m_label   ;
  std::vector<std::string> m_names   ;
  std::vector<double>      m_values  ;
};

// Class for the function "IFunction"
class Function3 : virtual public AIDA::IFunction
{
public:
  typedef std::vector<double> argument ;
public:
  Function3 () {};
  virtual ~Function3 () {};
  double value ( const argument& x ) const { return x[2] - 1; }
  int dimension () const { return 3; }
  bool setTitle(const std::string& ) { return false; }
  std::string title(void) const {return "";}
  bool isEqual(const AIDA::IFunction&) const { return false; }
  const std::vector<double>& gradient(const std::vector<double>&) const { return m_values;}
  bool providesGradient(void) const { return false; }
  std::string variableName(int) const  { return ""; }
  std::vector<std::string> variableNames(void) const {return m_names;}
  bool setParameters(const std::vector<double> &) {return true;}
  const std::vector<double>& parameters(void) const {return m_values;}
  int numberOfParameters(void) const {return 0;}
  std::vector<std::string> parameterNames() const {return m_names;}
  bool setParameter(const std::string&, double){return false;}
  double parameter(const std::string&) const { return 0.0;}
  int indexOfParameter(const std::string&) const {return 0;}
  const AIDA::IAnnotation& annotation(void) const {return *(AIDA::IAnnotation*)0;}
  AIDA::IAnnotation& annotation(void){return *(AIDA::IAnnotation*)0;}
  std::string codeletString(void) const {return "";}
  void* cast(const std::string &) const { return 0; }
private:
  std::string              m_version ;
  std::string              m_label   ;
  std::vector<std::string> m_names   ;
  std::vector<double>      m_values  ;
};

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode EqSolverIAlg::initialize() {

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
StatusCode EqSolverIAlg::execute() {

  MsgStream  log( msgSvc(), name() );
  log << MSG::INFO << "==> Execute" << endmsg;

  //the objects of IFunction's classes
  const Function1* fun1 = new Function1();
  const Function2* fun2 = new Function2();
  const Function3* fun3 = new Function3();

  // the objects of the class AdapterIFunction
  // @see Adapter.h
  const GaudiMath::AIDAFunction& adap1 = GaudiMath::adapter(*fun1);
  const GaudiMath::AIDAFunction& adap2 = GaudiMath::adapter(*fun2);
  const GaudiMath::AIDAFunction& adap3 = GaudiMath::adapter(*fun3);

  std::vector<const GenFunc*> function;

  function.push_back(&adap1);
  function.push_back(&adap2);
  function.push_back(&adap3);

//=============================================================================

  // Input number and value of the arguments of the function "GenFunc"
  IEqSolver::Arg arg (function.size ());

  arg[0] = 10;
  arg[1] = 5;
  arg[2] = 29;

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
StatusCode EqSolverIAlg::finalize() {

  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "==> Finalize" << endmsg;

  toolSvc()->releaseTool( m_publicTool  );
  toolSvc()->releaseTool( m_privateTool );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY(EqSolverIAlg)

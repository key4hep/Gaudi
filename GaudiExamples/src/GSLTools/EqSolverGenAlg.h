#ifndef ALGTOOLS_EQSOLVERGENALG_H
#define ALGTOOLS_EQSOLVERGENALG_H 1
// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/// Forward references
struct IEqSolver;

//-----------------------------------------------------------------------------
/**  @class EqSolverGenAlg EqSolver.h
 *   @file Algorithm file for the class IEqSolver
 *   @see GaudiGSL/IEqSolver.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2003-07-07
 */
//-----------------------------------------------------------------------------

class EqSolverGenAlg : public Algorithm
{

public:
  /// Standard constructor
  EqSolverGenAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~EqSolverGenAlg() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

protected:
private:
  IEqSolver* m_privateTool;
  IEqSolver* m_publicTool;
};
#endif // ALGTOOLS_EQSOLVERGENALG_H

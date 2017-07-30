#ifndef ALGTOOLS_EQSOLVERIALG_H
#define ALGTOOLS_EQSOLVERIALG_H 1


// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/// Forward references
struct IEqSolver;

//-----------------------------------------------------------------------------
/**  @class EqSolverIAlg EqSolver.h
 *   @file Algorithm file for the class IEqSolver
 *   @see GaudiGSL/IEqSolver.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2003-07-07
 */
//-----------------------------------------------------------------------------

class EqSolverIAlg : public Algorithm {

public:
  /// Standard constructor
  EqSolverIAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~EqSolverIAlg( ) override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

protected:

private:

  IEqSolver*   m_privateTool;
  IEqSolver*   m_publicTool;

};
#endif // ALGTOOLS_EQSOLVERIALG_H

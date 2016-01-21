#ifndef ALGTOOLS_EQSOLVERGENALG_H 
#define ALGTOOLS_EQSOLVERGENALG_H 1
// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/// Forward references
class IEqSolver;

//-----------------------------------------------------------------------------
/**  @class EqSolverGenAlg EqSolver.h
 *   @file Algorithm file for the class IEqSolver
 *   @see GaudiGSL/IEqSolver.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2003-07-07
 */
//-----------------------------------------------------------------------------

class EqSolverGenAlg : public Algorithm {

public:
  /// Standard constructor
  EqSolverGenAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~EqSolverGenAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:

  IEqSolver*   m_privateTool;
  IEqSolver*   m_publicTool;

};
#endif // ALGTOOLS_EQSOLVERGENALG_H

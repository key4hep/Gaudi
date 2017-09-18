#ifndef ALGTOOLS_FUNCMINIMUMGENALG_H
#define ALGTOOLS_FUNCMINIMUMGENALG_H 1

// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"


/// Forward references
struct IFuncMinimum;

//-----------------------------------------------------------------------------
/**  @class FuncMinimumGenAlg FuncMinimumGenAlg.h
 *   @file Algorithm file for the class IFuncMinimum
 *   @see GaudiGSL/IFuncMinimum.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2002-09-14
 */
//-----------------------------------------------------------------------------

class FuncMinimumGenAlg : public Algorithm {

public:
  /// Standard constructor
  FuncMinimumGenAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~FuncMinimumGenAlg( ) override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

protected:

private:

  IFuncMinimum*   m_privateTool;
  IFuncMinimum*   m_publicTool;

};
#endif // ALGTOOLS_FUNCMINIMUMGENALG_H

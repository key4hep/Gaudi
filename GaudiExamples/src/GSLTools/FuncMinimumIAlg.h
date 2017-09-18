#ifndef ALGTOOLS_FUNCMINIMUMIALG_H
#define ALGTOOLS_FUNCMINIMUMIALG_H 1

// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"


/// Forward references
struct IFuncMinimum;

//-----------------------------------------------------------------------------
/**  @class FuncMinimumIAlg FuncMinimumIAlg.h
 *   @file Algorithm file for the class IFuncMinimum
 *   @see GaudiGSL/IFuncMinimum.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2002-09-14
 */
//-----------------------------------------------------------------------------

class FuncMinimumIAlg : public Algorithm {

public:
  /// Standard constructor
  FuncMinimumIAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~FuncMinimumIAlg( ) override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

protected:

private:

  IFuncMinimum*   m_privateTool;
  IFuncMinimum*   m_publicTool;

};
#endif // ALGTOOLS_FUNCMINIMUMIALG_H

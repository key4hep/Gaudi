// $Id: FuncMinimumPAlg.h,v 1.1 2003/11/27 11:05:27 mato Exp $
#ifndef ALGTOOLS_FUNCMINIMUMPALG_H 
#define ALGTOOLS_FUNCMINIMUMPALG_H 1

// Include files

// from Gaudi
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"


/// Forward references
class IFuncMinimum;

//-----------------------------------------------------------------------------
/**  @class FuncMinimumPAlg FuncMinimumPAlg.h
 *   @file Algorithm file for the class IFuncMinimum
 *   @see GaudiGSL/IFuncMinimum.h
 *   @author Kirill Miklyaev kirillm@iris1.itep.ru
 *   @date   2002-09-14
 */
//-----------------------------------------------------------------------------

class FuncMinimumPAlg : public Algorithm {

public:
  /// Standard constructor
  FuncMinimumPAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~FuncMinimumPAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:

  IFuncMinimum*   m_privateTool;
  IFuncMinimum*   m_publicTool;

};
#endif // ALGTOOLS_FUNCMINIMUMPALG_H

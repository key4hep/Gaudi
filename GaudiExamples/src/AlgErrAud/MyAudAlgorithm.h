// $Id: MyAudAlgorithm.h,v 1.1 2007/01/22 16:06:14 hmd Exp $
#ifndef GAUDIEXAMPLE_MYAUDALGORITHM_H
#define GAUDIEXAMPLE_MYAUDALGORITHM_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

// Forward references
class IMyAudTool;

/** @class MyAudAlgorithm
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class MyAudAlgorithm : public Algorithm {
public:
  /// Constructor of this form must be provided
  MyAudAlgorithm(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  IMyAudTool*   m_tool;

  int m_i;
};

#endif    // GAUDIEXAMPLE_MYALGORITHM_H

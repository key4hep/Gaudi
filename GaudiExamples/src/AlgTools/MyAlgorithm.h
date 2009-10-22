// $Id: MyAlgorithm.h,v 1.4 2008/10/10 15:18:56 marcocle Exp $
#ifndef GAUDIEXAMPLE_MYALGORITHM_H
#define GAUDIEXAMPLE_MYALGORITHM_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

// Forward references
class IMyTool;

/** @class MyAlgorithm
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class MyAlgorithm : public Algorithm {
public:
  /// Constructor of this form must be provided
  MyAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  IMyTool*   m_privateTool;
  IMyTool*   m_publicTool;
  IMyTool*   m_privateGTool;
  IMyTool*   m_publicGTool;

  std::string m_privateToolType;
  IMyTool*   m_privateToolWithName;

  IMyOtherTool* m_privateOtherInterface;

};

#endif    // GAUDIEXAMPLE_MYALGORITHM_H

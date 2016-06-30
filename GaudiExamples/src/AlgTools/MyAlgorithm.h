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
  using Algorithm::Algorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
private:

  StringProperty m_privateToolType{this, "ToolWithName", "MyTool", "Type of the tool to use (internal name is ToolWithName)"};
  BooleanProperty m_privateToolsOnly{this, "PrivateToolsOnly", false, "Do not look for public tools."};

  IMyTool*   m_privateTool = nullptr;
  IMyTool*   m_publicTool = nullptr;
  IMyTool*   m_privateGTool = nullptr;
  IMyTool*   m_publicGTool = nullptr;

  IMyTool*   m_privateToolWithName = nullptr;

  IMyOtherTool* m_privateOtherInterface = nullptr;
};

#endif    // GAUDIEXAMPLE_MYALGORITHM_H

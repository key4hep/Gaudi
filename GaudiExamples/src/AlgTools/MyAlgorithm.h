/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLE_MYALGORITHM_H
#define GAUDIEXAMPLE_MYALGORITHM_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

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

  bool isClonable() const override { return true; }

private:
  Gaudi::Property<std::string> m_privateToolType{ this, "ToolWithName", "MyTool",
                                                  "Type of the tool to use (internal name is ToolWithName)" };
  Gaudi::Property<bool>        m_privateToolsOnly{ this, "PrivateToolsOnly", false, "Do not look for public tools." };

  IMyTool* m_privateTool  = nullptr;
  IMyTool* m_publicTool   = nullptr;
  IMyTool* m_privateGTool = nullptr;
  IMyTool* m_publicGTool  = nullptr;

  IMyTool* m_privateToolWithName = nullptr;

  IMyOtherTool* m_privateOtherInterface = nullptr;
};

#endif // GAUDIEXAMPLE_MYALGORITHM_H

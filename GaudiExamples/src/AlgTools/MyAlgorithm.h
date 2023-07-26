/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "IMyTool.h"
#include <Gaudi/Examples/TestAlg.h>

/** @class MyAlgorithm
    Trivial Algorithm for tutorial purposes

    @author nobody
*/
class MyAlgorithm : public Gaudi::Examples::TestAlg {
public:
  /// Constructor of this form must be provided or inherited from the base class
  MyAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  bool isClonable() const override { return true; }

private:
  Gaudi::Property<std::string> m_privateToolType{ this, "ToolWithName", "MyTool",
                                                  "Type of the tool to use (internal name is ToolWithName)" };
  Gaudi::Property<bool>        m_privateToolsOnly{ this, "PrivateToolsOnly", false, "Do not look for public tools." };

  IMyTool* m_privateTool = nullptr;
  IMyTool* m_publicTool  = nullptr;

  IMyTool* m_privateToolWithName = nullptr;

  IMyOtherTool* m_privateOtherInterface = nullptr;

  ToolHandle<IMyTool> m_legacyToolHandle{ "MyTool/LegacyToolHandle", this };

  ToolHandle<IMyTool>       m_myPrivToolHandle{ this, "PrivToolHandle", "MyTool/PrivToolHandle" };
  PublicToolHandle<IMyTool> m_myPubToolHandle{ this, "PubToolHandle", "MyTool/PubToolHandle" };

  PublicToolHandle<IAlgTool> m_myGenericToolHandle{ this, "GenericToolHandle", "MyTool/GenericToolHandle" };

  ToolHandle<IAlgTool> m_myUnusedToolHandle{ this, "UnusedToolHandle", "TestToolFailing/UnusedToolHandle" };

  ToolHandle<IMyTool> m_undefinedToolHandle{ this };
  ToolHandle<IMyTool> m_invalidToolHandle{ this, "InvalidToolHandle", "TestToolFailing" };

  ToolHandle<IWrongTool> m_wrongIfaceTool{ this, "WrongIfaceTool", "MyTool/WrongIfaceTool" };

  PublicToolHandle<const IMyTool> m_myConstToolHandle{ "MyTool/ConstGenericToolHandle" };

  PublicToolHandle<const IMyTool> m_myCopiedConstToolHandle;
  PublicToolHandle<const IMyTool> m_myCopiedConstToolHandle2;
  PublicToolHandle<IMyTool>       m_myCopiedToolHandle;

  PublicToolHandleArray<IMyTool> m_tha{ this,
                                        "MyPublicToolHandleArrayProperty",
                                        { "MyTool/AnotherConstGenericToolHandle", "MyTool/AnotherInstanceOfMyTool" } };
};

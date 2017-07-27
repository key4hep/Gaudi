#ifndef GAUDIEXAMPLE_MYALGORITHM_H
#define GAUDIEXAMPLE_MYALGORITHM_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"

// Forward references
class IMyTool;

/** @class MyAlgorithm
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class MyGaudiAlgorithm : public GaudiAlgorithm
{
public:
  /// Constructor of this form must be provided
  MyGaudiAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  bool isClonable() const override { return true; }

private:
  Gaudi::Property<std::string> m_privateToolType{this, "ToolWithName", "MyTool",
                                                 "Type of the tool to use (internal name is ToolWithName)"};

  IMyTool* m_privateTool  = nullptr;
  IMyTool* m_publicTool   = nullptr;
  IMyTool* m_privateGTool = nullptr;
  IMyTool* m_publicGTool  = nullptr;

  IMyTool* m_privateToolWithName = nullptr;

  IMyOtherTool* m_privateOtherInterface = nullptr;

  ToolHandle<IMyTool> m_myPrivToolHandle{this, "PrivToolHandle", "MyTool"};
  PublicToolHandle<IMyTool> m_myPubToolHandle{this, "PubToolHandle", "MyTool"};

  PublicToolHandle<IAlgTool> m_myGenericToolHandle{this, "GenericToolHandle", "MyTool"};

  ToolHandle<IAlgTool> m_myUnusedToolHandle{this, "UnusedToolHandle", "TestToolFailing"};

  PublicToolHandle<const IMyTool> m_myConstToolHandle{"MyTool/ConstGenericToolHandle"};

  PublicToolHandle<const IMyTool> m_myCopiedConstToolHandle;
  PublicToolHandle<const IMyTool> m_myCopiedConstToolHandle2;
  PublicToolHandle<IMyTool> m_myCopiedToolHandle;

  ToolHandleArray<IMyTool> m_tha;

  DataObjectHandle<DataObject> m_tracks;
  DataObjectHandle<DataObject> m_hits;
  DataObjectHandle<DataObject> m_raw;

  DataObjectHandle<DataObject> m_selectedTracks;

};

#endif // GAUDIEXAMPLE_MYALGORITHM_H

//	====================================================================
//  PartitionSwitchAlg.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/IPartitionControl.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/SmartIF.h"

/**@class PartitionSwitchAlg
  *
  * Small algorith, which switches the partition of a configurable
  * multi-service. The algorithm can be part of a sequence, which
  * allows for e.g. buffer tampering.
  *
  * @author:  M.Frank
  * @version: 1.0
  */
class PartitionSwitchAlg : public extends1<Algorithm, IPartitionControl> {

  typedef StatusCode         STATUS;
  typedef const std::string& CSTR;

private:

  /// Job option to set the requested partition name
  std::string        m_partName;
  /// Job option to set the tool manipulating the multi-service name
  std::string        m_toolType;
  /// reference to Partition Controller
  IPartitionControl* m_actor = nullptr;

public:

  /// Standard algorithm constructor
  PartitionSwitchAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : base_class(name, pSvcLocator)
  {
    declareProperty("Partition",  m_partName);
    declareProperty("Tool",       m_toolType="PartitionSwitchTool");
  }
  /// Standard Destructor
  ~PartitionSwitchAlg() override = default;

  /// Initialize
  STATUS initialize() override  {
    MsgStream log(msgSvc(), name());
    SmartIF<IAlgTool> tool(m_actor);
    STATUS sc = toolSvc()->retrieveTool(m_toolType,m_actor,this);
    if ( sc.isFailure() ) {
      log << MSG::ERROR << "Unable to load PartitionSwitchTool "
          << m_toolType << endmsg;
      return sc;
    }
    /// Release old tool
    if ( tool ) toolSvc()->releaseTool(tool);
    /// Now check if the partition is present. If not: try to create it
    IInterface* partititon = 0;
    sc = m_actor->get(m_partName, partititon);
    if ( !sc.isSuccess() )  {
      log << MSG::ERROR << "Cannot access partition \""
          << m_partName << "\"" << endmsg;
    }
    return sc;
  }

  /// Finalize
  STATUS finalize()  override {
    SmartIF<IAlgTool> tool(m_actor);
    if ( tool ) toolSvc()->releaseTool(tool);
    m_actor = nullptr;
    return STATUS::SUCCESS;
  }

  /// Execute procedure
  STATUS execute()  override {
    if ( m_actor )  {
      STATUS sc = m_actor->activate(m_partName);
      if ( !sc.isSuccess() )  {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "Cannot activate partition \""
            << m_partName << "\"!" << endmsg;
      }
      return sc;
    }
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "The partition control tool \"" << name()
        << "." << m_toolType << "\" cannot be accessed!" << endmsg;
    return STATUS::FAILURE;
  }

  void _check(STATUS sc, CSTR msg)  const {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << msg << " Status=" << sc.getCode() << endmsg;
  }
#define CHECK(x,y) if ( !x.isSuccess() ) _check(x, y); return x;

  /// Create a partition object. The name identifies the partition uniquely
  STATUS create(CSTR nam, CSTR typ)  override {
    STATUS sc = m_actor ? m_actor->create(nam,typ) : NO_INTERFACE;
    CHECK(sc, "Cannot create partition: "+nam+" of type "+typ);
  }
  /// Create a partition object. The name identifies the partition uniquely
  STATUS create(CSTR nam, CSTR typ, IInterface*& pPartition)  override {
    STATUS sc = m_actor ? m_actor->create(nam,typ,pPartition) : NO_INTERFACE;
    CHECK(sc, "Cannot create partition: "+nam+" of type "+typ);
  }
  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop(CSTR nam)  override {
    STATUS sc = m_actor ? m_actor->drop(nam) : NO_INTERFACE;
    CHECK(sc, "Cannot drop partition: "+nam);
  }
  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop(IInterface* pPartition)  override {
    STATUS sc = m_actor ? m_actor->drop(pPartition) : NO_INTERFACE;
    CHECK(sc, "Cannot drop partition by Interface.");
  }
  /// Activate a partition object. The name identifies the partition uniquely.
  STATUS activate(CSTR nam)  override {
    STATUS sc = m_actor ? m_actor->activate(nam) : NO_INTERFACE;
    CHECK(sc, "Cannot activate partition: "+nam);
  }
  /// Activate a partition object.
  STATUS activate(IInterface* pPartition)  override {
    STATUS sc = m_actor ? m_actor->activate(pPartition) : NO_INTERFACE;
    CHECK(sc, "Cannot activate partition by Interface.");
  }
  /// Access a partition object. The name identifies the partition uniquely.
  STATUS get(CSTR nam, IInterface*& pPartition) const  override {
    STATUS sc = m_actor ? m_actor->get(nam, pPartition) : NO_INTERFACE;
    CHECK(sc, "Cannot get partition "+nam);
  }
  /// Access the active partition object.
  STATUS activePartition(std::string& nam, IInterface*& pPartition) const  override {
    STATUS sc = m_actor ? m_actor->activePartition(nam, pPartition) : NO_INTERFACE;
    CHECK(sc, "Cannot determine active partition.");
  }
};

DECLARE_COMPONENT(PartitionSwitchAlg)

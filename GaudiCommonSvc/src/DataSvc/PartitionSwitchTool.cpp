// Framework include files
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IPartitionControl.h"

/** @class PartitionSwitchTool
 *
 *  @author Markus Frank
 *  @date   2004-06-24
 */
class PartitionSwitchTool: public extends<AlgTool,
                                          IPartitionControl> {

  using CSTR = const std::string&;
  using STATUS = StatusCode;

private:

  /// Job option to set the multi-service name
  std::string                m_actorName;
  /// reference to Partition Controller
  SmartIF<IPartitionControl> m_actor;

public:

  /// Standard constructor
  PartitionSwitchTool(const std::string& typ, const std::string& nam, const IInterface* parent)
  : base_class( typ, nam , parent)
  {
    declareProperty("Actor", m_actorName = "EventDataService");
  }
  /// Standard destructor
  ~PartitionSwitchTool()   override = default; 

  /// Initialize
  STATUS initialize()  override {
    /// Access partitioned multi-service
    STATUS sc = AlgTool::initialize();
    if ( !sc.isSuccess() )  {
      error() << "Cannot initialize base class!" << endmsg;
      return sc;
    }
    m_actor = nullptr;
    IPartitionControl* tmpPtr = nullptr;
    sc = service(m_actorName, tmpPtr);
    m_actor = tmpPtr;
    if ( !sc.isSuccess() )  {
      error() << "Cannot retrieve partition controller \""
          << m_actorName << "\"!" << endmsg;
      return sc;
    }
    return sc;
  }
  /// Finalize
  STATUS finalize()   override {
    m_actor = nullptr;
    return AlgTool::finalize();
  }

  void _check(STATUS sc, CSTR msg)  const {
    error() << msg << " Status=" << sc.getCode() << endmsg;
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

// Declaration of the Tool Factory
DECLARE_COMPONENT(PartitionSwitchTool)

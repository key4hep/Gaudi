// Framework include files
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IPartitionControl.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"

/** @class PartitionSwitchTool
 *
 *  @author Markus Frank
 *  @date   2004-06-24
 */
class PartitionSwitchTool : public extends<AlgTool, IPartitionControl>
{

  using CSTR   = const std::string&;
  using STATUS = StatusCode;

private:
  Gaudi::Property<std::string> m_actorName{this, "Actor", "EventDataService", "option to set the multi-service name"};
  /// reference to Partition Controller
  SmartIF<IPartitionControl> m_actor;

public:
  using extends::extends;

  /// Initialize
  STATUS initialize() override
  {
    /// Access partitioned multi-service
    STATUS sc = AlgTool::initialize();
    if ( !sc.isSuccess() ) {
      error() << "Cannot initialize base class!" << endmsg;
      return sc;
    }
    m_actor                   = nullptr;
    IPartitionControl* tmpPtr = nullptr;
    sc                        = service( m_actorName, tmpPtr );
    m_actor                   = tmpPtr;
    if ( !sc.isSuccess() ) {
      error() << "Cannot retrieve partition controller \"" << m_actorName.value() << "\"!" << endmsg;
      return sc;
    }
    return sc;
  }
  /// Finalize
  STATUS finalize() override
  {
    m_actor = nullptr;
    return AlgTool::finalize();
  }

  void _check( STATUS sc, CSTR msg ) const { error() << msg << " Status=" << sc.getCode() << endmsg; }
#define CHECK( x, y )                                                                                                  \
  if ( !x.isSuccess() ) _check( x, y );                                                                                \
  return x;

  /// Create a partition object. The name identifies the partition uniquely
  STATUS create( CSTR nam, CSTR typ ) override
  {
    STATUS sc = m_actor ? m_actor->create( nam, typ ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot create partition: " + nam + " of type " + typ );
  }
  /// Create a partition object. The name identifies the partition uniquely
  STATUS create( CSTR nam, CSTR typ, IInterface*& pPartition ) override
  {
    STATUS sc = m_actor ? m_actor->create( nam, typ, pPartition ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot create partition: " + nam + " of type " + typ );
  }
  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop( CSTR nam ) override
  {
    STATUS sc = m_actor ? m_actor->drop( nam ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot drop partition: " + nam );
  }
  /// Drop a partition object. The name identifies the partition uniquely
  STATUS drop( IInterface* pPartition ) override
  {
    STATUS sc = m_actor ? m_actor->drop( pPartition ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot drop partition by Interface." );
  }
  /// Activate a partition object. The name identifies the partition uniquely.
  STATUS activate( CSTR nam ) override
  {
    STATUS sc = m_actor ? m_actor->activate( nam ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot activate partition: " + nam );
  }
  /// Activate a partition object.
  STATUS activate( IInterface* pPartition ) override
  {
    STATUS sc = m_actor ? m_actor->activate( pPartition ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot activate partition by Interface." );
  }
  /// Access a partition object. The name identifies the partition uniquely.
  STATUS get( CSTR nam, IInterface*& pPartition ) const override
  {
    STATUS sc = m_actor ? m_actor->get( nam, pPartition ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot get partition " + nam );
  }
  /// Access the active partition object.
  STATUS activePartition( std::string& nam, IInterface*& pPartition ) const override
  {
    STATUS sc = m_actor ? m_actor->activePartition( nam, pPartition ) : IInterface::Status::NO_INTERFACE;
    CHECK( sc, "Cannot determine active partition." );
  }
};

// Declaration of the Tool Factory
DECLARE_COMPONENT( PartitionSwitchTool )

// ========== header
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataStoreLeaves.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/SmartIF.h"

class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;
class IRegistry;
/**
 * Tool to scan a transient store branch that collects all the objects that belong
 * to the same source (file).
 *
 * By default, the list of entries is cached and the cache is cleared at every
 * BeginEvent incident.
 *
 * \par Properties:
 * \par
 * \b DataService (string): Name of the data service to use [EventDataSvc]
 * \par
 * \b Root (string): Path to the element from which to start the scan [root of the data service]
 * \par
 * \b ScanOnBeginEvent (bool): If the scan has to be started during the BeginEvent incident (true) or on demand (false,
 * default)
 */
class DataSvcFileEntriesTool : public extends<AlgTool, IDataStoreLeaves, IIncidentListener>
{
public:
  /// Inherited constructor
  using extends::extends;

  /// Destructor
  ~DataSvcFileEntriesTool() override = default;

  /// Initialize the tool.
  StatusCode initialize() override;

  /// Finalize the tool.
  StatusCode finalize() override;

  /// Return the list of collected objects.
  /// If the scan was not yet done since the last BeginEvent incident, it is done
  /// when calling this function.
  /// The result of the scan is cached.
  const LeavesList& leaves() const override;

  /// Call-back function for the BeginEvent incident.
  /// Clears the internal cache, cache the file ID of the \b Root node and, if
  /// the property \b ScanOnBeginEvent is set to true, scans the data service.
  void handle( const Incident& incident ) override;

private:
  Gaudi::Property<std::string> m_dataSvcName{this, "DataService", "EventDataSvc", "Name of the data service to use"};
  Gaudi::Property<std::string> m_rootNode{this, "Root", "", "Path to the element from which to start the scan"};
  Gaudi::Property<bool> m_scanOnBeginEvent{
      this, "ScanOnBeginEvent", false,
      "If the scan has to be started during the BeginEvent incident (true) or on demand (false, default)"};
  Gaudi::Property<bool> m_ignoreOriginChange{
      this, "IgnoreOriginChange", false,
      "Disable the detection of the change in the origin of object between the BeginEvent and the scan"};

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;
  /// Pointer to the IDataManagerSvc interface of the data service.
  SmartIF<IDataManagerSvc> m_dataMgrSvc;
  /// Pointer to the IDataProviderSvc interface of the data service.
  SmartIF<IDataProviderSvc> m_dataSvc;

  /// Internal cache for the list of objects found during the scan.
  LeavesList m_leaves;

  /// Scan the data service starting from the node specified as \b Root.
  void i_collectLeaves();
  /// Scan the data service starting from the specified node.
  void i_collectLeaves( IRegistry* reg );

  /// Return the pointer to the IRegistry object associated to the node
  /// specified as \b Root.
  IRegistry* i_getRootNode();

  /// File ID of the \b Root node.
  /// It is cached every BeginEvent to be compared with the one seen during the
  /// collection of the leaves, to avoid that the collection is altered by
  /// previous calls to an OutputStream.
  std::string m_initialBase;
};

// ========== implementation
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"

#include "GaudiKernel/GaudiException.h"

StatusCode DataSvcFileEntriesTool::initialize()
{
  StatusCode sc = AlgTool::initialize();
  if ( sc.isFailure() ) return sc;

  // Retrieve the pointer to the needed services.

  m_incidentSvc = serviceLocator()->service( "IncidentSvc" );
  if ( !m_incidentSvc ) {
    error() << "Cannot get IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_dataMgrSvc = m_dataSvc = serviceLocator()->service( m_dataSvcName );
  if ( !m_dataSvc || !m_dataMgrSvc ) {
    error() << "Cannot get IDataProviderSvc+IDataManagerSvc " << m_dataSvcName << endmsg;
    return StatusCode::FAILURE;
  }

  // Register ourself to the incident service as listener for BeginEvent
  m_incidentSvc->addListener( this, IncidentType::BeginEvent );

  // If the Root node is not specified, take the name from the data service itself.
  if ( m_rootNode.empty() ) {
    m_rootNode = m_dataMgrSvc->rootName();
  }

  // Clear the cache (in case the instance is re-initilized).
  m_leaves.clear();
  return StatusCode::SUCCESS;
}

StatusCode DataSvcFileEntriesTool::finalize()
{
  // unregister from the incident service
  if ( m_incidentSvc ) {
    m_incidentSvc->removeListener( this, IncidentType::BeginEvent );
  }
  // Release the services
  m_incidentSvc.reset();
  m_dataMgrSvc.reset();
  m_dataSvc.reset();

  return AlgTool::finalize();
}

void DataSvcFileEntriesTool::handle( const Incident& incident )
{
  // Get the file id of the root node at every event
  IOpaqueAddress* addr = i_getRootNode()->address();
  if ( addr )
    m_initialBase = addr->par()[0];
  else
    m_initialBase.clear(); // use empty file id if there is no address

  m_leaves.clear();
  if ( m_scanOnBeginEvent ) {
    verbose() << "::handle scanning on " << incident.type() << endmsg;
    i_collectLeaves();
  }
}

const IDataStoreLeaves::LeavesList& DataSvcFileEntriesTool::leaves() const
{
  if ( m_leaves.empty() ) {
    const_cast<DataSvcFileEntriesTool*>( this )->i_collectLeaves();
  }
  return m_leaves;
}

IRegistry* DataSvcFileEntriesTool::i_getRootNode()
{
  DataObject* obj = nullptr;
  StatusCode sc   = m_dataSvc->retrieveObject( m_rootNode.value(), obj );
  if ( sc.isFailure() ) {
    throw GaudiException( "Cannot get " + m_rootNode + " from " + m_dataSvcName, name(), StatusCode::FAILURE );
  }
  return obj->registry();
}

void DataSvcFileEntriesTool::i_collectLeaves() { i_collectLeaves( i_getRootNode() ); }

/// todo: implement the scanning as an IDataStoreAgent
void DataSvcFileEntriesTool::i_collectLeaves( IRegistry* reg )
{
  // I do not put sanity checks on the pointers because I know how I'm calling the function
  IOpaqueAddress* addr = reg->address();
  if ( addr ) { // we consider only objects that are in a file
    if ( msgLevel( MSG::VERBOSE ) ) verbose() << "::i_collectLeaves added " << reg->identifier() << endmsg;
    m_leaves.push_back( reg->object() ); // add this object
    // Origin of the current object
    const std::string& base = addr->par()[0];
    // Compare with the origin seen during BeginEvent
    if ( !m_ignoreOriginChange && ( m_initialBase != base ) )
      throw GaudiException( "Origin of data has changed ('" + m_initialBase + "' !=  '" + base +
                                "'), probably OutputStream was called before "
                                "InputCopyStream: check options",
                            name(), StatusCode::FAILURE );

    std::vector<IRegistry*> lfs; // leaves of the current object
    StatusCode sc = m_dataMgrSvc->objectLeaves( reg, lfs );
    if ( sc.isSuccess() ) {
      for ( const auto& i : lfs ) {
        // Continue if the leaf has the same database as the parent
        if ( i->address() && i->address()->par()[0] == base ) {
          DataObject* obj = nullptr;
          sc              = m_dataSvc->retrieveObject( reg, i->name(), obj );
          if ( sc.isSuccess() ) {
            i_collectLeaves( i );
          } else {
            throw GaudiException( "Cannot get " + i->identifier() + " from " + m_dataSvcName, name(),
                                  StatusCode::FAILURE );
          }
        }
      }
    }
  }
}

DECLARE_COMPONENT( DataSvcFileEntriesTool )

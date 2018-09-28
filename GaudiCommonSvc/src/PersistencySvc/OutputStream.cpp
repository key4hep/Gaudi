// Framework include files
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IPersistencySvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Incident.h"

#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"

#include "OutputStream.h"

#include <set>

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( OutputStream )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )

// initialize data writer
StatusCode OutputStream::initialize()
{

  // Reset the number of events written
  m_events = 0;
  // Get access to the DataManagerSvc
  m_pDataManager = serviceLocator()->service( m_storeName );
  if ( !m_pDataManager ) {
    fatal() << "Unable to locate IDataManagerSvc interface" << endmsg;
    return StatusCode::FAILURE;
  }
  // Get access to the IncidentService
  m_incidentSvc = serviceLocator()->service( "IncidentSvc" );
  if ( !m_incidentSvc ) {
    warning() << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  // Get access to the assigned data service
  m_pDataProvider = serviceLocator()->service( m_storeName );
  if ( !m_pDataProvider ) {
    fatal() << "Unable to locate IDataProviderSvc interface of " << m_storeName << endmsg;
    return StatusCode::FAILURE;
  }
  if ( hasInput() ) {
    StatusCode status = connectConversionSvc();
    if ( !status.isSuccess() ) {
      fatal() << "Unable to connect to conversion service." << endmsg;
      if ( m_outputName != "" && m_fireIncidents )
        m_incidentSvc->fireIncident( Incident( m_outputName, IncidentType::FailOutputFile ) );
      return status;
    }
  }

  // Clear the list with optional items
  clearItems( m_optItemList );
  // Clear the item list
  clearItems( m_itemList );

  // Take the new item list from the properties.
  ON_DEBUG debug() << "ItemList    : " << m_itemNames.value() << endmsg;
  for ( const auto& i : m_itemNames ) addItem( m_itemList, i );

  // Take the new item list from the properties.
  ON_DEBUG debug() << "OptItemList : " << m_optItemNames.value() << endmsg;
  for ( const auto& i : m_optItemNames ) addItem( m_optItemList, i );

  // prepare the algorithm selected dependent locations
  ON_DEBUG debug() << "AlgDependentItemList : " << m_algDependentItemList.value() << endmsg;
  for ( const auto& a : m_algDependentItemList ) {
    // Get the algorithm pointer
    Algorithm* theAlgorithm = decodeAlgorithm( a.first );
    if ( theAlgorithm ) {
      // Get the item list for this alg
      auto& items = m_algDependentItems[theAlgorithm];
      // Clear the list for this alg
      clearItems( items );
      // fill the list again
      for ( const auto& i : a.second ) addItem( items, i );
    }
  }

  // Take the item list to the data service preload list.
  if ( m_doPreLoad ) {
    for ( auto& j : m_itemList ) m_pDataProvider->addPreLoadItem( *j ).ignore();
    // Not working: bad reference counting! pdataSvc->release();
  }

  if ( m_doPreLoadOpt ) {
    for ( auto& j : m_optItemList ) m_pDataProvider->addPreLoadItem( *j ).ignore();
  }
  info() << "Data source: " << m_storeName.value() << " output: " << m_output.value() << endmsg;

  // Decode the accept, required and veto Algorithms. The logic is the following:
  //  a. The event is accepted if all lists are empty.
  //  b. The event is provisionally accepted if any Algorithm in the accept list
  //     has been executed and has indicated that its filter is passed. This
  //     provisional acceptance can be overridden by the other lists.
  //  c. The event is rejected unless all Algorithms in the required list have
  //     been executed and have indicated that their filter passed.
  //  d. The event is rejected if any Algorithm in the veto list has been
  //     executed and has indicated that its filter has passed.
  m_acceptNames.useUpdateHandler();
  m_requireNames.useUpdateHandler();
  m_vetoNames.useUpdateHandler();
  return StatusCode::SUCCESS;
}

// terminate data writer
StatusCode OutputStream::finalize()
{
  info() << "Events output: " << m_events << endmsg;
  if ( m_fireIncidents ) m_incidentSvc->fireIncident( Incident( m_outputName, IncidentType::EndOutputFile ) );
  m_incidentSvc.reset();
  m_pDataProvider.reset();
  m_pDataManager.reset();
  m_pConversionSvc.reset();
  clearItems( m_optItemList );
  clearItems( m_itemList );
  return StatusCode::SUCCESS;
}

// Work entry point
StatusCode OutputStream::execute()
{
  // Clear any previously existing item list
  clearSelection();
  // Test whether this event should be output
  if ( isEventAccepted() ) {
    const StatusCode sc = writeObjects();
    clearSelection();
    ++m_events;
    if ( sc.isSuccess() && m_fireIncidents ) {
      m_incidentSvc->fireIncident( Incident( m_outputName, IncidentType::WroteToOutputFile ) );
    } else if ( m_fireIncidents ) {
      m_incidentSvc->fireIncident( Incident( m_outputName, IncidentType::FailOutputFile ) );
    }
    return sc;
  }
  return StatusCode::SUCCESS;
}

// Select the different objects and write them to file
StatusCode OutputStream::writeObjects()
{
  // Connect the output file to the service
  StatusCode status = collectObjects();
  if ( status.isSuccess() ) {
    IDataSelector* sel = selectedObjects();
    if ( sel->begin() != sel->end() ) {
      status = m_pConversionSvc->connectOutput( m_outputName, m_outputType );
      if ( status.isSuccess() ) {
        // Now pass the collection to the persistency service
        IOpaqueAddress* pAddress = nullptr;
        for ( auto& j : *sel ) {
          try {
            const StatusCode iret = m_pConversionSvc->createRep( j, pAddress );
            if ( !iret.isSuccess() ) {
              status = iret;
              continue;
            }
            IRegistry* pReg = j->registry();
            pReg->setAddress( pAddress );
          } catch ( const std::exception& excpt ) {
            const std::string loc = ( j->registry() ? j->registry()->identifier() : "UnRegistered" );
            fatal() << "std::exception during createRep for '" << loc << "' " << System::typeinfoName( typeid( *j ) )
                    << endmsg;
            fatal() << excpt.what() << endmsg;
            throw;
          }
        }
        for ( auto& j : *sel ) {
          try {
            IRegistry*       pReg           = j->registry();
            const StatusCode iret           = m_pConversionSvc->fillRepRefs( pReg->address(), j );
            if ( !iret.isSuccess() ) status = iret;
          } catch ( const std::exception& excpt ) {
            const std::string loc = ( j->registry() ? j->registry()->identifier() : "UnRegistered" );
            fatal() << "std::exception during fillRepRefs for '" << loc << "'" << System::typeinfoName( typeid( *j ) )
                    << endmsg;
            fatal() << excpt.what() << endmsg;
            throw;
          }
        }
        // Commit the data if there was no error; otherwise possibly discard
        if ( status.isSuccess() ) {
          status = m_pConversionSvc->commitOutput( m_outputName, true );
        } else {
          m_pConversionSvc->commitOutput( m_outputName, false ).ignore();
        }
      }
    }
  }
  return status;
}

// Place holder to create configurable data store agent
bool OutputStream::collect( IRegistry* dir, int level )
{
  if ( level < m_currentItem->depth() ) {
    if ( dir->object() ) {
      /*
        std::cout << "Analysing ("
        << dir->name()
        << ") Object:"
        << ((dir->object()==0) ? "UNLOADED" : "LOADED")
        << std::endl;
      */
      m_objects.push_back( dir->object() );
      return true;
    }
  }
  return false;
}

/// Collect all objects to be written to the output stream
StatusCode OutputStream::collectObjects()
{
  StatusCode status = StatusCode::SUCCESS;

  // Traverse the tree and collect the requested objects
  for ( auto& i : m_itemList ) {
    DataObject* obj = nullptr;
    m_currentItem   = i;
    StatusCode iret = m_pDataProvider->retrieveObject( m_currentItem->path(), obj );
    if ( iret.isSuccess() ) {
      iret                            = collectFromSubTree( obj );
      if ( !iret.isSuccess() ) status = iret;
    } else {
      error() << "Cannot write mandatory object(s) (Not found) " << m_currentItem->path() << endmsg;
      status = iret;
    }
  }

  // Traverse the tree and collect the requested objects (tolerate missing items here)
  for ( auto& i : m_optItemList ) {
    DataObject* obj              = nullptr;
    m_currentItem                = i;
    StatusCode iret              = m_pDataProvider->retrieveObject( m_currentItem->path(), obj );
    if ( iret.isSuccess() ) iret = collectFromSubTree( obj );
    if ( !iret.isSuccess() ) {
      ON_DEBUG
      debug() << "Ignore request to write non-mandatory object(s) " << m_currentItem->path() << endmsg;
    }
  }

  // Collect objects dependent on particular algorithms
  for ( const auto& iAlgItems : m_algDependentItems ) {
    Algorithm*   alg   = iAlgItems.first;
    const Items& items = iAlgItems.second;
    if ( alg->isExecuted() && alg->filterPassed() ) {
      ON_DEBUG
      debug() << "Algorithm '" << alg->name() << "' fired. Adding " << items << endmsg;
      for ( const auto& i : items ) {
        DataObject* obj = nullptr;
        m_currentItem   = i;
        StatusCode iret = m_pDataProvider->retrieveObject( m_currentItem->path(), obj );
        if ( iret.isSuccess() ) {
          iret                            = collectFromSubTree( obj );
          if ( !iret.isSuccess() ) status = iret;
        } else {
          error() << "Cannot write mandatory (algorithm dependent) object(s) (Not found) " << m_currentItem->path()
                  << endmsg;
          status = iret;
        }
      }
    }
  }

  if ( status.isSuccess() ) {
    // Remove duplicates from the list of objects, preserving the order in the list
    std::set<DataObject*>    unique;
    std::vector<DataObject*> tmp; // temporary vector with the reduced list
    tmp.reserve( m_objects.size() );
    for ( auto& o : m_objects ) {
      if ( !unique.count( o ) ) {
        // if the pointer is not in the set, add it to both the set and the temporary vector
        unique.insert( o );
        tmp.push_back( o );
      }
    }
    m_objects.swap( tmp ); // swap the data of the two vectors
  }

  return status;
}

// Clear collected object list
void OutputStream::clearSelection() { m_objects.clear(); }

// Remove all items from the output streamer list;
void OutputStream::clearItems( Items& itms )
{
  for ( auto& i : itms ) delete i;
  itms.clear();
}

// Find single item identified by its path (exact match)
DataStoreItem* OutputStream::findItem( const std::string& path )
{
  auto matchPath                               = [&]( const DataStoreItem* i ) { return i->path() == path; };
  auto                                       i = std::find_if( m_itemList.begin(), m_itemList.end(), matchPath );
  if ( i == m_itemList.end() ) {
    i = std::find_if( m_optItemList.begin(), m_optItemList.end(), matchPath );
    if ( i == m_optItemList.end() ) return nullptr;
  }
  return *i;
}

// Add item to output streamer list
void OutputStream::addItem( Items& itms, const std::string& descriptor )
{
  int         level    = 0;
  auto        sep      = descriptor.rfind( "#" );
  std::string obj_path = descriptor.substr( 0, sep );
  if ( sep != std::string::npos ) {
    std::string slevel = descriptor.substr( sep + 1 );
    level              = ( slevel == "*" ) ? 9999999 : std::stoi( slevel );
  }
  if ( m_verifyItems ) {
    size_t idx = obj_path.find( "/", 1 );
    while ( idx != std::string::npos ) {
      std::string sub_item = obj_path.substr( 0, idx );
      if ( !findItem( sub_item ) ) addItem( itms, sub_item + "#1" );
      idx = obj_path.find( "/", idx + 1 );
    }
  }
  itms.push_back( new DataStoreItem( obj_path, level ) );
  const auto& item = itms.back();
  ON_DEBUG
  debug() << "Adding OutputStream item " << item->path() << " with " << item->depth() << " level(s)." << endmsg;
}

// Connect to proper conversion service
StatusCode OutputStream::connectConversionSvc()
{
  StatusCode status = StatusCode( StatusCode::FAILURE, true );
  // Get output file from input
  std::string dbType, svc, shr;
  for ( auto attrib : Gaudi::Utils::AttribStringParser( m_output ) ) {
    const std::string& tag = attrib.tag;
    const std::string& val = attrib.value;
    switch (::toupper( tag[0] ) ) {
    case 'D':
      m_outputName = val;
      break;
    case 'T':
      dbType = val;
      break;
    case 'S':
      switch (::toupper( tag[1] ) ) {
      case 'V':
        svc = val;
        break;
      case 'H':
        shr = "YES";
        break;
      }
      break;
    case 'O': // OPT='<NEW<CREATE,WRITE,RECREATE>, UPDATE>'
      switch (::toupper( val[0] ) ) {
      case 'R':
        if (::strncasecmp( val.c_str(), "RECREATE", 3 ) == 0 )
          m_outputType = "RECREATE";
        else if (::strncasecmp( val.c_str(), "READ", 3 ) == 0 )
          m_outputType = "READ";
        break;
      case 'C':
      case 'N':
      case 'W':
        m_outputType = "NEW";
        break;
      case 'U':
        m_outputType = "UPDATE";
        break;
      default:
        m_outputType = "???";
        break;
      }
      break;
    default:
      break;
    }
  }
  if ( !shr.empty() ) m_outputType += "|SHARED";
  // Get access to the default Persistency service
  // The default service is the same for input as for output.
  // If this is not desired, then a specialized OutputStream must overwrite
  // this value.
  if ( !dbType.empty() || !svc.empty() ) {
    std::string typ   = !dbType.empty() ? dbType : svc;
    auto        ipers = serviceLocator()->service<IPersistencySvc>( m_persName );
    if ( !ipers ) {
      fatal() << "Unable to locate IPersistencySvc interface of " << m_persName << endmsg;
      return StatusCode::FAILURE;
    }
    IConversionSvc* cnvSvc = nullptr;
    status                 = ipers->getService( typ, cnvSvc );
    if ( !status.isSuccess() ) {
      fatal() << "Unable to locate IConversionSvc interface of database type " << typ << endmsg;
      return status;
    }
    // Increase reference count and keep service.
    m_pConversionSvc = cnvSvc;
  } else {
    fatal() << "Unable to locate IConversionSvc interface (Unknown technology) " << endmsg
            << "You either have to specify a technology name or a service name!" << endmsg
            << "Please correct the job option \"" << name() << ".Output\" !" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

Algorithm* OutputStream::decodeAlgorithm( const std::string& theName )
{
  Algorithm* theAlgorithm = nullptr;

  auto theAlgMgr = serviceLocator()->as<IAlgManager>();
  if ( theAlgMgr ) {
    // Check whether the supplied name corresponds to an existing
    // Algorithm object.
    SmartIF<IAlgorithm>& theIAlg = theAlgMgr->algorithm( theName );
    if ( theIAlg ) {
      try {
        theAlgorithm = dynamic_cast<Algorithm*>( theIAlg.get() );
      } catch ( ... ) {
        // do nothing
      }
    }
  } else {
    fatal() << "Can't locate ApplicationMgr!!!" << endmsg;
  }

  if ( !theAlgorithm ) {
    warning() << "Failed to decode Algorithm name " << theName << endmsg;
  }

  return theAlgorithm;
}

void OutputStream::decodeAlgorithms( Gaudi::Property<std::vector<std::string>>& theNames,
                                     std::vector<Algorithm*>&                   theAlgs )
{
  // Reset the list of Algorithms
  theAlgs.clear();

  // Build the list of Algorithms from the names list
  for ( const auto& it : theNames.value() ) {

    Algorithm* theAlgorithm = decodeAlgorithm( it );
    if ( theAlgorithm ) {
      // Check that the specified algorithm doesn't already exist in the list
      if ( std::find( std::begin( theAlgs ), std::end( theAlgs ), theAlgorithm ) == std::end( theAlgs ) ) {
        theAlgorithm->addRef();
        theAlgs.push_back( theAlgorithm );
      }
    } else {
      info() << it << " doesn't exist - ignored" << endmsg;
    }
  }
}

bool OutputStream::isEventAccepted() const
{
  auto passed = []( const Algorithm* alg ) { return alg->isExecuted() && alg->filterPassed(); };

  // Loop over all Algorithms in the accept list to see
  // whether any have been executed and have their filter
  // passed flag set. Any match causes the event to be
  // provisionally accepted.
  bool result = m_acceptAlgs.empty() || std::any_of( std::begin( m_acceptAlgs ), std::end( m_acceptAlgs ), passed );

  // Loop over all Algorithms in the required list to see
  // whether all have been executed and have their filter
  // passed flag set. Any mismatch causes the event to be
  // rejected.
  if ( result && !m_requireAlgs.empty() ) {
    result = std::all_of( std::begin( m_requireAlgs ), std::end( m_requireAlgs ), passed );
  }

  // Loop over all Algorithms in the veto list to see
  // whether any have been executed and have their filter
  // passed flag set. Any match causes the event to be
  // rejected.
  if ( result && !m_vetoAlgs.empty() ) {
    result = std::none_of( std::begin( m_vetoAlgs ), std::end( m_vetoAlgs ), passed );
  }
  return result;
}

bool OutputStream::hasInput() const
{
  return !( m_itemNames.empty() && m_optItemNames.empty() && m_algDependentItemList.empty() );
}

StatusCode OutputStream::collectFromSubTree( DataObject* pObj )
{
  return m_pDataManager->traverseSubTree( pObj,
                                          [this]( IRegistry* r, int level ) { return this->collect( r, level ); } );
}

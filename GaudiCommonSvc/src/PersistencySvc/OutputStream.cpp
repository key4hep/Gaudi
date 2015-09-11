// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IPersistencySvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/AttribStringParser.h"

#include "OutputStream.h"
#include "OutputStreamAgent.h"

#include <set>

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT(OutputStream)

#define ON_DEBUG if (log.level() <= MSG::DEBUG)

// Standard Constructor
OutputStream::OutputStream(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator),
  m_agent          { new OutputStreamAgent(this) }
{
  m_doPreLoad      = true;
  m_doPreLoadOpt   = false;
  m_verifyItems    = true;
  m_outputType     = "UPDATE";
  m_storeName      = "EventDataSvc";
  m_persName       = "EventPersistencySvc";
  ///in the baseclass, always fire the incidents by default
  ///in e.g. RecordStream this will be set to false, and configurable
  m_fireIncidents  = true;
  declareProperty("ItemList",         m_itemNames);
  declareProperty("OptItemList",      m_optItemNames);
  declareProperty("AlgDependentItemList", m_algDependentItemList);
  declareProperty("Preload",          m_doPreLoad);
  declareProperty("PreloadOptItems",  m_doPreLoadOpt);
  declareProperty("Output",           m_output);
  declareProperty("OutputFile",       m_outputName);
  declareProperty("EvtDataSvc",       m_storeName);
  declareProperty("EvtConversionSvc", m_persName);
  declareProperty("AcceptAlgs",       m_acceptNames);
  declareProperty("RequireAlgs",      m_requireNames);
  declareProperty("VetoAlgs",         m_vetoNames);
  declareProperty("VerifyItems",      m_verifyItems);
  ///in the baseclass, always fire the incidents by default
  ///in RecordStream this will be set to false, and configurable

  // Associate action handlers with the AcceptAlgs, RequireAlgs and VetoAlgs.
  m_acceptNames.declareUpdateHandler ( &OutputStream::acceptAlgsHandler , this );
  m_requireNames.declareUpdateHandler( &OutputStream::requireAlgsHandler, this );
  m_vetoNames.declareUpdateHandler   ( &OutputStream::vetoAlgsHandler   , this );

  //setProperty( "OutputLevel", 2 );

}


// initialize data writer
StatusCode OutputStream::initialize() {
  MsgStream log(msgSvc(), name());

  // Reset the number of events written
  m_events = 0;
  // Get access to the DataManagerSvc
  m_pDataManager = serviceLocator()->service(m_storeName);
  if( !m_pDataManager )   {
    log << MSG::FATAL << "Unable to locate IDataManagerSvc interface" << endmsg;
    return StatusCode::FAILURE;
  }
  // Get access to the IncidentService
  m_incidentSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incidentSvc )  {
    log << MSG::WARNING << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  // Get access to the assigned data service
  m_pDataProvider = serviceLocator()->service(m_storeName);
  if( !m_pDataProvider )   {
    log << MSG::FATAL << "Unable to locate IDataProviderSvc interface of " << m_storeName << endmsg;
    return StatusCode::FAILURE;
  }
  if ( hasInput() )  {
    StatusCode status = connectConversionSvc();
    if( !status.isSuccess() )   {
      log << MSG::FATAL << "Unable to connect to conversion service." << endmsg;
      if(m_outputName!="" && m_fireIncidents) m_incidentSvc->fireIncident(Incident(m_outputName,
                                                                                   IncidentType::FailOutputFile));
      return status;
    }
  }

  // Clear the list with optional items
  clearItems(m_optItemList);
  // Clear the item list
  clearItems(m_itemList);

  // Take the new item list from the properties.
  ON_DEBUG log << MSG::DEBUG << "ItemList    : " << m_itemNames << endmsg;
  for( const auto& i : m_itemNames ) addItem( m_itemList, i );

  // Take the new item list from the properties.
  ON_DEBUG log << MSG::DEBUG << "OptItemList : " << m_optItemNames << endmsg;
  for( const auto& i : m_optItemNames ) addItem( m_optItemList, i );

  // prepare the algorithm selected dependent locations
  ON_DEBUG log << MSG::DEBUG << "AlgDependentItemList : " << m_algDependentItemList << endmsg;
  for ( const auto& a : m_algDependentItemList )
  {
    // Get the algorithm pointer
    Algorithm * theAlgorithm = decodeAlgorithm( a.first );
    if ( theAlgorithm )
    {
      // Get the item list for this alg
      auto& items = m_algDependentItems[theAlgorithm];
      // Clear the list for this alg
      clearItems( items );
      // fill the list again
      for ( const auto& i : a.second ) addItem( items, i );
    }
  }

  // Take the item list to the data service preload list.
  if ( m_doPreLoad )    {
    for(auto& j : m_itemList) m_pDataProvider->addPreLoadItem( *j ).ignore();
    // Not working: bad reference counting! pdataSvc->release();
  }

  if ( m_doPreLoadOpt )    {
    for(auto& j : m_optItemList) m_pDataProvider->addPreLoadItem( *j ).ignore();
  }
  log << MSG::INFO << "Data source: " << m_storeName  << " output: " << m_output << endmsg;

  // Decode the accept, required and veto Algorithms. The logic is the following:
  //  a. The event is accepted if all lists are empty.
  //  b. The event is provisionally accepted if any Algorithm in the accept list
  //     has been executed and has indicated that its filter is passed. This
  //     provisional acceptance can be overridden by the other lists.
  //  c. The event is rejected unless all Algorithms in the required list have
  //     been executed and have indicated that their filter passed.
  //  d. The event is rejected if any Algorithm in the veto list has been
  //     executed and has indicated that its filter has passed.
  decodeAcceptAlgs ().ignore();
  decodeRequireAlgs().ignore();
  decodeVetoAlgs   ().ignore();
  return StatusCode::SUCCESS;
}

// terminate data writer
StatusCode OutputStream::finalize() {
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Events output: " << m_events << endmsg;
  if(m_fireIncidents) m_incidentSvc->fireIncident(Incident(m_outputName,
                                                           IncidentType::EndOutputFile));
  m_incidentSvc.reset();
  m_pDataProvider.reset();
  m_pDataManager.reset();
  m_pConversionSvc.reset();
  clearItems(m_optItemList);
  clearItems(m_itemList);
  return StatusCode::SUCCESS;
}

// Work entry point
StatusCode OutputStream::execute()
{
  // Clear any previously existing item list
  clearSelection();
  // Test whether this event should be output
  if ( isEventAccepted() )
  {
    const StatusCode sc = writeObjects();
    clearSelection();
    ++m_events;
    if ( sc.isSuccess() && m_fireIncidents )
    {
      m_incidentSvc->fireIncident(Incident(m_outputName,
                                           IncidentType::WroteToOutputFile));
    }
    else if ( m_fireIncidents )
    {
      m_incidentSvc->fireIncident(Incident(m_outputName,
                                           IncidentType::FailOutputFile));
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
  if ( status.isSuccess() )
  {
    IDataSelector*  sel = selectedObjects();
    if ( sel->begin() != sel->end() )
    {
      status = m_pConversionSvc->connectOutput(m_outputName, m_outputType);
      if ( status.isSuccess() )
      {
        // Now pass the collection to the persistency service
        IOpaqueAddress* pAddress = nullptr;
        for ( auto& j : *sel )
        {
          try
          {
            const StatusCode iret = m_pConversionSvc->createRep( j, pAddress );
            if ( !iret.isSuccess() )
            {
              status = iret;
              continue;
            }
            IRegistry* pReg = j->registry();
            pReg->setAddress(pAddress);
          }
          catch ( const std::exception & excpt )
          {
            MsgStream log( msgSvc(), name() );
            const std::string loc = ( j->registry() ?
                                      j->registry()->identifier() : "UnRegistered" );
            log << MSG::FATAL
                << "std::exception during createRep for '" << loc << "' "
                << System::typeinfoName( typeid(*j) )
                << endmsg;
            log << MSG::FATAL << excpt.what() << endmsg;
            throw;
          }
        }
        for ( auto& j : *sel )
        {
          try
          {
            IRegistry* pReg = j->registry();
            const StatusCode iret = m_pConversionSvc->fillRepRefs( pReg->address(), j );
            if ( !iret.isSuccess() ) status = iret;
          }
          catch ( const std::exception & excpt )
          {
            MsgStream log( msgSvc(), name() );
            const std::string loc = ( j->registry() ?
                                      j->registry()->identifier() : "UnRegistered" );
            log << MSG::FATAL
                << "std::exception during fillRepRefs for '" << loc << "'"
                << System::typeinfoName( typeid(*j) )
                << endmsg;
            log << MSG::FATAL << excpt.what() << endmsg;
            throw;
          }
        }
        // Commit the data if there was no error; otherwise possibly discard
        if ( status.isSuccess() )
        {
          status = m_pConversionSvc->commitOutput(m_outputName, true);
        }
        else
        {
          m_pConversionSvc->commitOutput(m_outputName, false).ignore();
        }
      }
    }
  }
  return status;
}

// Place holder to create configurable data store agent
bool OutputStream::collect(IRegistry* dir, int level)    {
  if ( level < m_currentItem->depth() )   {
    if ( dir->object() )   {
      /*
        std::cout << "Analysing ("
        << dir->name()
        << ") Object:"
        << ((dir->object()==0) ? "UNLOADED" : "LOADED")
        << std::endl;
      */
      m_objects.push_back(dir->object());
      return true;
    }
  }
  return false;
}

/// Collect all objects to be written to the output stream
StatusCode OutputStream::collectObjects()   {
  MsgStream log(msgSvc(), name());
  StatusCode status = StatusCode::SUCCESS;

  // Traverse the tree and collect the requested objects
  for ( auto& i : m_itemList) {
    DataObject* obj = nullptr;
    m_currentItem = i;
    StatusCode iret = m_pDataProvider->retrieveObject(m_currentItem->path(), obj);
    if ( iret.isSuccess() )  {
      iret = m_pDataManager->traverseSubTree(obj, m_agent.get());
      if ( !iret.isSuccess() )  status = iret;
    }
    else  {
      log << MSG::ERROR << "Cannot write mandatory object(s) (Not found) "
          << m_currentItem->path() << endmsg;
      status = iret;
    }
  }

  // Traverse the tree and collect the requested objects (tolerate missing items here)
  for ( auto&  i : m_optItemList ) {
    DataObject* obj = nullptr;
    m_currentItem = i;
    StatusCode iret = m_pDataProvider->retrieveObject(m_currentItem->path(), obj);
    if ( iret.isSuccess() )  {
      iret = m_pDataManager->traverseSubTree(obj, m_agent.get());
    }
    if ( !iret.isSuccess() )    {
      ON_DEBUG
        log << MSG::DEBUG << "Ignore request to write non-mandatory object(s) "
            << m_currentItem->path() << endmsg;
    }
  }

  // Collect objects dependent on particular algorithms
  for ( const auto& iAlgItems : m_algDependentItems )
  {
    Algorithm * alg    = iAlgItems.first;
    const Items& items = iAlgItems.second;
    if ( alg->isExecuted() && alg->filterPassed() )
    {
      ON_DEBUG
        log << MSG::DEBUG << "Algorithm '" << alg->name() << "' fired. Adding " << items << endmsg;
      for ( const auto& i : items )
      {
        DataObject* obj = nullptr;
        m_currentItem = i;
        StatusCode iret = m_pDataProvider->retrieveObject(m_currentItem->path(),obj);
        if ( iret.isSuccess() )
        {
          iret = m_pDataManager->traverseSubTree(obj,m_agent.get());
          if ( !iret.isSuccess() ) status = iret;
        }
        else
        {
          log << MSG::ERROR << "Cannot write mandatory (algorithm dependent) object(s) (Not found) "
              << m_currentItem->path() << endmsg;
          status = iret;
        }
      }
    }
  }

  if (status.isSuccess())
  {
    // Remove duplicates from the list of objects, preserving the order in the list
    std::set<DataObject*> unique;
    std::vector<DataObject*> tmp; // temporary vector with the reduced list
    tmp.reserve(m_objects.size());
    for (auto& o : m_objects ) {
      if (!unique.count(o)) {
        // if the pointer is not in the set, add it to both the set and the temporary vector
        unique.insert(o);
        tmp.push_back(o);
      }
    }
    m_objects.swap(tmp); // swap the data of the two vectors
  }

  return status;
}

// Clear collected object list
void OutputStream::clearSelection()     {
  m_objects.clear();
}

// Remove all items from the output streamer list;
void OutputStream::clearItems(Items& itms)     {
  for ( auto& i : itms ) delete i;
  itms.clear();
}

// Find single item identified by its path (exact match)
DataStoreItem*
OutputStream::findItem(const std::string& path)  {
  auto matchPath = [&](const DataStoreItem* i) { return i->path() == path; } ;
  auto i = std::find_if( m_itemList.begin(), m_itemList.end(), matchPath );
  if (i == m_itemList.end()) {
     i = std::find_if( m_optItemList.begin(), m_optItemList.end(), matchPath );
     if (i == m_optItemList.end()) return nullptr;
  }
  return *i;
}

// Add item to output streamer list
void OutputStream::addItem(Items& itms, const std::string& descriptor)   {
  MsgStream log(msgSvc(), name());
  int level = 0;
  auto  sep = descriptor.rfind("#");
  std::string obj_path = descriptor.substr(0,sep);
  if ( sep != std::string::npos ) {
    std::string slevel = descriptor.substr(sep+1);
    level = ( slevel == "*" ) ? 9999999 : std::stoi(slevel);
  }
  if ( m_verifyItems )  {
    size_t idx = obj_path.find("/",1);
    while(idx != std::string::npos)  {
      std::string sub_item = obj_path.substr(0,idx);
      if ( 0 == findItem(sub_item) ) addItem(itms, sub_item+"#1");
      idx = obj_path.find("/",idx+1);
    }
  }
  itms.push_back( new DataStoreItem(obj_path, level) );
  const auto& item = itms.back();
  ON_DEBUG
    log << MSG::DEBUG << "Adding OutputStream item " << item->path()
        << " with " << item->depth()
        << " level(s)." << endmsg;
}

// Connect to proper conversion service
StatusCode OutputStream::connectConversionSvc()   {
  StatusCode status = StatusCode(StatusCode::FAILURE, true);
  MsgStream log(msgSvc(), name());
  // Get output file from input
  std::string dbType, svc, shr;
  for(auto attrib: Gaudi::Utils::AttribStringParser(m_output)) {
    const std::string& tag = attrib.tag;
    const std::string& val = attrib.value;
    switch( ::toupper(tag[0]) )    {
    case 'D':
      m_outputName = val;
      break;
    case 'T':
      dbType = val;
      break;
    case 'S':
      switch( ::toupper(tag[1]) )   {
      case 'V':    svc = val;      break;
      case 'H':    shr = "YES";    break;
      }
      break;
    case 'O':   // OPT='<NEW<CREATE,WRITE,RECREATE>, UPDATE>'
      switch( ::toupper(val[0]) )   {
      case 'R':
        if ( ::strncasecmp(val.c_str(),"RECREATE",3)==0 )
          m_outputType = "RECREATE";
        else if ( ::strncasecmp(val.c_str(),"READ",3)==0 )
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
  if ( !dbType.empty() || !svc.empty() )   {
    std::string typ = !dbType.empty() ? dbType : svc;
    auto ipers = serviceLocator()->service<IPersistencySvc>(m_persName);
    if( !ipers )   {
      log << MSG::FATAL << "Unable to locate IPersistencySvc interface of " << m_persName << endmsg;
      return StatusCode::FAILURE;
    }
    IConversionSvc *cnvSvc = nullptr;
    status = ipers->getService(typ, cnvSvc);
    if( !status.isSuccess() )   {
      log << MSG::FATAL << "Unable to locate IConversionSvc interface of database type " << typ << endmsg;
      return status;
    }
    // Increase reference count and keep service.
    m_pConversionSvc = cnvSvc;
  }
  else
  {
    log << MSG::FATAL
        << "Unable to locate IConversionSvc interface (Unknown technology) " << endmsg
        << "You either have to specify a technology name or a service name!" << endmsg
        << "Please correct the job option \"" << name() << ".Output\" !"     << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode OutputStream::decodeAcceptAlgs( ) {
  MsgStream log(msgSvc(), name());
  ON_DEBUG
    log << MSG::DEBUG << "AcceptAlgs  : " << m_acceptNames.value() << endmsg;
  return decodeAlgorithms( m_acceptNames, m_acceptAlgs );
}

void OutputStream::acceptAlgsHandler( Property& /* theProp */ )  {
  StatusCode sc = decodeAlgorithms( m_acceptNames, m_acceptAlgs );
  if (sc.isFailure()) {
    throw GaudiException("Failure in OutputStream::decodeAlgorithms",
                         "OutputStream::acceptAlgsHandler",sc);
  }
}

StatusCode OutputStream::decodeRequireAlgs( )  {
  MsgStream log(msgSvc(), name());
  ON_DEBUG
    log << MSG::DEBUG << "RequireAlgs : " << m_requireNames.value() << endmsg;
  return decodeAlgorithms( m_requireNames, m_requireAlgs );
}

void OutputStream::requireAlgsHandler( Property& /* theProp */ )  {
  StatusCode sc = decodeAlgorithms( m_requireNames, m_requireAlgs );
  if (sc.isFailure()) {
    throw GaudiException("Failure in OutputStream::decodeAlgorithms",
                         "OutputStream::requireAlgsHandler",sc);
  }
}

StatusCode OutputStream::decodeVetoAlgs( )  {
  MsgStream log(msgSvc(), name());
  ON_DEBUG
    log << MSG::DEBUG << "VetoAlgs    : " << m_vetoNames.value() << endmsg;
  return decodeAlgorithms( m_vetoNames, m_vetoAlgs );
}

void OutputStream::vetoAlgsHandler( Property& /* theProp */ )  {
  StatusCode sc = decodeAlgorithms( m_vetoNames, m_vetoAlgs );
  if (sc.isFailure()) {
    throw GaudiException("Failure in OutputStream::decodeAlgorithms",
                         "OutputStream::vetoAlgsHandler",sc);
  }
}

Algorithm* OutputStream::decodeAlgorithm( const std::string& theName )
{
  Algorithm * theAlgorithm = nullptr;

  SmartIF<IAlgManager> theAlgMgr(serviceLocator());
  if ( theAlgMgr )
  {
    // Check whether the supplied name corresponds to an existing
    // Algorithm object.
    SmartIF<IAlgorithm> &theIAlg = theAlgMgr->algorithm(theName);
    if ( theIAlg )
    {
      try
      {
        theAlgorithm = dynamic_cast<Algorithm*>(theIAlg.get());
      }
      catch(...)
      {
        // do nothing
      }
    }
  }
  else
  {
    MsgStream log( msgSvc( ), name( ) );
    log << MSG::FATAL << "Can't locate ApplicationMgr!!!" << endmsg;
  }

  if ( !theAlgorithm )
  {
    MsgStream log( msgSvc( ), name( ) );
    log << MSG::WARNING
        << "Failed to decode Algorithm name " << theName << endmsg;
  }

  return theAlgorithm;
}

StatusCode OutputStream::decodeAlgorithms( StringArrayProperty& theNames,
                                           std::vector<Algorithm*>& theAlgs )
{
  // Reset the list of Algorithms
  theAlgs.clear( );

  StatusCode result = StatusCode::SUCCESS;

  // Build the list of Algorithms from the names list
  for ( const auto& it : theNames.value() ) 
  {

    Algorithm * theAlgorithm = decodeAlgorithm( it );
    if ( theAlgorithm )
    {
      // Check that the specified algorithm doesn't already exist in the list
      if ( std::find( std::begin(theAlgs), std::end(theAlgs), theAlgorithm ) == std::end(theAlgs) ) {
        theAlgorithm->addRef();
        theAlgs.push_back( theAlgorithm );
      }
    }
    else
    {
      MsgStream log( msgSvc( ), name( ) );
      log << MSG::INFO << it << " doesn't exist - ignored" << endmsg;
    }

  }
  result = StatusCode::SUCCESS;

  return result;
}

bool OutputStream::isEventAccepted( ) const  {
  auto passed = [](const Algorithm* alg) { return alg->isExecuted() 
                                               && alg->filterPassed(); };

  // Loop over all Algorithms in the accept list to see
  // whether any have been executed and have their filter
  // passed flag set. Any match causes the event to be
  // provisionally accepted.
  bool result = m_acceptAlgs.empty() ||
           std::any_of( std::begin(m_acceptAlgs), std::end(m_acceptAlgs), passed);

  // Loop over all Algorithms in the required list to see
  // whether all have been executed and have their filter
  // passed flag set. Any mismatch causes the event to be
  // rejected.
  if ( result && !m_requireAlgs.empty() ) {
    result = std::all_of( std::begin(m_requireAlgs), std::end(m_requireAlgs), passed);
  }

  // Loop over all Algorithms in the veto list to see
  // whether any have been executed and have their filter
  // passed flag set. Any match causes the event to be
  // rejected.
  if ( result && !m_vetoAlgs.empty() ) {
    result = std::none_of( std::begin(m_vetoAlgs), std::end(m_vetoAlgs), passed);
  }
  return result;
}

bool OutputStream::hasInput() const {
  return !(m_itemNames.empty() && m_optItemNames.empty() &&
           m_algDependentItemList.empty());
}

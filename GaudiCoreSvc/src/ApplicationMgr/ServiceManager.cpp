// Include files
#include "ServiceManager.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Incident.h"

#include <iostream>
#include <cassert>

#define ON_DEBUG if (msgLevel(MSG::DEBUG))
#define ON_VERBOSE if (msgLevel(MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

using ROOT::Reflex::PluginService;

/// needed when no service is found or could be returned
static SmartIF<IService> no_service;

// constructor
ServiceManager::ServiceManager(IInterface* application):
  base_class(application, IService::interfaceID()),
  m_loopCheck(true),
  m_appSvc(application)
{
  // Set the service locator to myself
  m_svcLocator = static_cast<ISvcLocator*>(this);

  addRef(); // Initial count set to 1
}

// destructor
ServiceManager::~ServiceManager() {
  //-- inform the orphan services that I am gone....
  for (ListSvc::iterator it = m_listsvc.begin(); it != m_listsvc.end(); it++ ) {
    it->service->setServiceManager(0);
  }
}

//------------------------------------------------------------------------------
// Instantiate a service
SmartIF<IService>& ServiceManager::createService(const Gaudi::Utils::TypeNameString& typeName)
//------------------------------------------------------------------------------
{
  // Check if the service is already existing
  if(existsService(typeName.name())) {
    // return an error because a service with that name already exists
    return no_service;
  }

  /// @FIXME: check how this hack works
  StatusCode rc = StatusCode::FAILURE;
  rc.setChecked(); //hack to avoid going into infinite recursion on ~StatusCode

  const std::string &name = typeName.name();
  std::string type = typeName.type();
  if (!typeName.haveType()) { // the type is not explicit
    // see we have some specific type mapping for the name
    MapType::iterator it = m_maptype.find(typeName.name());
    if( it != m_maptype.end() ) {
      type = (*it).second; // use the declared type
    }
  }

  /// @FIXME: what does this mean?
  std::string::size_type ip;
  if ( (ip = type.find("__")) != std::string::npos) {
    type.erase(ip,type.length());
  }

  IService* service = PluginService::Create<IService*>(type, name, static_cast<ISvcLocator*>(this)); // serviceLocator().get());
  if ( !service ) {
    service = PluginService::CreateWithId<IService*>(type, name, static_cast<ISvcLocator*>(this)); // serviceLocator().get());
  }

  if ( service ) {
    m_listsvc.push_back(service);
    // Check the compatibility of the version of the interface obtained
    if( !isValidInterface(service) ) {
      fatal() << "Incompatible interface IService version for " << type << endmsg;
      return no_service;
    }
    service->setServiceManager(this);
    return m_listsvc.back().service;
  }
  fatal() << "No Service factory for " << type << " available." << endmsg;
  return no_service;
}

//------------------------------------------------------------------------------
// add a service to the managed list
StatusCode ServiceManager::addService(IService* svc, int prio)
//------------------------------------------------------------------------------
{
  ListSvc::iterator it = find(svc);
  if (it != m_listsvc.end()) {
    it->priority = prio; // if the service is already known, it is equivalent to a setPriority
    it->active = true;   // and make it active
  } else {
    m_listsvc.push_back(ServiceItem(svc,prio,true));
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
// add the service with the give type and name to the active list
StatusCode ServiceManager::addService(const Gaudi::Utils::TypeNameString& typeName, int prio)
//------------------------------------------------------------------------------
{
  ListSvc::iterator it = find(typeName.name()); // try to find the service by name
  if (it == m_listsvc.end()) { // not found
    // If the service does not exist, we create it
    SmartIF<IService> &svc = createService(typeName);
    if (svc.isValid()) {
      it = find(svc.get()); // now it is in the list because createService added it
      it->priority = prio;
      StatusCode sc = StatusCode(StatusCode::SUCCESS, true);
      if (targetFSMState() >= Gaudi::StateMachine::INITIALIZED) {
        sc = svc->sysInitialize();
        if (sc.isSuccess() && targetFSMState() >= Gaudi::StateMachine::RUNNING) {
          sc = svc->sysStart();
        }
      }
      if(sc.isFailure()) { // if initialization failed, remove it from the list
        error() << "Unable to initialize service \"" << typeName.name() << "\""
                << endmsg;
        m_listsvc.erase(it);
        // Note: removing it from the list + the SmartIF going out of scope should trigger the delete
        // delete svc.get();
        return sc;
      } else { // initialization successful, we can work with the service
        // Move the just initialized service to the back of the list
        // (we care more about order of initialization than of creation)
        m_listsvc.push_back(*it);
        m_listsvc.erase(it);
        it = --m_listsvc.end(); // last entry (the iterator was invalidated by erase)
      }
    } else {
      return StatusCode::FAILURE;
    }
  } else {
    // if the service is already known, it is equivalent to a setPriority
    it->priority = prio;
  }
  // 'it' is defined because either we found the service or we created it
  // Now we can activate the service
  it->active = true;   // and make it active
  return StatusCode(StatusCode::SUCCESS, true);
}

//------------------------------------------------------------------------------
// Returns a smart pointer to a service.
SmartIF<IService> &ServiceManager::service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf) {
  const std::string &name = typeName.name();

  // Acquire the RAII lock to avoid simultaneous attempts from different threads to initialize a service
  boost::lock_guard<boost::recursive_mutex> lck(m_svcinitmutex);

  ListSvc::iterator it = find(name);

  if (it !=  m_listsvc.end()) {
    if (m_loopCheck &&
        (createIf && it->service->FSMState() == Gaudi::StateMachine::CONFIGURED)) {
      error()
          << "Initialization loop detected when creating service \"" << name
          << "\""
          << endmsg;
      return no_service;
    }
    return it->service;
  } else {
    // Service not found. The user may be interested in one of the interfaces
    // of the application manager itself
    if( name == "ApplicationMgr" ||
        name == "APPMGR" ||
        name == "" ) {
      return m_appSvc;
    } else if ( createIf ){
      //last resort: we try to create the service
      if (addService(typeName).isSuccess()){
        return find(name)->service;
      }
    }
  }
  return no_service;
}

//------------------------------------------------------------------------------
const std::list<IService*>& ServiceManager::getServices( ) const
//------------------------------------------------------------------------------
{
  m_listOfPtrs.clear();
  for (ListSvc::const_iterator it = m_listsvc.begin(); it != m_listsvc.end(); ++it) {
    m_listOfPtrs.push_back(const_cast<IService*>(it->service.get()));
  }
  return m_listOfPtrs;
}

//------------------------------------------------------------------------------
bool ServiceManager::existsService( const std::string& name) const
//------------------------------------------------------------------------------
{
  return find(name) != m_listsvc.end();
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::removeService(IService* svc)
//------------------------------------------------------------------------------
{
  ListSvc::iterator it = find(svc);
  if (it != m_listsvc.end()) {
    m_listsvc.erase(it);
    return StatusCode(StatusCode::SUCCESS,true);
  }
  return StatusCode(StatusCode::FAILURE,true);
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::removeService(const std::string& name)
//------------------------------------------------------------------------------
{
  ListSvc::iterator it = find(name);
  if (it != m_listsvc.end()) {
    m_listsvc.erase(it);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::declareSvcType( const std::string& svcname,
                                           const std::string& svctype )
//------------------------------------------------------------------------------
{
  std::pair<MapType::iterator, bool> p = m_maptype.insert(std::make_pair(svcname, svctype));
  if( p.second == false) {
    m_maptype.erase ( p.first );
    p = m_maptype.insert(std::make_pair(svcname, svctype) );
    if( p.second == false) return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::initialize()
//------------------------------------------------------------------------------
{
  m_listsvc.sort(); // ensure that the list is ordered by priority
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  ListSvc tmpList(m_listsvc);

  StatusCode sc(StatusCode::SUCCESS, true);
  // call initialize() for all services
  for (ListSvc::iterator it = tmpList.begin(); it != tmpList.end(); ++it ) {
    if (!it->active) continue; // only act on active services
    const std::string& name = it->service->name();
    switch (it->service->FSMState()) {
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Service " << name << " already initialized" << endmsg;
      break;
    case Gaudi::StateMachine::OFFLINE:
      DEBMSG << "Initializing service " << name << endmsg;
      sc = it->service->sysInitialize();
      if( !sc.isSuccess() ) {
        error() << "Unable to initialize Service: " << name << endmsg;
        return sc;
      } break;
    default:
      error() << "Service " << name
          << " not in the correct state to be initialized ("
          << it->service->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::start()
//------------------------------------------------------------------------------
{
  m_listsvc.sort(); // ensure that the list is ordered by priority
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  ListSvc tmpList(m_listsvc);

  StatusCode sc(StatusCode::SUCCESS, true);
  // call initialize() for all services
  for (ListSvc::iterator it = tmpList.begin(); it != tmpList.end(); ++it ) {
    if (!it->active) continue; // only act on active services
    const std::string& name = it->service->name();
    switch (it->service->FSMState()) {
    case Gaudi::StateMachine::RUNNING:
      DEBMSG << "Service " << name
          << " already started" << endmsg;
      break;
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Starting service " << name << endmsg;
      sc = it->service->sysStart();
      if( !sc.isSuccess() ) {
        error() << "Unable to start Service: " << name << endmsg;
        return sc;
      } break;
    default:
      error() << "Service " << name
          << " not in the correct state to be started ("
          << it->service->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode ServiceManager::stop()
//------------------------------------------------------------------------------
{
  m_listsvc.sort(); // ensure that the list is ordered by priority
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  ListSvc tmpList(m_listsvc);

  StatusCode sc(StatusCode::SUCCESS, true);
  ListSvc::reverse_iterator it;
  // call stop() for all services
  for (it = tmpList.rbegin(); it != tmpList.rend(); ++it ) {
    if (!it->active) continue; // only act on active services
    const std::string& name = it->service->name();
    switch (it->service->FSMState()) {
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Service " << name << " already stopped" << endmsg;
      break;
    case Gaudi::StateMachine::RUNNING:
      DEBMSG << "Stopping service " << name << endmsg;
      sc = it->service->sysStop();
      if( !sc.isSuccess() ) {
        error() << "Unable to stop Service: " << name << endmsg;
        return sc;
      } break;
    default:
      DEBMSG << "Service " << name
          << " not in the correct state to be stopped ("
          << it->service->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::reinitialize()
//------------------------------------------------------------------------------
{
  m_listsvc.sort(); // ensure that the list is ordered by priority
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  ListSvc tmpList(m_listsvc);

  StatusCode sc(StatusCode::SUCCESS, true);
  ListSvc::iterator it;
  // Re-Initialize all services
  for ( it = tmpList.begin(); it != tmpList.end(); ++it ) {
    if (!it->active) continue; // only act on active services
    sc = it->service->sysReinitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to re-initialize Service: " << it->service->name() << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::restart()
//------------------------------------------------------------------------------
{
  m_listsvc.sort(); // ensure that the list is ordered by priority
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  ListSvc tmpList(m_listsvc);

  StatusCode sc(StatusCode::SUCCESS, true);
  ListSvc::iterator it;
  // Re-Start all services
  for ( it = tmpList.begin(); it != tmpList.end(); ++it ) {
    if (!it->active) continue; // only act on active services
    sc = it->service->sysRestart();
    if( !sc.isSuccess() ) {
      error() << "Unable to re-start Service: " << it->service->name() << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::finalize()
//------------------------------------------------------------------------------
{
  // make sure that HistogramDataSvc and THistSvc get finalized after the
  // ToolSvc, and the FileMgr and StatusCodeSvc after that
  int pri_tool = getPriority("ToolSvc");
  if (pri_tool != 0) {
    setPriority("THistSvc",pri_tool-10).ignore();
    setPriority("ChronoStatSvc",pri_tool-20).ignore();
    setPriority("AuditorSvc",pri_tool-30).ignore();
    setPriority("NTupleSvc",pri_tool-10).ignore();
    setPriority("HistogramDataSvc",pri_tool-10).ignore();
    // Preserve the relative ordering between HistogramDataSvc and HistogramPersistencySvc
    setPriority("HistogramPersistencySvc",pri_tool-20).ignore();
    setPriority("HistorySvc",pri_tool-30).ignore();
    setPriority("FileMgr",pri_tool-40).ignore();
  }


  // get list of PostFinalize clients
  std::vector<IIncidentListener*> postFinList;
  {
    SmartIF<IIncidentSvc> p_inc(service("IncidentSvc",false));
    if (p_inc.isValid()) {
      p_inc->getListeners(postFinList,IncidentType::SvcPostFinalize);
    }
  }

  // make sure the StatusCodeSvc gets finalized really late:
  setPriority("StatusCodeSvc",-9999).ignore();

  m_listsvc.sort(); // ensure that the list is ordered by priority
  // dump();

  StatusCode sc(StatusCode::SUCCESS, true);
  {
    // we work on a copy to avoid to operate twice on the services created on demand
    // (which are already in the correct state).
    ListSvc tmpList(m_listsvc);

    // call finalize() for all services in reverse order
    ListSvc::reverse_iterator rit;
    for (rit = tmpList.rbegin(); rit != tmpList.rend(); ++rit ) {
      if (!rit->active) continue; // only act on active services
      const std::string& name = rit->service->name();
      // ignore the current state for the moment
      // if( Gaudi::StateMachine::INITIALIZED == rit->service->state() ) {
      DEBMSG << "Finalizing service " << name << endmsg;
      if ( !(rit->service->sysFinalize()).isSuccess() ) {
        warning() << "Finalization of service " << name << " failed" << endmsg;
        sc = StatusCode::FAILURE;
      }
    }
  }

  // call SvcPostFinalize on all clients
  if (!postFinList.empty()) {
    DEBMSG << "Will call SvcPostFinalize for " << postFinList.size() << " clients"
           << endmsg;
    Incident inc("ServiceManager", IncidentType::SvcPostFinalize);
    std::vector<IIncidentListener*>::iterator itr;
    for (itr = postFinList.begin(); itr != postFinList.end(); ++itr) {
      (*itr)->handle(inc);
    }
  }

  // loop over all Active Services, removing them one by one.
  // They should be deleted because the reference counting goes to 0.
  DEBMSG << "Looping over all active services..." << endmsg;
  ListSvc::iterator it = m_listsvc.begin();
  while (it != m_listsvc.end()) {
    DEBMSG << "---- " << it->service->name()
	   << " (refCount = " << it->service->refCount() << ")"
	   << endmsg;
    if (it->service->refCount() < 1) {
      warning() << "Too low reference count for " << it->service->name()
                << " (should not go below 1 at this point)" << endmsg;
      it->service->addRef();
    }
    if (it->active) {
      it = m_listsvc.erase(it);
    } else {
      ++it;
    }
  }

  return sc;
}


//------------------------------------------------------------------------------
int
ServiceManager::getPriority(const std::string& name) const {
//------------------------------------------------------------------------------
  ListSvc::const_iterator it = find(name);
  return (it != m_listsvc.end()) ? it->priority: 0;
}

//------------------------------------------------------------------------------
StatusCode
ServiceManager::setPriority(const std::string& name, int prio) {
//------------------------------------------------------------------------------
  ListSvc::iterator it = find(name);
  if (it != m_listsvc.end()) {
    it->priority = prio;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
// Get the value of the initialization loop check flag.
//------------------------------------------------------------------------------
bool ServiceManager::loopCheckEnabled() const {
  return m_loopCheck;
}
//------------------------------------------------------------------------------
// Set the value of the initialization loop check flag.
//------------------------------------------------------------------------------
void ServiceManager::setLoopCheckEnabled(bool en) {
  m_loopCheck = en;
}


//------------------------------------------------------------------------------
// Dump out contents of service list
//------------------------------------------------------------------------------
void ServiceManager::dump() const {

  MsgStream log(msgSvc(),name());

  log << MSG::INFO << "\n"
      << "=====================  listing all services  ===================\n"
      << " prior   ref name                           active\n";

  ListSvc::const_iterator it;
  for (it=m_listsvc.begin(); it != m_listsvc.end(); ++it) {

    log.width(6);
    log.flags(std::ios_base::right);
    log << it->priority << " ";
    log.width(5);
    log << it->service->refCount() << " ";
    log.width(30);
    log.flags(std::ios_base::left);
    log << it->service->name() << " ";
    log.width(2);
    log << it->active << std::endl;

  }

  log << "=================================================================\n";
  log << endmsg;

}


DECLARE_OBJECT_FACTORY(ServiceManager)

// Include files
#include "ServiceManager.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/reverse.h"

#include <iostream>
#include <cassert>
#include <algorithm>
#include <functional>

#define ON_DEBUG if (msgLevel(MSG::DEBUG))
#define ON_VERBOSE if (msgLevel(MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

/// needed when no service is found or could be returned
static SmartIF<IService> no_service;

/// utility for various calls...
namespace {
    template <typename C>
    std::vector<IService*> activeSvc(const C& lst) {
       std::vector<IService*> v; v.reserve(lst.size());
       for(auto& i : lst) {
           if (i.active) v.push_back(i.service.get());
       }
       return v;
    }
}

// constructor
ServiceManager::ServiceManager(IInterface* application):
  base_class(application, IService::interfaceID()),
  m_appSvc(application)
{
  // Set the service locator to myself
  m_svcLocator = this;
  addRef(); // increase ref count, so we live forever...
}

// destructor
ServiceManager::~ServiceManager() {
  //-- inform the orphan services that I am gone....
  for (auto& svc : m_listsvc) svc.service->setServiceManager(nullptr);
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

  const std::string &name = typeName.name();
  std::string type = typeName.type();
  if (!typeName.haveType()) { // the type is not explicit
    // see we have some specific type mapping for the name
    auto it = m_maptype.find(typeName.name());
    if( it != m_maptype.end() ) {
      type = it->second; // use the declared type
    }
  }

  /// @FIXME: what does this mean?
  auto ip = type.find("__");
  if ( ip != std::string::npos) type.erase(ip,type.length());

  IService* service = Service::Factory::create(type, name, this);
  if ( !service ) {
    fatal() << "No Service factory for " << type << " available." << endmsg;
    return no_service;
  }
  // Check the compatibility of the version of the interface obtained
  if( !isValidInterface(service) ) {
    fatal() << "Incompatible interface IService version for " << type << endmsg;
    return no_service;
  }

  m_listsvc.push_back(service);
  service->setServiceManager(this);
  return m_listsvc.back().service; // DANGER: returns a reference to a SmartIF in m_listsvc, and hence does no longer allow relocations of those...
}

//------------------------------------------------------------------------------
// add a service to the managed list
StatusCode ServiceManager::addService(IService* svc, int prio)
//------------------------------------------------------------------------------
{
  ListSvc::iterator it = find(svc);
  LockGuard_t lck(m_gLock);
  if (it != m_listsvc.end()) {
    it->priority = prio; // if the service is already known, it is equivalent to a setPriority
    it->active = true;   // and make it active
  } else {
    m_listsvc.emplace_back(svc,prio,true);
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
// add the service with the give type and name to the active list
StatusCode ServiceManager::addService(const Gaudi::Utils::TypeNameString& typeName, int prio)
//------------------------------------------------------------------------------
{
  auto it = find(typeName.name()); // try to find the service by name
  if (it == m_listsvc.end()) { // not found
    // If the service does not exist, we create it
    SmartIF<IService>& svc = createService(typeName); // WARNING: svc is now a reference to something that lives in m_listsvc
    if (!svc) return StatusCode::FAILURE;
    it = find(svc.get()); // now it is in the list because createService added it
    it->priority = prio;
    StatusCode sc = StatusCode(StatusCode::SUCCESS, true);
    if (targetFSMState() >= Gaudi::StateMachine::INITIALIZED) { // WARNING: this can trigger a recursion!!!
      sc = svc->sysInitialize();
      if (sc.isSuccess() && targetFSMState() >= Gaudi::StateMachine::RUNNING) {
        sc = svc->sysStart();
      }
    }
    if(sc.isFailure()) { // if initialization failed, remove it from the list
      error() << "Unable to initialize service \"" << typeName.name() << "\""
              << endmsg;
      LockGuard_t lck(m_gLock);
      m_listsvc.erase(it);
      // Note: removing it from the list + the SmartIF going out of scope should trigger the delete
      // delete svc.get();
      return sc;
    }
    // initialization successful, we can work with the service
    // Move the just initialized service to the back of the list
    // (we care more about order of initialization than of creation)
    LockGuard_t lck(m_gLock);
    m_listsvc.push_back(*it);
    m_listsvc.erase(it);
    it = std::prev(std::end(m_listsvc)); // last entry (the iterator was invalidated by erase)
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

  Mutex_t *imut;
  {
    // get the global lock, then extract/create the service specific mutex
    // then release global lock

    LockGuard_t lk(m_gLock);      
    auto mit = m_lockMap.find(name);
    if (mit == m_lockMap.end()) {
      mit = m_lockMap.emplace(name, std::unique_ptr<Mutex_t>( new Mutex_t )).first;
    }
    imut = mit->second.get();
  }

  {
    // now we have the service specific lock on the above mutex
    LockGuard_t lk2(*imut);

  auto it = find(name);

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
  }

  // Service not found. The user may be interested in one of the interfaces
  // of the application manager itself
  if( name == "ApplicationMgr" ||
      name == "APPMGR" ||
      name == "" ) {
    return m_appSvc;
  }

  //last resort: we try to create the service
  if ( createIf && addService(typeName).isSuccess()){
    return find(name)->service;
  }

  return no_service;
}
}

//------------------------------------------------------------------------------
const std::list<IService*>& ServiceManager::getServices( ) const
//------------------------------------------------------------------------------
{
  m_listOfPtrs.clear();
  std::transform( std::begin(m_listsvc), std::end(m_listsvc),
                  std::back_inserter(m_listOfPtrs),
                  [](ListSvc::const_reference i) {
                      return const_cast<IService*>(i.service.get());
  });
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
  auto it = find(svc);
  if (it == m_listsvc.end()) return StatusCode(StatusCode::FAILURE,true);
  m_listsvc.erase(it);
  return StatusCode(StatusCode::SUCCESS,true);
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::removeService(const std::string& name)
//------------------------------------------------------------------------------
{
  auto it = find(name);
  if (it == m_listsvc.end()) return StatusCode::FAILURE;
  m_listsvc.erase(it);
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::declareSvcType( const std::string& svcname,
                                           const std::string& svctype )
//------------------------------------------------------------------------------
{
  // once we get to C++17, replace with m_maptype.insert_or_assign...
  auto p = m_maptype.emplace( svcname, svctype );
  if ( !p.second ) p.first->second = svctype;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::initialize()
//------------------------------------------------------------------------------
{
  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // we work on a copy to avoid to operate twice on the services created on demand
  // which are already in the correct state.

  StatusCode sc(StatusCode::SUCCESS, true);
  // call initialize() for all services
  for (auto& it : activeSvc(m_listsvc) ) {
    const std::string& name = it->name();
    switch (it->FSMState()) {
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Service " << name << " already initialized" << endmsg;
      break;
    case Gaudi::StateMachine::OFFLINE:
      DEBMSG << "Initializing service " << name << endmsg;
      sc = it->sysInitialize();
      if( !sc.isSuccess() ) {
        error() << "Unable to initialize Service: " << name << endmsg;
        return sc;
      } break;
    default:
      error() << "Service " << name
          << " not in the correct state to be initialized ("
          << it->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::start()
//------------------------------------------------------------------------------
{
  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // we work on a copy to avoid to operate twice on the services created on demand
  // (which are already in the correct state.
  // only act on active services
  StatusCode sc(StatusCode::SUCCESS, true);
  // call initialize() for all services
  for (auto& it : activeSvc(m_listsvc)) {
    const std::string& name = it->name();
    switch (it->FSMState()) {
    case Gaudi::StateMachine::RUNNING:
      DEBMSG << "Service " << name
          << " already started" << endmsg;
      break;
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Starting service " << name << endmsg;
      sc = it->sysStart();
      if( !sc.isSuccess() ) {
        error() << "Unable to start Service: " << name << endmsg;
        return sc;
      } break;
    default:
      error() << "Service " << name
          << " not in the correct state to be started ("
          << it->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode ServiceManager::stop()
//------------------------------------------------------------------------------
{
  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // we work on a copy to avoid to operate twice on the services created on demand
  // which are already in the correct state.
  // only act on active services

  StatusCode sc(StatusCode::SUCCESS, true);
  // call stop() for all services
  for (const auto& svc : reverse(activeSvc(m_listsvc))) {
    const std::string& name = svc->name();
    switch (svc->FSMState()) {
    case Gaudi::StateMachine::INITIALIZED:
      DEBMSG << "Service " << name << " already stopped" << endmsg;
      break;
    case Gaudi::StateMachine::RUNNING:
      DEBMSG << "Stopping service " << name << endmsg;
      sc = svc->sysStop();
      if( !sc.isSuccess() ) {
        error() << "Unable to stop Service: " << name << endmsg;
        return sc;
      } break;
    default:
      DEBMSG << "Service " << name
          << " not in the correct state to be stopped ("
          << svc->FSMState() << ")" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::reinitialize()
//------------------------------------------------------------------------------
{
  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // we work on a copy to avoid to operate twice on the services created on demand
  // which are already in the correct state.
  // only act on active services
  StatusCode sc(StatusCode::SUCCESS, true);
  // Re-Initialize all services
  for (auto& svc : activeSvc(m_listsvc)) {
    sc = svc->sysReinitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to re-initialize Service: " << svc->name() << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ServiceManager::restart()
//------------------------------------------------------------------------------
{
  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // we work on a copy to avoid to operate twice on the services created on demand
  // which are already in the correct state.
  // only act on active services
  StatusCode sc(StatusCode::SUCCESS, true);
  // Re-Start all services
  for (auto& svc : activeSvc(m_listsvc)) {
    sc = svc->sysRestart();
    if( !sc.isSuccess() ) {
      error() << "Unable to re-start Service: " << svc->name() << endmsg;
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
  auto p_inc = service<IIncidentSvc>("IncidentSvc",false);
  if (p_inc) {
    p_inc->getListeners(postFinList,IncidentType::SvcPostFinalize);
    p_inc.reset();
  }

  // make sure the StatusCodeSvc gets finalized really late:
  setPriority("StatusCodeSvc",-9999).ignore();

  // ensure that the list is ordered by priority
  m_listsvc.sort();
  // dump();

  StatusCode sc(StatusCode::SUCCESS, true);
  {
    // we work on a copy to avoid to operate twice on the services created on demand
    // which are already in the correct state.
    // only act on active services
    // call finalize() for all services in reverse order
    for (const auto& svc : reverse(activeSvc(m_listsvc)) ) {
      const std::string& name = svc->name();
      // ignore the current state for the moment
      // if( Gaudi::StateMachine::INITIALIZED == svc->state() )
      DEBMSG << "Finalizing service " << name << endmsg;
      if ( !svc->sysFinalize().isSuccess() ) {
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
    for (auto& itr : postFinList) itr->handle(inc);
  }

  // loop over all Active Services, removing them one by one.
  // They should be deleted because the reference counting goes to 0.
  DEBMSG << "Looping over all active services..." << endmsg;
  auto it = m_listsvc.begin();
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
  auto it = find(name);
  return (it != m_listsvc.end()) ? it->priority: 0;
}

//------------------------------------------------------------------------------
StatusCode
ServiceManager::setPriority(const std::string& name, int prio) {
//------------------------------------------------------------------------------
  auto it = find(name);
  if (it == m_listsvc.end()) return StatusCode::FAILURE;
  it->priority = prio;
  return StatusCode::SUCCESS;
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

  for (const auto& svc : m_listsvc) {

    log.width(6);
    log.flags(std::ios_base::right);
    log << svc.priority << " ";
    log.width(5);
    log << svc.service->refCount() << " ";
    log.width(30);
    log.flags(std::ios_base::left);
    log << svc.service->name() << " ";
    log.width(2);
    log << svc.active << std::endl;

  }

  log << "=================================================================\n";
  log << endmsg;

}


DECLARE_OBJECT_FACTORY(ServiceManager)

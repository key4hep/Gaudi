#include "AlgExecStateSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"

#include "GaudiKernel/SvcFactory.h"

DECLARE_SERVICE_FACTORY(AlgExecStateSvc)

//=============================================================================


AlgExecStateSvc::AlgExecStateSvc( const std::string& name, ISvcLocator* svcLoc ):
base_class(name,svcLoc), m_isInit(false)
{

}

//-----------------------------------------------------------------------------

AlgExecStateSvc::~AlgExecStateSvc() {

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::init() {

  std::string wbn;

  // seriously? do we have no way of getting a Service by type???
  std::list<IService*> lst = serviceLocator()->getServices();
  for (auto &is : lst) {
    IHiveWhiteBoard *iwb = dynamic_cast<IHiveWhiteBoard*>(is);
    if (iwb != 0) {
      wbn = is->name();
      verbose() << "HiveWhiteBoard service name is " << wbn << endmsg;
      break;
    }
  }

  SmartIF<IHiveWhiteBoard> wbs;
  wbs = serviceLocator()->service(wbn,false);

  if (wbs.isValid()) {
    m_algStates.resize(wbs->getNumberOfStores());
    m_eventStatus.resize(wbs->getNumberOfStores());
  } else {
    m_algStates.resize(1);
    m_eventStatus.resize(1);
  }

  debug() << "resizing state containers to : " << m_algStates.size() << endmsg;

  SmartIF<IAlgManager> algMan( serviceLocator() );
  if (!algMan.isValid()) {
    fatal() << "could not get the AlgManager" << endmsg;
    throw GaudiException("In AlgExecStateSvc, unable to get the AlgManager!",
                         "AlgExecStateSvc",StatusCode::FAILURE);
  }

  m_isInit = true;

  auto algos = algMan->getAlgorithms();
  for (auto &alg : algos) {
    addAlg( alg );
  }

  for (auto &alg : m_preInitAlgs) {
    addAlg( alg );
  }


  if (msgLevel( MSG::VERBOSE )) {
    std::ostringstream ost;
    dump(ost);
    verbose() << "dumping state:\n" << ost.str() << endmsg;
  }

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::checkInit() const {

  if (!m_isInit) {
    fatal() << "AlgExecStateSvc not initialized before first use" << endmsg;
    throw GaudiException("AlgExecStateSvc not initialized before first use!",
                         "AlgExecStateSvc",StatusCode::FAILURE);
  }
}

//-----------------------------------------------------------------------------

StatusCode
AlgExecStateSvc::initialize() {

  // Initialise mother class (read properties, ...)
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess()) {
    warning () << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  // moved all initialization to init(). hopefully it will get called in time....

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------

StatusCode
AlgExecStateSvc::finalize() {

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::dump(std::ostringstream& ost) const {

  checkInit();

  ost << "Event: " << trans(m_eventStatus.at(0)) << std::endl;
  ost << "Algs:  " << m_algStates.at(0).size() << std::endl;

  size_t ml(0);
  for (auto &e : m_algStates.at(0)) {
    if ( e.first.str().length() > ml ) {
      ml = e.first.str().length();
    }
  }

  for (size_t is =0; is < m_algStates.size(); ++is) {
    ost << " - Slot " << is << std::endl;
    const AlgStateMap_t& m = m_algStates.at(is);
    for (auto &e : m) {
      ost << "  + " << std::setw(ml) << e.first.str() << "  " << e.second 
          << std::endl;
    }
  }

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::dump(std::ostringstream& ost, const EventContext& ctx) const {

  ost << "Event: " << trans(m_eventStatus.at(ctx.slot())) << std::endl;
  ost << "Algs: " << m_algStates.at(ctx.slot()).size() << std::endl;

  size_t ml(0);
  for (auto &e : m_algStates.at(ctx.slot())) {
    if ( e.first.str().length() > ml ) {
      ml = e.first.str().length();
    }
  }

  ost << " - Slot " << ctx.slot() << std::endl;
  const AlgStateMap_t& m = m_algStates.at(ctx.slot());
  for (auto &e : m) {
    ost << "  + " << std::setw(ml) << e.first.str() << "  " << e.second 
        << std::endl;
  }

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::addAlg(IAlgorithm* iAlg) { 
  return addAlg(iAlg->nameKey());
}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::addAlg(const Gaudi::StringKey& alg) {
  

  if (!m_isInit) {
    debug() << "preInit: will add Alg " << alg.str() << " later" << endmsg;
    m_preInitAlgs.push_back( alg );
    return;
  }

  if (m_algStates.size() > 0) {
    if (m_algStates.at(0).find(alg) != m_algStates.at(0).end()) {
      // already added
      return;
    }
  }

  {
    // in theory, this should only get called during initialization (serial)
    // so shouldn't have to protect with a mutex...
    std::lock_guard<std::mutex> lock(m_mut);
    
    AlgExecState s;
    for (size_t i = 0; i<m_algStates.size(); ++i) {
      m_algStates.at(i)[alg] = s;
    }
  }

  debug() << "adding alg " << alg.str() << " to " 
          << m_algStates.size() << " slots" << endmsg;
  
}

//-----------------------------------------------------------------------------

const AlgExecState& 
AlgExecStateSvc::algExecState(const Gaudi::StringKey& algName, 
                              const EventContext& ctx) const {

  checkInit();

  AlgStateMap_t::const_iterator itr = m_algStates.at(ctx.slot()).find( algName );

  if (UNLIKELY( itr == m_algStates.at(ctx.slot()).end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} 
      + algName.str() + " in AlgStateMap", name(), StatusCode::FAILURE};
  }

  return itr->second;

}

//-----------------------------------------------------------------------------

const AlgExecState& 
AlgExecStateSvc::algExecState(IAlgorithm* iAlg, 
                              const EventContext& ctx) const {

  return algExecState(iAlg->nameKey(), ctx);

}

//-----------------------------------------------------------------------------

AlgExecState& 
AlgExecStateSvc::algExecState(IAlgorithm* iAlg, 
                              const EventContext& ctx) {

  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);

  AlgStateMap_t::iterator itr = m_algStates.at(ctx.slot()).find( iAlg->nameKey() );

  if (UNLIKELY( itr == m_algStates.at(ctx.slot()).end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} 
      + iAlg->name() + " in AlgStateMap", name(), StatusCode::FAILURE};
  }

  return itr->second;

}

//-----------------------------------------------------------------------------

const IAlgExecStateSvc::AlgStateMap_t& 
AlgExecStateSvc::algExecStates(const EventContext& ctx) const {

  checkInit();

  return m_algStates.at(ctx.slot());
}

//-----------------------------------------------------------------------------

const AlgExecState& 
AlgExecStateSvc::algExecState(const Gaudi::StringKey& algName) const {

  checkInit();

  AlgStateMap_t::const_iterator itr = m_algStates.at(0).find( algName );

  if (UNLIKELY( itr == m_algStates.at(0).end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} 
      + algName.str() + " in AlgStateMap", name(), StatusCode::FAILURE};
  }

  return itr->second;

}

//-----------------------------------------------------------------------------

AlgExecState& 
AlgExecStateSvc::algExecState(IAlgorithm* iAlg) {

  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);

  AlgStateMap_t::iterator itr = m_algStates.at(0).find( iAlg->nameKey() );

  if (UNLIKELY( itr == m_algStates.at(0).end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} 
      + iAlg->name() + " in AlgStateMap", name(), StatusCode::FAILURE};
  }

  return itr->second;

}

//-----------------------------------------------------------------------------

const AlgExecState& 
AlgExecStateSvc::algExecState(IAlgorithm* iAlg) const {
  checkInit();
  return algExecState(iAlg->nameKey());
}

//-----------------------------------------------------------------------------

const IAlgExecStateSvc::AlgStateMap_t& 
AlgExecStateSvc::algExecStates() const {

  checkInit();

  return m_algStates.at(0);
}

//-----------------------------------------------------------------------------

const EventStatus::Status&
AlgExecStateSvc::eventStatus(const EventContext& ctx) const {
  checkInit();
  return m_eventStatus.at(ctx.slot());
}

//-----------------------------------------------------------------------------

const EventStatus::Status&
AlgExecStateSvc::eventStatus() const {
  checkInit();
  return m_eventStatus.at(0);
}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::setEventStatus(const EventStatus::Status& sc, const EventContext& ctx) {
  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  m_eventStatus.at(ctx.slot()) = sc;
}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::setEventStatus(const EventStatus::Status& sc) {
  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  m_eventStatus.at(0) = sc;
}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::updateEventStatus(const bool& fail) {
  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  if (m_eventStatus.at(0) == EventStatus::Success) {
    if (fail) 
      m_eventStatus.at(0) = EventStatus::AlgFail;
  } else if (m_eventStatus.at(0) == EventStatus::Invalid) {
    if (! fail) {
      m_eventStatus.at(0) = EventStatus::Success;
    } else {
      m_eventStatus.at(0) = EventStatus::AlgFail;
    }
  }
}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::updateEventStatus(const bool& fail, const EventContext& ctx) {
  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  if (m_eventStatus.at(ctx.slot()) == EventStatus::Success) {
    if (fail) 
      m_eventStatus.at(ctx.slot()) = EventStatus::AlgFail;
  } else if (m_eventStatus.at(ctx.slot()) == EventStatus::Invalid) {
    if (! fail) {
      m_eventStatus.at(ctx.slot()) = EventStatus::Success;
    } else {
      m_eventStatus.at(ctx.slot()) = EventStatus::AlgFail;
    }
  }
}


//-----------------------------------------------------------------------------

void
AlgExecStateSvc::reset(const EventContext& ctx) {

  verbose() << "reset(" << ctx.slot() << ")" << endmsg;

  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  for (auto &e : m_algStates.at(ctx.slot())) {
    e.second.reset();
  }

  m_eventStatus.at(ctx.slot()) = EventStatus::Invalid;

}

//-----------------------------------------------------------------------------

void
AlgExecStateSvc::reset() {

  verbose() << "reset()" << endmsg;

  std::call_once(m_initFlag, &AlgExecStateSvc::init, this);
  for (auto &e : m_algStates.at(0)) {
    e.second.reset();
  }

  m_eventStatus.at(0) = EventStatus::Invalid;

}

//-----------------------------------------------------------------------------

std::string
AlgExecStateSvc::trans(const EventStatus::Status& s) const {
  switch (s) {
  case EventStatus::Invalid :
    return "Invalid";
  case EventStatus::Success :
    return "Success";
  case EventStatus::AlgFail :
    return "AlgFail";
  case EventStatus::AlgStall :
    return "AlgStall";
  case EventStatus::Other :
    return "Other";
  default :
    return "Should not happen";
  }

}

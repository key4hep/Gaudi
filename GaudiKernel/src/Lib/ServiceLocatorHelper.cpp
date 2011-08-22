#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/ServiceLocatorHelper.h"

std::string
ServiceLocatorHelper::threadName() const {
  return getGaudiThreadIDfromName(requesterName());
}

std::string
ServiceLocatorHelper::threadedName(const std::string& name) const {
  return ( isInThread() ? name + threadName() : name );
}

bool
ServiceLocatorHelper::isInThread() const {
  return isGaudiThreaded(requesterName());
}

StatusCode
ServiceLocatorHelper::locateService(const std::string& name,
				    const InterfaceID& iid,
				    void** ppSvc,
				    bool quiet) const {
  SmartIF<IService> theSvc = service(name, quiet, false);
  if (!theSvc.isValid()) return StatusCode::FAILURE;
  StatusCode sc = theSvc->queryInterface(iid, ppSvc);
  if (!sc.isSuccess()) {
    *ppSvc = 0;
    if (!quiet) log() << MSG::ERROR
      << "ServiceLocatorHelper::locateService: wrong interface id "
      << iid << " for service " << name << endmsg;
  }
  return sc;
}

StatusCode
ServiceLocatorHelper::createService(const std::string& name,
				    const InterfaceID& iid,
				    void** ppSvc) const {
  SmartIF<IService> theSvc = service(name, false, true);
  if (!theSvc.isValid()) return StatusCode::FAILURE;
  StatusCode sc = theSvc->queryInterface(iid, ppSvc);
  if (!sc.isSuccess()) {
    *ppSvc = 0;
    log() << MSG::ERROR
      << "ServiceLocatorHelper::createService: wrong interface id "
      << iid << " for service " << name << endmsg;
  }
  return sc;
}

StatusCode
ServiceLocatorHelper::createService(const std::string& type,
				    const std::string& name,
				    const InterfaceID& iid,
				    void** ppSvc) const {
  return createService(type + "/" + name, iid, ppSvc);
}

SmartIF<IService> ServiceLocatorHelper::service(const std::string &name, const bool quiet, const bool createIf) const {
  SmartIF<IService> theSvc;
  if (isInThread()) {
    //first we look for  a thread-specific version of the service
    theSvc = serviceLocator()->service(name + threadName(), createIf);
  }
  // if not, try to find the common, single-threaded version of the service
  if (!theSvc.isValid()){
    theSvc = serviceLocator()->service(name, createIf);
  }

  if (theSvc.isValid()) {
    if (!quiet) {
      if (UNLIKELY(log().level() <= MSG::VERBOSE))
        log() << MSG::VERBOSE
              << "ServiceLocatorHelper::service: found service " << name <<endmsg;
    }
  } else {
    // if not return an error
    if (!quiet) {
      log() << MSG::ERROR
            << "ServiceLocatorHelper::service: can not locate service "
            << name;
      if (isInThread()) log() << MSG::ERROR << " or " << name + threadName();
      log() << MSG::ERROR << endmsg;
    }
  }
  return theSvc;
}

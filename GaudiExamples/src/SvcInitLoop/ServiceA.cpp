#include "ServiceA.h"

// Static Factory declaration
DECLARE_COMPONENT(ServiceA)

ServiceA::ServiceA(const std::string& nam, ISvcLocator* svcLoc) :
 Service(nam, svcLoc)
{
}

StatusCode ServiceA::initialize() {
  Service::initialize().ignore();
  IService *pService(0);
  const bool CREATENOW(true);
  return service("ServiceB", pService, CREATENOW);
}

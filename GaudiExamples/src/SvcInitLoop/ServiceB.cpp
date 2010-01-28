#include "GaudiKernel/SvcFactory.h"
#include "ServiceB.h"


// Static Factory declaration

DECLARE_SERVICE_FACTORY(ServiceB)

ServiceB::ServiceB(const std::string& nam, ISvcLocator* svcLoc) :
 Service(nam, svcLoc)
{
}

StatusCode
ServiceB::initialize() {
  Service::initialize().ignore();
  IService *pService(0);
  const bool CREATENOW(true);
  return service("ServiceA", pService, CREATENOW);
}

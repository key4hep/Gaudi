#include "ServiceA.h"

// Static Factory declaration
DECLARE_COMPONENT( ServiceA )

StatusCode ServiceA::initialize() {
  Service::initialize().ignore();
  IService*  pService = nullptr;
  const bool CREATENOW( true );
  return service( "ServiceB", pService, CREATENOW );
}

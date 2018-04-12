#include "ServiceB.h"

// Static Factory declaration
DECLARE_COMPONENT( ServiceB )

StatusCode ServiceB::initialize()
{
  Service::initialize().ignore();
  IService*  pService = nullptr;
  const bool CREATENOW( true );
  return service( "ServiceA", pService, CREATENOW );
}

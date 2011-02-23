#define  PERSISTENCYSVC_DETPERSISTENCYSVC_CPP

#include <iostream>

// Implementation specific definitions
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "DetPersistencySvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(DetPersistencySvc)

// Finalize the service.
StatusCode DetPersistencySvc::finalize()
{
  StatusCode status = PersistencySvc::finalize();
  return status;
}

// Initialize the service.
StatusCode DetPersistencySvc::initialize()
{
  StatusCode status = PersistencySvc::initialize();
  return status;
}

// Standard Constructor
DetPersistencySvc::DetPersistencySvc(const std::string& name, ISvcLocator* svc)
 :  PersistencySvc(name, svc)
{}

// Standard Destructor
DetPersistencySvc::~DetPersistencySvc()
{}

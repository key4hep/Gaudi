//====================================================================
//	EvtPersistencySvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Event data persistency service
//               This specialized service only deals with event related
//               data
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  PERSISTENCYSVC_EVTPERSISTENCYSVC_CPP

// Implementation specific definitions
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "EvtPersistencySvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(EvtPersistencySvc)

/// Finalize the service.
StatusCode EvtPersistencySvc::finalize()     {
  StatusCode status = PersistencySvc::finalize();
  return status;
}

/// Initialize the service.
StatusCode EvtPersistencySvc::initialize()     {
  StatusCode status = PersistencySvc::initialize();
  return status;
}

/// Standard Constructor
EvtPersistencySvc::EvtPersistencySvc(const std::string& name, ISvcLocator* svc)
 :  PersistencySvc(name, svc)
{
}

/// Standard Destructor
EvtPersistencySvc::~EvtPersistencySvc()   {
}

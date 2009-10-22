//====================================================================
//	EvtDataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient event data service.
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
#define  DATASVC_EVTDATASVC_CPP

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"

#include "EvtDataSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(EvtDataSvc)

/// Service initialisation
StatusCode EvtDataSvc::initialize()    {
  // Nothing to do: just call base class initialisation
  StatusCode      status  = DataSvc::initialize();
  ISvcLocator*    svc_loc = serviceLocator();

  // Attach data loader facility
  m_cnvSvc = svc_loc->service("EventPersistencySvc");
  status = setDataLoader( m_cnvSvc );
  return status;
}
/// Service reinitialisation
StatusCode EvtDataSvc::reinitialize()    {
  // Do nothing for this service
  return StatusCode::SUCCESS;
}
/// Service finalization
StatusCode EvtDataSvc::finalize()    {
  m_cnvSvc = 0; // release
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS ;
}

/// Standard Constructor
EvtDataSvc::EvtDataSvc(const std::string& name,ISvcLocator* svc):
  DataSvc(name,svc) {
}

/// Standard Destructor
EvtDataSvc::~EvtDataSvc() {
}

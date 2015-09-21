// Include files
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ModuleIncident.h"
#include "GaudiKernel/MsgStream.h"
#include "DLLClassManager.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/IAlgorithm.h"

#include <iostream>
#include <cassert>

// default creator
DLLClassManager::DLLClassManager( IInterface* iface ) {
  m_pOuter = iface;
  m_svclocator = m_pOuter;

  assert(m_svclocator.isValid());

  addRef(); // Initial count set to 1
}

// implementation of IClassManager::loadModule
StatusCode DLLClassManager::loadModule( const std::string& module,
					bool fireIncident ) {
  // Access the message service if not yet done already
  if (!m_msgsvc) m_msgsvc = m_svclocator;
  MsgStream log(m_msgsvc, "DllClassManager");

  std::string mod = module=="" ? System::moduleNameFull() : module;
  if( module == "NONE" ) return StatusCode::SUCCESS;

  void* libHandle = nullptr;
  StatusCode status = StatusCode::FAILURE;
  try
  {
    status = System::loadDynamicLib( module, &libHandle);
  }
  catch ( const std::exception & excpt )
  {
    if ( m_msgsvc )
    {
      log << MSG::ERROR << "Exception whilst loading " << module << " : " << excpt.what() << endmsg;
    }
    status = StatusCode::FAILURE;
  }

  if( status.isFailure() ) {
    // DLL library not loaded
    log << MSG::ERROR << "Could not load module " << module << endmsg;
    log << MSG::ERROR << "System Error: " << System::getLastErrorString() << endmsg;
    return StatusCode::FAILURE;
  }
  //FIXME this is a hack to avoid a very early call to moduleLoad from
  //FIXME AppMgr::i_startup
  if (fireIncident && !module.empty()) {
    //now fire ModuleLoadedIncident
    const bool CREATEIF(true);
    auto pIncidentSvc = m_svclocator->service<IIncidentSvc>("IncidentSvc", CREATEIF);
    if( !pIncidentSvc )  {
      log << MSG::FATAL << "Can not locate IncidentSvc" << endmsg;
      throw GaudiException("Error retrieving IncidentSvc",
			   "DLLClassManager::DLLClassManager", StatusCode::FAILURE);
    }
    pIncidentSvc->fireIncident(ModuleLoadedIncident("DLLClassManager",
						     module) );
  }

  return StatusCode::SUCCESS;
}

// implementation of IInterface::queryInterface
StatusCode DLLClassManager::queryInterface(const InterfaceID& iid, void** pinterface) {
  // try local interfaces
  StatusCode sc = base_class::queryInterface(iid, pinterface);
  if (sc.isSuccess()) return sc;
  // fall back on the owner
  return m_pOuter->queryInterface(iid, pinterface);
}

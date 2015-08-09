// $Id: DLLClassManager.h,v 1.7 2006/12/11 10:37:02 leggett Exp $	//

#ifndef GAUDI_DLLCLASSMANAGER_H
#define GAUDI_DLLCLASSMANAGER_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IClassManager.h"
#include <string>
#include <list>
#include <map>

// Forward declarations
class ISvcLocator;
class ICnvManager;
class IAlgManager;
class ISvcManager;
class IObjManager;

//
// ClassName:   DLLClassManager
//
// Description: The ClassManager class is in charge of managing (i.e. loading and
//              declaring dynamic libraries containing concrete class factories).
//
// Author:      Pere Mato
//
class DLLClassManager : public implements1<IClassManager> {
public:
  // default creator
  DLLClassManager( IInterface* iface );
  // virtual destructor
  ~DLLClassManager() override = default;

  // implementation of IClassManager::loadModule
  StatusCode loadModule( const std::string& module, bool fireIncident=true ) override;

  /// implementation of IInterface::queryInterface
  StatusCode queryInterface(const InterfaceID& iid, void** pinterface) override;

private:
  SmartIF<ISvcLocator>  m_svclocator;  // Service locator reference
  SmartIF<IMessageSvc>  m_msgsvc;      // Message Service reference
  SmartIF<IInterface>   m_pOuter;      // Interface hub reference (ApplicationMgr)
};
#endif  // GAUDI_DLLCLASSMANAGER_H



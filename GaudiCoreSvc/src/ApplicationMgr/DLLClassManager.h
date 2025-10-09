/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include <GaudiKernel/IClassManager.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/SmartIF.h>
#include <list>
#include <map>
#include <string>

// Forward declarations
class ISvcLocator;
class ICnvManager;
class IAlgManager;
class ISvcManager;
class IObjManager;
class IMessageSvc;

//
// ClassName:   DLLClassManager
//
// Description: The ClassManager class is in charge of managing (i.e. loading and
//              declaring dynamic libraries containing concrete class factories).
//
// Author:      Pere Mato
//
class DLLClassManager : public implements<IClassManager> {
public:
  // default creator
  DLLClassManager( IInterface* iface );

  // implementation of IClassManager::loadModule
  StatusCode loadModule( const std::string& module, bool fireIncident = true ) override;

  // Allow delegation of interfaces implementations
  void const* i_cast( const InterfaceID& iid ) const override;

private:
  SmartIF<ISvcLocator> m_svclocator; // Service locator reference
  SmartIF<IMessageSvc> m_msgsvc;     // Message Service reference
  SmartIF<IInterface>  m_pOuter;     // Interface hub reference (ApplicationMgr)
};

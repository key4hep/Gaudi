/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
#define DATASVC_EVTDATASVC_CPP

#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/ISvcLocator.h>

#include "EvtDataSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( EvtDataSvc )

/// Service initialisation
StatusCode EvtDataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode status = DataSvc::initialize();

  // Attach data loader facility
  m_cnvSvc = serviceLocator()->service( "EventPersistencySvc" );
  status   = setDataLoader( m_cnvSvc );
  return status;
}
/// Service reinitialisation
StatusCode EvtDataSvc::reinitialize() {
  // Do nothing for this service
  return StatusCode::SUCCESS;
}
/// Service finalization
StatusCode EvtDataSvc::finalize() {
  m_cnvSvc = nullptr; // release
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS;
}

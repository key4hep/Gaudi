/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
#define PERSISTENCYSVC_EVTPERSISTENCYSVC_CPP

// Implementation specific definitions
#include "EvtPersistencySvc.h"
#include "GaudiKernel/ISvcLocator.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( EvtPersistencySvc )

/// Standard Constructor
EvtPersistencySvc::EvtPersistencySvc( const std::string& name, ISvcLocator* svc ) : PersistencySvc( name, svc ) {}

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
#define PERSISTENCYSVC_DETPERSISTENCYSVC_CPP

#include <iostream>

// Implementation specific definitions
#include "DetPersistencySvc.h"
#include <GaudiKernel/ISvcLocator.h>

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( DetPersistencySvc )

// Standard Constructor
DetPersistencySvc::DetPersistencySvc( const std::string& name, ISvcLocator* svc ) : PersistencySvc( name, svc ) {}

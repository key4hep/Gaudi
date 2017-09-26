#define PERSISTENCYSVC_DETPERSISTENCYSVC_CPP

#include <iostream>

// Implementation specific definitions
#include "DetPersistencySvc.h"
#include "GaudiKernel/ISvcLocator.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( DetPersistencySvc )

// Standard Constructor
DetPersistencySvc::DetPersistencySvc( const std::string& name, ISvcLocator* svc ) : PersistencySvc( name, svc ) {}

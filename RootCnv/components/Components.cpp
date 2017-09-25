#include "RootCnv/RootCnvSvc.h"
#include "RootCnv/RootEvtSelector.h"
#include "RootCnv/RootPerfMonSvc.h"

#ifndef DECLARE_COMPONENT
#define DECLARE_COMPONENT( x ) DECLARE_SERVICE_FACTORY( x )
#endif

using namespace Gaudi;
DECLARE_COMPONENT( RootCnvSvc )
DECLARE_COMPONENT( RootEvtSelector )
DECLARE_COMPONENT( RootPerfMonSvc )

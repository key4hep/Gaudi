// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/AudFactory.h,v 1.8 2007/05/25 16:39:16 marcocle Exp $
#ifndef GAUDIKERNEL_AUDFACTORY_H
#define GAUDIKERNEL_AUDFACTORY_H

#include "GaudiKernel/Kernel.h"
#include "Reflex/PluginService.h"

#define DECLARE_AUDITOR(x)  /*dummy*/
#define DECLARE_NAMESPACE_AUDITOR(n,x)  /*dummy*/
#define DECLARE_AUDITOR_FACTORY(x) PLUGINSVC_FACTORY(x,IAuditor*(std::string, ISvcLocator*))
#define DECLARE_NAMESPACE_AUDITOR_FACTORY(n,x) using n::x; PLUGINSVC_FACTORY(x,IAuditor*(std::string, ISvcLocator*))

#endif // GAUDIKERNEL_AUDFACTORY_H

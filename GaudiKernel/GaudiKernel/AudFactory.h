#ifndef GAUDIKERNEL_AUDFACTORY_H
#define GAUDIKERNEL_AUDFACTORY_H

#include <Gaudi/PluginService.h>

class IAuditor;
class ISvcLocator;

typedef Gaudi::PluginService::Factory2<IAuditor*,
                                       const std::string&,
                                       ISvcLocator*> AudFactory;

// Macros to declare component factories
#define DECLARE_AUDITOR_FACTORY(x) DECLARE_FACTORY(x, AudFactory)
#define DECLARE_NAMESPACE_AUDITOR_FACTORY(n,x) using n::x; DECLARE_FACTORY(x, AudFactory)

#endif // GAUDIKERNEL_AUDFACTORY_H

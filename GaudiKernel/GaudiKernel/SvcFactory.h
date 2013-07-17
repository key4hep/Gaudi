#ifndef KERNEL_SVCFACTORY_H
#define KERNEL_SVCFACTORY_H

#include <Gaudi/PluginService.h>

class IService;
class ISvcLocator;

typedef Gaudi::PluginService::Factory2<IService*,
                                       const std::string&,
                                       ISvcLocator*> SvcFactory;

// Macros to declare component factories
#define DECLARE_SERVICE_FACTORY(x)              DECLARE_FACTORY(x, SvcFactory)
#define DECLARE_NAMED_SERVICE_FACTORY(x, n)     DECLARE_FACTORY_WITH_ID(x, n, SvcFactory)
#define DECLARE_NAMESPACE_SERVICE_FACTORY(n, x) using n::x; DECLARE_FACTORY(x, SvcFactory)


#endif // GAUDIKERNEL_SVCFACTORY_H

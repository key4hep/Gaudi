#ifndef GAUDIKERNEL_ALGFACTORY_H
#define GAUDIKERNEL_ALGFACTORY_H

#include <Gaudi/PluginService.h>

class IAlgorithm;
class ISvcLocator;

typedef Gaudi::PluginService::Factory2<IAlgorithm*,
                                       const std::string&,
                                       ISvcLocator*> AlgFactory;

// Macros to declare component factories
#define DECLARE_ALGORITHM_FACTORY(x)              DECLARE_FACTORY(x, AlgFactory)
#define DECLARE_NAMED_ALGORITHM_FACTORY(x, n)     DECLARE_FACTORY_WITH_ID(x, n, AlgFactory)
#define DECLARE_NAMESPACE_ALGORITHM_FACTORY(n, x) using n::x; DECLARE_FACTORY(x, AlgFactory)

#endif // GAUDIKERNEL_ALGFACTORY_H

#ifndef GAUDI_KERNEL_TOOLFACTORY_H
#define GAUDI_KERNEL_TOOLFACTORY_H

#include <Gaudi/PluginService.h>

class IAlgTool;
class IInterface;

typedef Gaudi::PluginService::Factory3<IAlgTool*,
                                       const std::string&,
                                       const std::string&,
                                       const IInterface*> ToolFactory;

// Macros to declare component factories
#define DECLARE_TOOL_FACTORY(x)              DECLARE_FACTORY(x, ToolFactory)
#define DECLARE_NAMESPACE_TOOL_FACTORY(n,x)  using n::x; DECLARE_FACTORY(x, ToolFactory)

#endif // GAUDI_KERNEL_TOOLFACTORY_H

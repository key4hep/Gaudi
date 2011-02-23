// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ToolFactory.h,v 1.9 2007/01/23 09:13:07 mato Exp $
#ifndef GAUDI_KERNEL_TOOLFACTORY_H
#define GAUDI_KERNEL_TOOLFACTORY_H

#include "Reflex/PluginService.h"
#include "RVersion.h"

// The following is needed to be backward compatible with the old factories of Gaudi. Otherwise the components
// having the constructor/destructor protected will not be working

class IAlgTool;
class IInterface;

template <typename T> class ToolFactory {
  public:
  static IAlgTool* create(const std::string& type, const std::string& name,  IInterface *parent) {
    return new T(type, name, parent );
  }
};

namespace {
  template < typename P > class Factory<P, IAlgTool*(std::string, std::string, const IInterface*)> {
  public:
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
    static void* Func( void*, const std::vector<void*>& arg, void*) {
      return ToolFactory<P>::create(*(std::string*)(arg[0]), *(std::string*)(arg[1]), (IInterface*)(arg[2]));
    }
#else
    static void Func( void *retaddr, void*, const std::vector<void*>& arg, void*) {
      *(IAlgTool**) retaddr = ToolFactory<P>::create(*(std::string*)(arg[0]), *(std::string*)(arg[1]), (IInterface*)(arg[2]));
    }
#endif
  };
}

// Macros to declare components
#define DECLARE_ALGTOOL(x)             /*dummy*/
#define DECLARE_NAMESPACE_ALGTOOL(n,x) /*dummy*/
#define DECLARE_TOOL(x)                /*dummy*/
#define DECLARE_NAMESPACE_TOOL(n,x)    /*dummy*/

// Macros to declare component factories
#define DECLARE_TOOL_FACTORY(x)              PLUGINSVC_FACTORY(x,IAlgTool*(std::string, std::string, const IInterface*))
#define DECLARE_NAMESPACE_TOOL_FACTORY(n,x)  using n::x; PLUGINSVC_FACTORY(x,IAlgTool*(std::string, std::string, const IInterface*))

#endif // GAUDI_KERNEL_TOOLFACTORY_H

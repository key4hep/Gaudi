// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/SvcFactory.h,v 1.11 2008/10/09 21:00:37 marcocle Exp $
#ifndef KERNEL_SVCFACTORY_H
#define KERNEL_SVCFACTORY_H

#include "Reflex/PluginService.h"
#include "RVersion.h"

// The following is needed to be backward compatible with the old factories of Gaudi. Otherwise the components
// having the constructor/destructor protected will not be working

class IService;
class ISvcLocator;

template <typename T> class SvcFactory {
public:
  static IService* create(const std::string& name, ISvcLocator *svcloc) {
    return new T(name, svcloc );
  }
};

namespace {
  template < typename P > class Factory<P, IService*(std::string, ISvcLocator*)> {
  public:
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
    static void* Func( void*, const std::vector<void*>& arg, void*) {
      return SvcFactory<P>::create(*(std::string*)(arg[0]), (ISvcLocator*)(arg[1]));
    }
#else
    static void Func( void *retaddr, void*, const std::vector<void*>& arg, void*) {
      *(IService**) retaddr = SvcFactory<P>::create(*(std::string*)(arg[0]), (ISvcLocator*)(arg[1]));
    }
#endif
  };
}

// Macros to declare component factories

#define DECLARE_SERVICE_FACTORY(x)              PLUGINSVC_FACTORY(x,IService*(std::string, ISvcLocator*))
#define DECLARE_NAMED_SERVICE_FACTORY(x,n)      PLUGINSVC_FACTORY_WITH_ID(x,std::string(#n), IService*(std::string, ISvcLocator*))
#define DECLARE_NAMESPACE_SERVICE_FACTORY(n,x)  using n::x; PLUGINSVC_FACTORY(x,IService*(std::string, ISvcLocator*))
#define DECLARE_SERVICE(x)  /*dummy*/
#define DECLARE_NAMESPACE_SERVICE(n,x) /*dummy*/


#endif // GAUDIKERNEL_SVCFACTORY_H

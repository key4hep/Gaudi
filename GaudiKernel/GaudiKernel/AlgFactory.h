// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/AlgFactory.h,v 1.10 2008/10/09 21:00:37 marcocle Exp $
#ifndef GAUDIKERNEL_ALGFACTORY_H
#define GAUDIKERNEL_ALGFACTORY_H

#include "GaudiKernel/Kernel.h"
#include "Reflex/PluginService.h"
#include "RVersion.h"

// The following is needed to be backward compatible with the old factories of Gaudi. Otherwise the components
// having the constructor/destructor protected will not be working

class IAlgorithm;
class ISvcLocator;

template <typename T> class AlgFactory {
public:
  static IAlgorithm* create(const std::string& name, ISvcLocator *svcloc) {
    return new T(name, svcloc );
  }
};

namespace {
  template < typename P > class Factory<P, IAlgorithm*(std::string, ISvcLocator*)> {
  public:
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
    static void* Func( void*, const std::vector<void*>& arg, void*) {
      return AlgFactory<P>::create(*(std::string*)(arg[0]), (ISvcLocator*)(arg[1]));
    }
#else
    static void Func( void *retaddr, void*, const std::vector<void*>& arg, void*) {
      *(IAlgorithm**) retaddr = AlgFactory<P>::create(*(std::string*)(arg[0]), (ISvcLocator*)(arg[1]));
    }
#endif
  };
}

// Macros to declare component factories
#define DECLARE_ALGORITHM_FACTORY(x)          PLUGINSVC_FACTORY(x,IAlgorithm*(std::string, ISvcLocator*))
#define DECLARE_NAMED_ALGORITHM_FACTORY(x,n)  PLUGINSVC_FACTORY_WITH_ID(x,std::string(#n), IAlgorithm*(std::string, ISvcLocator*))

#define DECLARE_NAMESPACE_ALGORITHM_FACTORY(n,x)  using n::x; PLUGINSVC_FACTORY(x,IAlgorithm*(std::string, ISvcLocator*))
#define DECLARE_ALGORITHM(x)  /*dummy*/
#define DECLARE_NAMESPACE_ALGORITHM(n,x) /*dummy*/

#endif // GAUDIKERNEL_ALGFACTORY_H

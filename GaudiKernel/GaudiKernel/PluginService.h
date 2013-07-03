#ifndef _GAUDIKERNEL_PLUGINSERVICE_H_
#define _GAUDIKERNEL_PLUGINSERVICE_H_

#include <Reflex/PluginService.h>

namespace Gaudi {
  // Wrap the Reflex plugin service in the Gaudi namespace to simplify the
  // migration to ROOT 6.
  using ROOT::Reflex::PluginService;
}



#endif

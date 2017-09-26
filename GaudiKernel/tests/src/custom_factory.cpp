/*
 * Simple example of a custom factory to test:
 *
 * GAUDI-975: relax the checks in genconf to allow custom component factories
 * https://its.cern.ch/jira/browse/GAUDI-975
 *
 */

#include "Gaudi/PluginService.h"

struct IMyInterface {
  typedef Gaudi::PluginService::Factory<IMyInterface*> Factory;
};

struct MyImplementation : virtual public IMyInterface {
};

DECLARE_COMPONENT( MyImplementation )

#ifndef GAUDIKERNEL_DATAOBJECTFACTORY_H
#define GAUDIKERNEL_DATAOBJECTFACTORY_H

#include <Gaudi/PluginService.h>

class IInterface;
class DataObject;
class ContainedObject;

typedef Gaudi::PluginService::Factory1<IInterface*,
                                       IInterface*> ObjFactory;

typedef Gaudi::PluginService::Factory0<DataObject*> DataObjFactory;
typedef Gaudi::PluginService::Factory0<ContainedObject*> ContainedObjFactory;

// Macros to declare component factories
#define DECLARE_OBJECT_FACTORY(x)              DECLARE_FACTORY(x, ObjFactory)
#define DECLARE_NAMESPACE_OBJECT_FACTORY(n,x)  using n::x; DECLARE_FACTORY(x, ObjFactory)

#define DECLARE_CONTAINEDOBJECT_FACTORY( x )   DECLARE_FACTORY_WITH_ID(x, x::classID(), ContainedObjFactory)
#define DECLARE_DATAOBJECT_FACTORY( x )        DECLARE_FACTORY_WITH_ID(x, x::classID(), DataObjFactory)

#endif    // GAUDIKERNEL_OBJECTFactory_H

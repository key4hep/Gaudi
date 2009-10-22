// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ObjectFactory.h,v 1.10 2006/12/06 17:18:03 mato Exp $
#ifndef GAUDIKERNEL_DATAOBJECTFACTORY_H
#define GAUDIKERNEL_DATAOBJECTFACTORY_H

#include "Reflex/PluginService.h"
// Macros to declare component factories
#define DECLARE_OBJECT_FACTORY(x)              PLUGINSVC_FACTORY(x,IInterface*(IInterface*))
#define DECLARE_NAMESPACE_OBJECT_FACTORY(n,x)  using n::x; PLUGINSVC_FACTORY(x,IInterface*(IInterface*))

#define DECLARE_CONTAINEDOBJECT_FACTORY( x )   PLUGINSVC_FACTORY_WITH_ID(x,x::classID(),ContainedObject*(void))
#define DECLARE_DATAOBJECT_FACTORY( x )        PLUGINSVC_FACTORY_WITH_ID(x,x::classID(),DataObject*(void))

#define DECLARE_OBJECT(x)       /* dummy */
#define DECLARE_DATAOBJECT(x)   /* dummy */
#define DECLARE_NAMESPACE_OBJECT(n,x)      /* dummy */
#define DECLARE_NAMESPACE_DATAOBJECT(n,x)  /* dummy */

#endif    // GAUDIKERNEL_OBJECTFactory_H

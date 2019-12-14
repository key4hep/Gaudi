/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_DATAOBJECTFACTORY_H
#define GAUDIKERNEL_DATAOBJECTFACTORY_H

#include <Gaudi/PluginService.h>

class IInterface;
class DataObject;
class ContainedObject;

using ObjFactory          = Gaudi::PluginService::Factory<IInterface*( IInterface* )>;
using DataObjFactory      = Gaudi::PluginService::Factory<DataObject*()>;
using ContainedObjFactory = Gaudi::PluginService::Factory<ContainedObject*()>;

// Macros to declare component factories
#define DECLARE_OBJECT_FACTORY( x ) DECLARE_FACTORY( x, ObjFactory )
#define DECLARE_NAMESPACE_OBJECT_FACTORY( n, x )                                                                       \
  using n::x;                                                                                                          \
  DECLARE_FACTORY( x, ObjFactory )

#define DECLARE_CONTAINEDOBJECT_FACTORY( x ) DECLARE_FACTORY_WITH_ID( x, x::classID(), ContainedObjFactory )
#define DECLARE_DATAOBJECT_FACTORY( x ) DECLARE_FACTORY_WITH_ID( x, x::classID(), DataObjFactory )

#endif // GAUDIKERNEL_OBJECTFactory_H

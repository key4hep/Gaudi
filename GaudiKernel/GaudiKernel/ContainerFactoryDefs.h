//====================================================================
// ContainerFactoriesDefs.cpp
//--------------------------------------------------------------------
//
//	Package   : LHCbEvent
//
//	Author    : Markus Frank
//  History   :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who     
// +---------+----------------------------------------------+---------
// | 21/07/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//====================================================================
#ifndef CONTAINERFACTORIESDEFS_H
#define CONTAINERFACTORIESDEFS_H  1

#define _ImplementContainerDictionaryFactory(x)
#define _ImplementDataObjectDictionaryFactory(x)
#define _ImplementDictionaryFactory(x)
#define DLL_DECL_CONTAINERDICT(x)
#define DLL_DECL_OBJECTDICT(x)


#include "GaudiKernel/ObjectList.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/ObjectFactory.h"


#define _ImplementContainedFactories(x)      \
  _ImplementContainerDictionaryFactory(x)    \
  _ImplementContainedObjectFactory(x)        \
  _ImplementDataObjectFactory( x##Vector )   \
  _ImplementDataObjectFactory( x##List ) 

#define DLL_DECL_CONTAINEDOBJECTFACTORIES(x) \
  DLL_DECL_CONTAINERDICT(x)                  \
  DLL_DECL_OBJECTFACTORY(x)                  \
  DLL_DECL_OBJECTFACTORY( x##Vector )        \
  DLL_DECL_OBJECTFACTORY( x##List )

#endif // CONTAINERFACTORIESDEFS_H


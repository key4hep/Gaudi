/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
#pragma once

#define _ImplementContainerDictionaryFactory( x )
#define _ImplementDataObjectDictionaryFactory( x )
#define _ImplementDictionaryFactory( x )
#define DLL_DECL_CONTAINERDICT( x )
#define DLL_DECL_OBJECTDICT( x )

#include <GaudiKernel/ObjectFactory.h>
#include <GaudiKernel/ObjectList.h>
#include <GaudiKernel/ObjectVector.h>

#define _ImplementContainedFactories( x )                                                                              \
  _ImplementContainerDictionaryFactory( x ) _ImplementContainedObjectFactory( x )                                      \
      _ImplementDataObjectFactory( x##Vector ) _ImplementDataObjectFactory( x##List )

#define DLL_DECL_CONTAINEDOBJECTFACTORIES( x )                                                                         \
  DLL_DECL_CONTAINERDICT( x )                                                                                          \
  DLL_DECL_OBJECTFACTORY( x )                                                                                          \
  DLL_DECL_OBJECTFACTORY( x##Vector )                                                                                  \
  DLL_DECL_OBJECTFACTORY( x##List )

/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_LOADFACTORYENTRIES_H
#define GAUDIKERNEL_LOADFACTORYENTRIES_H 1
//====================================================================
//  LoadFactoryEntries.h
//--------------------------------------------------------------------
//
//  Package    : GaudiKernel
//
//  Description: Implementation of getFactoryEntries() routine.
//               This routine is needed for forcing the linker
//               to load all the components of the library.
//
//====================================================================

// DllMain entry point
#include <GaudiKernel/DllMain.icpp>

#if !defined( __APPLE__ )
void GaudiDll::initialize( void* ) {}
void GaudiDll::finalize( void* ) {}
#endif

#if defined( GAUDI_V20_COMPAT ) && !defined( G21_HIDE_SYMBOLS )
#  ifdef _WIN32
#    define FACTORYTABLE_API __declspec( dllexport )
#  else
#    define FACTORYTABLE_API
#  endif

#  define LOAD_FACTORY_ENTRIES( x )                                                                                    \
    extern "C" FACTORYTABLE_API void* x##_getFactoryEntries() { return nullptr; }

#else

#  define LOAD_FACTORY_ENTRIES( x )                                                                                    \
    extern "C" GAUDI_EXPORT void* x##_getFactoryEntries() { return nullptr; }

#endif // GAUDI_V20_COMPAT

#endif // GAUDIKERNEL_LOADFACTORYENTRIES_H

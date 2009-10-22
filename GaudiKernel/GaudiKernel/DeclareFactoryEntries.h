// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/DeclareFactoryEntries.h,v 1.10 2006/01/10 20:03:57 hmd Exp $
#ifndef GAUDIKERNEL_DECLAREFACTORYENTRIES_H
#define GAUDIKERNEL_DECLAREFACTORYENTRIES_H 1
//====================================================================
//  DeclareFactoryEntries.h
//--------------------------------------------------------------------
//
//  Package    : GaudiKernel
//
//  Description: Implementation of <Package>_load routine.
//               This routine is needed for forcing the linker
//               to load all the components of the library. 
//
//====================================================================

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/AudFactory.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/ObjectFactory.h"

#define DECLARE_FACTORY_ENTRIES(x) \
void x##_load(); \
\
extern "C" void x##_loadRef() { \
  x##_load();  \
} \
\
void x##_load()


#endif // GAUDIKERNEL_DECLAREFACTORYENTRIES_H

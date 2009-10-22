/** @file VirtualDestructors.cpp
 *
 * This file is used to provide virtual destructors for abstract interfaces.
 * They need to be defined in the linker library to make the dynamic_cast work.
 *
 * @author Marco Clemencic
 *
 */

#include "GaudiKernel/IFileAccess.h"
IFileAccess::~IFileAccess() {}

#include "GaudiKernel/HistogramBase.h"
namespace Gaudi {
  HistogramBase::~HistogramBase(){}
}

#include "GaudiKernel/IComponentManager.h"
IComponentManager::~IComponentManager(){}

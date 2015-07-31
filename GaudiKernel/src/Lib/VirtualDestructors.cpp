/** @file VirtualDestructors.cpp
 *
 * This file is used to provide virtual destructors for abstract interfaces.
 * They need to be defined in the linker library to make the dynamic_cast work.
 *
 * @author Marco Clemencic
 *
 */

#include "GaudiKernel/IFileAccess.h"
IFileAccess::~IFileAccess() = default;

#include "GaudiKernel/HistogramBase.h"
namespace Gaudi {
  HistogramBase::~HistogramBase() = default;
}

#include "GaudiKernel/HistoryObj.h"
HistoryObj::~HistoryObj() = default;

#include "GaudiKernel/IVersHistoryObj.h"
IVersHistoryObj::~IVersHistoryObj() = default;

#include "GaudiKernel/IIoComponent.h"
IIoComponent::~IIoComponent() = default;

#include "GaudiKernel/IIoComponentMgr.h"
IIoComponentMgr::~IIoComponentMgr() = default;

#include "GaudiKernel/IFileMgr.h"
IFileMgr::~IFileMgr() = default;


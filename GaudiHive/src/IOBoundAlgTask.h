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
#ifndef GAUDIHIVE_IOBOUNDALGTASK_H
#define GAUDIHIVE_IOBOUNDALGTASK_H

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgTask.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"

#include <functional>

/**@class IOBoundAlgTask IOBoundAlgTask.h GaudiHive/src/IOBoundAlgTask.h
 *
 *  Wrapper around I/O-bound Gaudi-algorithms. It may also cover the accelerator-targeted algorithms.
 *  It must be used to prepare algorithms before sending them to I/O-bound scheduler.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */

class IOBoundAlgTask : public IAlgTask {
public:
  IOBoundAlgTask( IAlgorithm* algorithm, const EventContext& ctx, ISvcLocator* svcLocator, IAlgExecStateSvc* aem,
                  std::function<StatusCode()> promote2ExecutedClosure )
      : m_algorithm( algorithm )
      , m_evtCtx( ctx )
      , m_aess( aem )
      , m_serviceLocator( svcLocator )
      , m_promote2ExecutedClosure( std::move( promote2ExecutedClosure ) ) {}

  StatusCode operator()() override final;

private:
  SmartIF<IAlgorithm>         m_algorithm;
  const EventContext&         m_evtCtx;
  IAlgExecStateSvc*           m_aess;
  SmartIF<ISvcLocator>        m_serviceLocator;
  std::function<StatusCode()> m_promote2ExecutedClosure;
};

#endif

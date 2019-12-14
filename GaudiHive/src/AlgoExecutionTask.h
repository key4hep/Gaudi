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
#ifndef GAUDIHIVE_ALGOEXECUTIONTASK_H
#define GAUDIHIVE_ALGOEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"

#include "AvalancheSchedulerSvc.h"

// External libs
#include "tbb/task.h"

class AlgoExecutionTask : public tbb::task {
public:
  AlgoExecutionTask( AvalancheSchedulerSvc* scheduler, ISvcLocator* svcLocator, IAlgExecStateSvc* aem )
      : m_scheduler( scheduler ), m_aess( aem ), m_serviceLocator( svcLocator ){};
  tbb::task* execute() override;

private:
  AvalancheSchedulerSvc* m_scheduler;
  IAlgExecStateSvc*      m_aess;
  SmartIF<ISvcLocator>   m_serviceLocator;
};

#endif

#ifndef GAUDIHIVE_ALGOEXECUTIONTASK_H
#define GAUDIHIVE_ALGOEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"

// External libs
#include "tbb/task.h"

class AlgoExecutionTask : public tbb::task
{
public:
  AlgoExecutionTask( IAlgorithm* algorithm, EventContext* ctx, ISvcLocator* svcLocator, IAlgExecStateSvc* aem )
      : m_algorithm( algorithm ), m_evtCtx( ctx ), m_aess( aem ), m_serviceLocator( svcLocator ){};
  tbb::task* execute() override;

private:
  SmartIF<IAlgorithm>  m_algorithm;
  EventContext*        m_evtCtx;
  IAlgExecStateSvc*    m_aess;
  SmartIF<ISvcLocator> m_serviceLocator;
};

#endif

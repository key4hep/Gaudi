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
  AlgoExecutionTask( IAlgorithm* algorithm, const EventContext& ctx, ISvcLocator* svcLocator, IAlgExecStateSvc* aem,
                     std::function<StatusCode()> promote2ExecutedClosure )
      : m_algorithm( algorithm )
      , m_evtCtx( ctx )
      , m_aess( aem )
      , m_serviceLocator( svcLocator )
      , m_promote2ExecutedClosure( std::move( promote2ExecutedClosure ) ){};
  tbb::task* execute() override;

private:
  SmartIF<IAlgorithm>         m_algorithm;
  const EventContext&         m_evtCtx;
  IAlgExecStateSvc*           m_aess;
  SmartIF<ISvcLocator>        m_serviceLocator;
  std::function<StatusCode()> m_promote2ExecutedClosure;
};

#endif

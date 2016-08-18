#ifndef GAUDIHIVE_ALGOEXECUTIONTASK_H
#define GAUDIHIVE_ALGOEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

#include "ForwardSchedulerSvc.h"

// External libs
#include "tbb/task.h"

class AlgoExecutionTask: public tbb::task {
public:
  AlgoExecutionTask(IAlgorithm* algorithm, 
                    unsigned int algoIndex, 
                    EventContext* ctx,
                    ISvcLocator* svcLocator,
                    ForwardSchedulerSvc* schedSvc,
                    IAlgExecStateSvc* aem):
    m_algorithm(algorithm),
    m_evtCtx(ctx),
    m_algoIndex(algoIndex),
    m_schedSvc(schedSvc),
    m_aess(aem),
    m_serviceLocator(svcLocator){};
  virtual tbb::task* execute();
private:  
  SmartIF<IAlgorithm> m_algorithm;
  EventContext* m_evtCtx;
  const unsigned int m_algoIndex;
  // For the callbacks on task finishing
  SmartIF<ForwardSchedulerSvc> m_schedSvc;
  IAlgExecStateSvc *m_aess;
  SmartIF<ISvcLocator> m_serviceLocator;
};

#endif

#ifndef GAUDIKERNEL_ALGOEXECUTIONTASK_H
#define GAUDIKERNEL_ALGOEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/IAlgorithm.h"
#include "SchedulerSvc.h"

// External libs
#include "tbb/task.h"

// TODO: the index should be set on the algo by the algoManager
class AlgoExecutionTask: public tbb::task {
public:
  AlgoExecutionTask(IAlgorithm* algorithm, 
                    unsigned int algoIndex, 
                    SchedulerSvc* schedSvc):
    m_algorithm(algorithm),
    m_algoIndex(algoIndex),
    m_schedSvc(schedSvc){};
  virtual tbb::task* execute();
private:  
  IAlgorithm* m_algorithm;
  const unsigned int m_algoIndex;
  // For the callbacks on task finishing
  SchedulerSvc* m_schedSvc;
};

#endif

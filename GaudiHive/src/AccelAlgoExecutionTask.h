#ifndef GAUDIHIVE_ACCELALGOEXECUTIONTASK_H
#define GAUDIHIVE_ACCELALGOEXECUTIONTASK_H

// Framework include files
#include "GaudiKernel/IAlgTask.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"

#include "ForwardSchedulerSvc.h"

/**@class AccelAlgoExecutionTask AccelAlgoExecutionTask.h GaudiHive/src/AccelAlgoExecutionTask.h
 *
 *  Wrapper around Gaudi algorithm, specializing on accelerator-targeted Gaudi algorithm.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */

class AccelAlgoExecutionTask : public IAlgTask {
public:
    AccelAlgoExecutionTask(IAlgorithm* algorithm,
                           unsigned int algoIndex,
                           ISvcLocator* svcLocator,
                           ForwardSchedulerSvc* schedSvc):
    m_algorithm(algorithm),
    m_algoIndex(algoIndex),
    m_schedSvc(schedSvc),
    m_serviceLocator(svcLocator){};

    ~AccelAlgoExecutionTask() {};

    virtual StatusCode execute();

private:
  SmartIF<IAlgorithm> m_algorithm;
  const unsigned int m_algoIndex;
  // For the callbacks on task finishing
  SmartIF<ForwardSchedulerSvc> m_schedSvc;
  SmartIF<ISvcLocator> m_serviceLocator;
};

#endif

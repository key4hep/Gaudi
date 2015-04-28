#ifndef GAUDIHIVE_IOBOUNDALGTASK_H
#define GAUDIHIVE_IOBOUNDALGTASK_H

// Framework include files
#include "GaudiKernel/IAlgTask.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"

#include "ForwardSchedulerSvc.h"

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
    IOBoundAlgTask(IAlgorithm* algorithm,
                           unsigned int algoIndex,
                           ISvcLocator* svcLocator,
                           ForwardSchedulerSvc* schedSvc):
    m_algorithm(algorithm),
    m_algoIndex(algoIndex),
    m_schedSvc(schedSvc),
    m_serviceLocator(svcLocator){};

    ~IOBoundAlgTask() {};

    virtual StatusCode execute();

private:
  SmartIF<IAlgorithm> m_algorithm;
  const unsigned int m_algoIndex;
  // For the callbacks on task finishing
  SmartIF<ForwardSchedulerSvc> m_schedSvc; // TODO consider using it through its interface (IScheduler?)
  SmartIF<ISvcLocator> m_serviceLocator;
};

#endif

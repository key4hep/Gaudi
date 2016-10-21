#ifndef GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H
#define GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

#include "AlgResourcePool.h"
#include "ExecutionFlowManager.h"
#include "DataFlowManager.h"

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>

// External libs
#include "tbb/concurrent_queue.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"

class SequentialTask; //forward declaration


//---------------------------------------------------------------------------

/**@class ParallelSequentialSchedulerSvc ParallelSequentialSchedulerSvc.h
 *
 *  This SchedulerSvc implements the IScheduler interface.
 * It executes all the algorithms in sequence for several events in flight.
 * It pulls the algorithms from the AlgResourcePool
 *  @author  Daniel Funke
 *  @version 0.1
 */
class ParallelSequentialSchedulerSvc: public extends<Service,
                                                     IScheduler> {
public:
  /// Constructor
  ParallelSequentialSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~ParallelSequentialSchedulerSvc() override;

  /// Initialise
  StatusCode initialize() override;

  /// Finalise
  StatusCode finalize() override;

  /// Make an event available to the scheduler
  StatusCode pushNewEvent(EventContext* eventContext) override;

  // Make multiple events available to the scheduler
  StatusCode pushNewEvents(std::vector<EventContext*>& eventContexts) override;

  /// Blocks until an event is availble
  StatusCode popFinishedEvent(EventContext*& eventContext) override;

  /// Try to fetch an event from the scheduler
  StatusCode tryPopFinishedEvent(EventContext*& eventContext) override;

  /// Get free slots number
  unsigned int freeSlots() override;


private:

  /// Decide if the top alglist or its flat version has to be used
  bool m_useTopAlgList;

  /// Cache the list of algs to be executed
  std::list<IAlgorithm*> m_algList;

  /// Queue of finished events
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  /// Atomic to account for asyncronous updates by the scheduler wrt the rest
  // number of events in flight
  std::atomic_int m_freeSlots;

  /// A shortcut to the whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard;

  /// The whiteboard name
  std::string m_whiteboardSvcName;

  /// Cache for the algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;

  /// Algorithm Execution State manager
  SmartIF<IAlgExecStateSvc> m_aess;

  /// Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose
  int m_threadPoolSize;

  //TBB scheduler
  std::unique_ptr<tbb::task_scheduler_init> m_tbb_sched;

  //control flow manager
  concurrency::ExecutionFlowManager m_controlFlow;

  /// Vector to bookkeep the information necessary to the index2name conversion
  std::vector<std::string> m_algname_vect;

  /// Map to bookkeep the information necessary to the name2index conversion
  std::unordered_map<std::string,unsigned int> m_algname_index_map;

  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class SequentialTask;

};

class SequentialTask: public tbb::task {
public:
  SequentialTask(ISvcLocator* svcLocator,
                 EventContext* eventContext,
                 ParallelSequentialSchedulerSvc* scheduler,
                 IAlgResourcePool* algPool,
                 IAlgExecStateSvc* aem):

                   m_serviceLocator(svcLocator),
                   m_eventContext(eventContext),
                   m_scheduler(scheduler),
                   m_algPool(algPool),
                   m_aess(aem) {

  };
  tbb::task* execute() override;
private:
  SmartIF<ISvcLocator> m_serviceLocator;
  EventContext* m_eventContext;
  SmartIF<ParallelSequentialSchedulerSvc> m_scheduler;
  SmartIF<IAlgResourcePool> m_algPool;
  SmartIF<IAlgExecStateSvc> m_aess;
};

#endif // GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H

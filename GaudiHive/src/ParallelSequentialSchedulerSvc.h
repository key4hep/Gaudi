#ifndef GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H
#define GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/EventContext.h"

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
class ParallelSequentialSchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  ParallelSequentialSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~ParallelSequentialSchedulerSvc();

  /// Initialise
  virtual StatusCode initialize();

  /// Finalise
  virtual StatusCode finalize();

  /// Make an event available to the scheduler
  virtual StatusCode pushNewEvent(EventContext* eventContext);

  // Make multiple events available to the scheduler
  virtual StatusCode pushNewEvents(std::vector<EventContext*>& eventContexts);

  /// Blocks until an event is availble
  virtual StatusCode popFinishedEvent(EventContext*& eventContext);

  /// Try to fetch an event from the scheduler
  virtual StatusCode tryPopFinishedEvent(EventContext*& eventContext);

  /// Get free slots number
  virtual unsigned int freeSlots();


private:

  /// Decide if the top alglist or its flat version has to be used
  bool m_useTopAlgList;

  /// Cache the list of algs to be executed
  std::list<IAlgorithm*> m_algList;

  /// The context of the event being processed
  EventContext* m_eventContext;

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

   /// Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose
    int m_threadPoolSize;

   //TBB scheduler
   tbb::task_scheduler_init* m_tbb_sched;

   //control flow manager
   concurrency::ExecutionFlowManager m_controlFlow;

   /// Vector to bookkeep the information necessary to the index2name conversion
   std::vector<std::string> m_algname_vect;

   /// Map to bookkeep the information necessary to the name2index conversion
   std::unordered_map<std::string,unsigned int> m_algname_index_map;

   /// Ugly, will disappear when the deps are declared only within the C++ code of the algos.
   std::vector<std::vector<std::string>> m_algosDependencies;

  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class SequentialTask;

};

class SequentialTask: public tbb::task {
public:
    SequentialTask(ISvcLocator* svcLocator,
                   EventContext* eventContext,
                   ParallelSequentialSchedulerSvc* scheduler,
                   IAlgResourcePool* algPool):

                m_serviceLocator(svcLocator),
                m_eventContext(eventContext),
                m_scheduler(scheduler),
                m_algPool(algPool){

    };
    virtual tbb::task* execute();
private:
    SmartIF<ISvcLocator> m_serviceLocator;
    EventContext* m_eventContext;
    SmartIF<ParallelSequentialSchedulerSvc> m_scheduler;
    SmartIF<IAlgResourcePool> m_algPool;
};

#endif // GAUDIHIVE_PARALLELSEQUENTIALSCHEDULERSVC_H

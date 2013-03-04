#ifndef GAUDIHIVE_FORWARDSCHEDULERSVC_H
#define GAUDIHIVE_FORWARDSCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IRunable.h" 
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgResourcePool.h"

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>

// External libs
#include "tbb/concurrent_queue.h"


//---------------------------------------------------------------------------

/**@class SequentialSchedulerSvc SequentialSchedulerSvc.h 
 * 
 *  This SchedulerSvc implements the IScheduler interface. 
 * It executes all the algorithms in sequence, pulling them from the 
 * AlgResourcePool. No task level parallelism is involved.
 * Given its sequential nature, the scheduler does not run in its own thread.
 * It has therefore to be treated a as a THREAD UNSAFE and NON REENTRANT 
 * entity.
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class SequentialSchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  SequentialSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~SequentialSchedulerSvc();

  /// Initialise
  virtual StatusCode initialize();
  
  /// Finalise
  virtual StatusCode finalize();  

  /// Make an event available to the scheduler
  virtual StatusCode pushNewEvent(EventContext* eventContext);
  
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
  
  /// The number of free slots (0 or 1)
  int m_freeSlots;
  
};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

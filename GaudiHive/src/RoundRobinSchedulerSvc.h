#ifndef GAUDIHIVE_ROUNDROBINSCHEDULERSVC_H
#define GAUDIHIVE_ROUNDROBINSCHEDULERSVC_H

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

/**@class RoundRobinSchedulerSvc RoundRobinSchedulerSvc.h 
 * 
 * The RoundRobinSchedulerSvc implements the IScheduler interface.
 * It deals with  multiple events and tries to handle all events
 * algorithm type by algorithm type, using one single thread. 
 * It serves as simple implementation against the concurrent state machine
 * and provides a test for instruction cache locality 
 * 
 *  @author  Benedikt Hegner 
 *  @version 1.0
 */
class RoundRobinSchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  RoundRobinSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~RoundRobinSchedulerSvc();

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
  
  /// Queue of finished events
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;
  
  /// The number of free slots (0 or 1)
  int m_freeSlots;
  
};

#endif // GAUDIHIVE_ROUNDROBINSCHEDULERSVC_H

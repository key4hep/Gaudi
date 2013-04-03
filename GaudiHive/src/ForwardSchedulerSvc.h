#ifndef GAUDIHIVE_FORWARDSCHEDULERSVC_H
#define GAUDIHIVE_FORWARDSCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IRunable.h" 
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IHiveWhiteBoard.h"

// Local includes
#include "AlgsExecutionStates.h"
#include "ControlFlowManager.h"
#include "DataFlowManager.h"

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>

// External libs
#include "tbb/concurrent_queue.h"

typedef AlgsExecutionStates::State State;

//---------------------------------------------------------------------------

/**@class ForwardSchedulerSvc ForwardSchedulerSvc.h GaudiKernel/ForwardSchedulerSvc.h
 *
 *  The SchedulerSvc implements the IScheduler interface. It manages all the 
 *  execution states of the algorithms and interacts with the TBB runtime for 
 *  the algorithm tasks submission. A state machine takes care of the tracking 
 *  of the execution state of the algorithms.
 * 
 *  @author  Danilo Piparo
 *  @author  Benedikt Hegner
 *  @version 1.1
 */
class ForwardSchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  ForwardSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~ForwardSchedulerSvc();

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

  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  void m_activate();

  /// Deactivate scheduler
  StatusCode m_deactivate();

  /// Flag to track if the scheduler is active or not
  bool m_isActive;

  /// The thread in which the activate function runs
  std::thread m_thread;

  std::unordered_map<std::string,unsigned int> m_algname_index_map;

   /// Convert a name to an integer
  inline unsigned int m_algname2index(const std::string& algoname)  ;

  std::vector<std::string> m_algname_vect;  

  /// Convert an integer to a name
  inline const std::string& m_index2algname (unsigned int index) ;

  /// The whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard; 
  
  /// The whiteboard name
  std::string m_whiteboardSvcName; 

  // Event slots management -------------------------------------------------
    
  class EventSlot{
  public:
    EventSlot(const std::vector<std::vector<std::string>>& algoDependencies, 
              unsigned int numberOfAlgorithms,
              unsigned int numberOfControlFlowNodes,
              SmartIF<IMessageSvc> MS):
                eventContext(nullptr),
                algsStates(numberOfAlgorithms,MS),
                complete(false),
                dataFlowMgr(algoDependencies),
                controlFlowState(numberOfControlFlowNodes,-1){};
      
    ~EventSlot(){};

    void reset(EventContext* theeventContext){
      eventContext=theeventContext;
      algsStates.reset();
      dataFlowMgr.reset();
      complete=false;
      controlFlowState.assign(controlFlowState.size(),-1);
    };
    
    // Members ----
    EventContext* eventContext;
    AlgsExecutionStates algsStates; 
    bool complete;
    DataFlowManager dataFlowMgr;
    std::vector<int> controlFlowState;
  };   
  
  int m_maxEventsInFlight;
  std::vector<EventSlot> m_eventSlots;  
  /// This is atomic to account for asyncronous updates by the scheduler wrt the rest
  std::atomic_uint m_freeSlots;
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  StatusCode m_eventFailed(EventContext* eventContext);
  
  // States management ------------------------------------------------------

  unsigned int m_maxAlgosInFlight;
  unsigned int m_algosInFlight;

  typedef int EventSlotIndex;
  typedef unsigned int AlgoSlotIndex;

  /// Loop on algorithm in the slots and promote them to successive states
  StatusCode m_updateStates(EventSlotIndex si=-1);
  StatusCode m_promoteToControlReady(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToDataReady(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToExecuted(AlgoSlotIndex iAlgo, EventSlotIndex si, IAlgorithm* algo);
  StatusCode m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si);

  /// Check if the scheduling is in a stall
  StatusCode m_isStalled(EventSlotIndex si);

  // Algos Management -------------------------------------------------------
  SmartIF<IAlgResourcePool>  m_algResourcePool;

  // DP Super ugly, but will disappear when the deps are declared within the C++ code of the algos.
  std::vector<std::vector<std::string>> m_algosDependencies;

  /// Drain the actions present in the queue
  StatusCode m_drain();

  /// Size of the threadpool initialised by TBB
  unsigned int m_threadPoolSize;

  // Actions management -----------------------------------------------------
  typedef std::function<StatusCode ()> action;
  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;  

  // Member to take care of the control flow 
  concurrency::ControlFlowManager m_cfManager;
  
  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;

};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

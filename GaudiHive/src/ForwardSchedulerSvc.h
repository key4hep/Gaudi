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
 *  This is a forward scheduler: algorithms are scheduled for execution as soon
 *  as their data dependencies are available in the whiteboard.
 *  # Algorithms management
 *  The activate() method runs in a separate thread. It checks a TBB concurrent
 *  bounded queue of closures in a loop via the Pop method. This allows not to
 *  use a cpu entirely to check the presence of new actions to be taken. In
 *  other words, the asynchronous actions are serialised via the actions queue.
 *  Once a task terminates, a call to the promoteToExecuted method will be
 *  pushed into the actions queue. The promoteToExecuted method also triggers
 *  a call to the updateStates method, which brushes all algorithms, checking if
 *  their state can be changed. It's indeed possible that upon termination of an
 *  algorithm, the control flow and/or the data flow allow the submission of
 *  more algorithms.
 *  ## Algorithms dependencies
 *  There are two ways of declaring algorithms dependencies. One which is only
 *  temporarly available to ease developments consists in declaring them through
 *  AlgosDependencies property as a list of list. The order of these sublist
 *  must be the same one of the algorithms in the TopAlg list.
 *  The second one consists in declaring the data dependencies directly within
 *  the algorithms via data object handles.
 *  # Events management
 *  The scheduler accepts events to be processed (in the form of eventContexts)
 *  and releases processed events. This flow is implemented through three
 *  methods:
 *  * pushNewEvent: to make an event available to the scheduler.
 *  * tryPopFinishedEvent: to retrieve an event from the scheduler
 *  * popFinishedEvent: to retrieve an event from the scheduler (blocking)
 *
 * Please refer to the full documentation of the methods for more details.
 *
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

  // Make multiple events available to the scheduler
  virtual StatusCode pushNewEvents(std::vector<EventContext*>& eventContexts);

  /// Blocks until an event is availble
  virtual StatusCode popFinishedEvent(EventContext*& eventContext);

  /// Try to fetch an event from the scheduler
  virtual StatusCode tryPopFinishedEvent(EventContext*& eventContext);

  /// Get free slots number
  virtual unsigned int freeSlots();


private:

  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  void activate();

  /// Deactivate scheduler
  StatusCode deactivate();

  /// Flag to track if the scheduler is active or not
  bool m_isActive;

  /// The thread in which the activate function runs
  std::thread m_thread;

  /// Convert a name to an integer
  inline unsigned int algname2index(const std::string& algoname);

  /// Map to bookkeep the information necessary to the name2index conversion
  std::unordered_map<std::string,unsigned int> m_algname_index_map;

  /// Convert an integer to a name
  inline const std::string& index2algname (unsigned int index);

  /// Vector to bookkeep the information necessary to the index2name conversion
  std::vector<std::string> m_algname_vect;

  /// A shortcut to the whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard;

  /// The whiteboard name
  std::string m_whiteboardSvcName;

  // Event slots management -------------------------------------------------
  /// Class representing the event slot
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

    /// Reset all resources in order to reuse the slot
    void reset(EventContext* theeventContext){
      eventContext=theeventContext;
      algsStates.reset();
      dataFlowMgr.reset();
      complete=false;
      controlFlowState.assign(controlFlowState.size(),-1);
    };

    /// Cache for the eventContext
    EventContext* eventContext;
    /// Vector of algorithms states
    AlgsExecutionStates algsStates;
    /// Flags completion of the event
    bool complete;
    /// DataFlowManager of this slot
    DataFlowManager dataFlowMgr;
    /// State of the control flow
    std::vector<int> controlFlowState;
  };

  /// Vector of events slots
  std::vector<EventSlot> m_eventSlots;

  /// Maximum number of event processed simultaneously
  int m_maxEventsInFlight;

  /// Atomic to account for asyncronous updates by the scheduler wrt the rest
  std::atomic_int m_freeSlots;

  /// Queue of finished events
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  /// Method to check if an event failed and take appropriate actions
  StatusCode eventFailed(EventContext* eventContext);

  /// List of events to be skipped. The number is the number in the job.
  std::vector<unsigned int> m_eventNumberBlacklist;

  // States management ------------------------------------------------------

  /// Maximum number of simultaneous algorithms
  unsigned int m_maxAlgosInFlight;

  /// Number of algoritms presently in flight
  unsigned int m_algosInFlight;

  /// Loop on algorithm in the slots and promote them to successive states (-1 means all slots, while empty string
  /// means skipping an update of the Control Flow state)
  StatusCode updateStates(int si=-1, const std::string& algo_name=std::string());

  /// Algorithm promotion: Accepted by the control flow
  StatusCode promoteToControlReady(unsigned int iAlgo, int si);
  StatusCode promoteToDataReady(unsigned int iAlgo, int si);
  StatusCode promoteToScheduled(unsigned int iAlgo, int si);
  StatusCode promoteToExecuted(unsigned int iAlgo, int si, IAlgorithm* algo);
  StatusCode promoteToFinished(unsigned int iAlgo, int si);

  /// Check if the scheduling is in a stall
  StatusCode isStalled(int si);

  /// Dump the state of the scheduler
  void dumpSchedulerState(int iSlot);

  /// Keep track of update actions scheduled
  bool m_updateNeeded;

  // Algos Management -------------------------------------------------------
  /// Cache for the algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;

  /// Ugly, will disappear when the deps are declared only within the C++ code of the algos.
  std::vector<std::vector<std::string>> m_algosDependencies;

  /// Drain the actions present in the queue
  StatusCode m_drain();

  /// Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose
  int m_threadPoolSize;

  // Actions management -----------------------------------------------------

  typedef std::function<StatusCode ()> action;

  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;

  /// Member to take care of the control flow
  concurrency::ControlFlowManager m_cfManager;
  // XXX: CF tests. Temporary property to switch between ControlFlow implementations
  bool m_CFNext;

  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;

};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

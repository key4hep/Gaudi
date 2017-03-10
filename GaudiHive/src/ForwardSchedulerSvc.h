#ifndef GAUDIHIVE_FORWARDSCHEDULERSVC_H
#define GAUDIHIVE_FORWARDSCHEDULERSVC_H

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// External libs
#include "tbb/concurrent_queue.h"

// Framework include files
#include "GaudiKernel/IAccelerator.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/Service.h"

// Local includes
#include "AlgsExecutionStates.h"
#include "DataFlowManager.h"
#include "EventSlot.h"
#include "ExecutionFlowManager.h"

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
class ForwardSchedulerSvc : public extends<Service, IScheduler>
{
public:
  /// Constructor
  using extends::extends;

  /// Destructor
  ~ForwardSchedulerSvc() override = default;

  /// Initialise
  StatusCode initialize() override;

  /// Finalise
  StatusCode finalize() override;

  /// Make an event available to the scheduler
  StatusCode pushNewEvent( EventContext* eventContext ) override;

  // Make multiple events available to the scheduler
  StatusCode pushNewEvents( std::vector<EventContext*>& eventContexts ) override;

  /// Blocks until an event is availble
  StatusCode popFinishedEvent( EventContext*& eventContext ) override;

  /// Try to fetch an event from the scheduler
  StatusCode tryPopFinishedEvent( EventContext*& eventContext ) override;

  /// Get free slots number
  unsigned int freeSlots() override;

private:
  enum ActivationState { INACTIVE = 0, ACTIVE = 1, FAILURE = 2 };

  Gaudi::Property<int> m_maxEventsInFlight{this, "MaxEventsInFlight", 0,
                                           "Maximum number of event processed simultaneously"};
  Gaudi::Property<int> m_threadPoolSize{
      this, "ThreadPoolSize", -1,
      "Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose"};
  Gaudi::Property<std::string> m_whiteboardSvcName{this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name"};
  Gaudi::Property<std::string> m_IOBoundAlgSchedulerSvcName{this, "IOBoundAlgSchedulerSvc", "IOBoundAlgSchedulerSvc"};
  Gaudi::Property<unsigned int> m_maxAlgosInFlight{this, "MaxAlgosInFlight", 1,
                                                   "[[deprecated]] Taken from the whiteboard"};

  Gaudi::Property<unsigned int> m_maxIOBoundAlgosInFlight{this, "MaxIOBoundAlgosInFlight", 0,
                                                          "Maximum number of simultaneous I/O-bound algorithms"};
  // XXX: CF tests. Temporary property to switch between ControlFlow implementations
  Gaudi::Property<bool> m_CFNext{this, "useGraphFlowManagement", false,
                                 "Temporary property to switch between ControlFlow implementations"};
  // XXX: CF tests. Temporary property to switch between DataFlow implementations
  Gaudi::Property<bool> m_DFNext{this, "DataFlowManagerNext", false,
                                 "Temporary property to switch between DataFlow implementations"};
  Gaudi::Property<bool> m_simulateExecution{
      this, "SimulateExecution", false,
      "Flag to perform single-pass simulation of execution flow before the actual execution"};
  Gaudi::Property<std::string> m_optimizationMode{this, "Optimizer", "",
                                                  "The following modes are currently available: PCE, COD, DRE,  E"};
  Gaudi::Property<bool> m_dumpIntraEventDynamics{this, "DumpIntraEventDynamics", false,
                                                 "Dump intra-event concurrency dynamics to csv file"};
  Gaudi::Property<bool> m_useIOBoundAlgScheduler{this, "PreemptiveIOBoundTasks", false,
                                                 "Turn on preemptive way of scheduling of I/O-bound algorithms"};
  Gaudi::Property<std::vector<std::vector<std::string>>> m_algosDependencies{
      this, "AlgosDependencies", {}, "[[deprecated]]"};
  Gaudi::Property<bool> m_checkDeps{this, "CheckDependencies", false, 
      "Runtime check of Algorithm Data Dependencies"};
  Gaudi::Property<std::string> m_useDataLoader{this, "DataLoaderAlg", "",
      "Attribute unmet input dependencies to this DataLoader Algorithm"};

  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  void activate();

  /// Deactivate scheduler
  StatusCode deactivate();

  /// Flag to track if the scheduler is active or not
  std::atomic<ActivationState> m_isActive{INACTIVE};

  /// The thread in which the activate function runs
  std::thread m_thread;

  /// Convert a name to an integer
  inline unsigned int algname2index( const std::string& algoname );

  /// Map to bookkeep the information necessary to the name2index conversion
  std::unordered_map<std::string, unsigned int> m_algname_index_map;

  /// Convert an integer to a name
  inline const std::string& index2algname( unsigned int index );

  /// Vector to bookkeep the information necessary to the index2name conversion
  std::vector<std::string> m_algname_vect;

  /// A shortcut to the whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard;

  /// A shortcut to IO-bound algorithm scheduler
  SmartIF<IAccelerator> m_IOBoundAlgScheduler;

  /// Vector of events slots
  std::vector<EventSlot> m_eventSlots;

  /// Atomic to account for asyncronous updates by the scheduler wrt the rest
  std::atomic_int m_freeSlots;

  /// Queue of finished events
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  /// Method to check if an event failed and take appropriate actions
  StatusCode eventFailed( EventContext* eventContext );

  /// Algorithm execution state manager
  SmartIF<IAlgExecStateSvc> m_algExecStateSvc;

  // States management ------------------------------------------------------

  /// Number of algoritms presently in flight
  unsigned int m_algosInFlight = 0;

  /// Number of algoritms presently in flight
  unsigned int m_IOBoundAlgosInFlight = 0;

  /// Loop on algorithm in the slots and promote them to successive states (-1 means all slots, while empty string
  /// means skipping an update of the Control Flow state)
  StatusCode updateStates( int si = -1, const std::string& algo_name = std::string() );

  /// Algorithm promotion: Accepted by the control flow
  StatusCode promoteToControlReady( unsigned int iAlgo, int si );
  StatusCode promoteToDataReady( unsigned int iAlgo, int si );
  StatusCode promoteToScheduled( unsigned int iAlgo, int si );
  StatusCode promoteToAsyncScheduled( unsigned int iAlgo, int si ); // tests of an asynchronous scheduler
  StatusCode promoteToExecuted( unsigned int iAlgo, int si, IAlgorithm* algo, EventContext* );
  StatusCode promoteToAsyncExecuted( unsigned int iAlgo, int si, IAlgorithm* algo,
                                     EventContext* ); // tests of an asynchronous scheduler
  StatusCode promoteToFinished( unsigned int iAlgo, int si );

  /// Check if the scheduling is in a stall
  StatusCode isStalled( int si );

  /// Dump the state of the scheduler
  void dumpSchedulerState( int iSlot );

  /// Keep track of update actions scheduled
  bool m_updateNeeded = true;

  // Algos Management -------------------------------------------------------
  /// Cache for the algorithm resource pool
  SmartIF<IAlgResourcePool> m_algResourcePool;

  /// Drain the actions present in the queue
  StatusCode m_drain();

  // Actions management -----------------------------------------------------

  typedef std::function<StatusCode()> action;

  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;

  /// Member to take care of the control flow
  concurrency::ExecutionFlowManager m_efManager;

  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;
  friend class IOBoundAlgTask;

  // Service for thread pool initialization
  SmartIF<IThreadPoolSvc> m_threadPoolSvc;

  bool m_first = true;

  class SchedulerState
  {

  public:
    SchedulerState( Algorithm* a, EventContext* e, pthread_t t ) : m_a( a ), m_e( *e ), m_t( t ) {}

    Algorithm* alg() const { return m_a; }
    EventContext ctx() const { return m_e; }
    pthread_t thread() const { return m_t; }

    friend std::ostream& operator<<( std::ostream& os, const SchedulerState& ss )
    {
      os << ss.ctx() << "  a: " << ss.alg()->name() << " [" << std::hex << ss.alg() << std::dec << "]  t: 0x"
         << std::hex << ss.thread() << std::dec;
      return os;
    }

    bool operator==( const SchedulerState& ss ) const { return ( m_a == ss.alg() ); }

    bool operator==( Algorithm* a ) const { return ( m_a == a ); }

    bool operator<( const SchedulerState& rhs ) const { return ( m_a < rhs.alg() ); }

  private:
    Algorithm* m_a;
    EventContext m_e;
    pthread_t m_t;
  };

  static std::list<SchedulerState> m_sState;
  static std::mutex m_ssMut;

public:
  void addAlg( Algorithm*, EventContext*, pthread_t );
  bool delAlg( Algorithm* );
  void dumpState() override;

private:
  void dumpState( std::ostringstream& );
};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

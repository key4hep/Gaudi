#ifndef GAUDIHIVE_FORWARDSCHEDULERSVC_H
#define GAUDIHIVE_FORWARDSCHEDULERSVC_H

// Local includes
#include "AlgsExecutionStates.h"
#include "DataFlowManager.h"
#include "EventSlot.h"
#include "ExecutionFlowManager.h"

// Framework include files
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/Service.h"

// C++ include files
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// External libs
#include "tbb/concurrent_queue.h"
#include "tbb/task.h"

typedef AlgsExecutionStates::State State;
typedef std::function<StatusCode()> action;

//---------------------------------------------------------------------------

/**@class ForwardSchedulerSvc ForwardSchedulerSvc.h GaudiKernel/ForwardSchedulerSvc.h
 *
 *  The SchedulerSvc implements the IScheduler interface. It manages all the
 *  execution states of the algorithms and interacts with the TBB runtime for
 *  the algorithm tasks submission. A state machine takes care of the tracking
 *  of the execution state of the algorithms.
 *
 *  This is a forward scheduler: an algorithm is scheduled for execution as soon
 *  as its data dependencies are available in the whiteboard, and as soon as a
 *  worker thread becomes available. The scheduler has no means for automatic
 *  intra-event throughput maximization. Consider the AvalancheScheduler if you
 *  need those.
 *
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
 *
 *  ## Algorithms dependencies
 *  There are two ways of declaring algorithms dependencies. One which is only
 *  temporarly available to ease developments consists in declaring them through
 *  AlgosDependencies property as a list of list. The order of these sublist
 *  must be the same one of the algorithms in the TopAlg list.
 *  The second one consists in declaring the data dependencies directly within
 *  the algorithms via data object handles.
 *
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
  Gaudi::Property<int> m_threadPoolSize{this, "ThreadPoolSize", -1,
    "Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose"};
  Gaudi::Property<std::string> m_whiteboardSvcName{this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name"};
  Gaudi::Property<unsigned int> m_maxAlgosInFlight{this, "MaxAlgosInFlight", 1,
                                                   "[[deprecated]] Taken from the whiteboard"};
  Gaudi::Property<std::vector<std::vector<std::string>>> m_algosDependencies{
      this, "AlgosDependencies", {}, "[[deprecated]]"};
  Gaudi::Property<bool> m_checkDeps{this, "CheckDependencies", false, 
      "Runtime check of Algorithm Data Dependencies"};
  Gaudi::Property<std::string> m_useDataLoader{this, "DataLoaderAlg", "",
      "Attribute unmet input dependencies to this DataLoader Algorithm"};

  Gaudi::Property<bool> m_showDataDeps{this, "ShowDataDependencies", true,
      "Show the INPUT and OUTPUT data dependencies of Algorithms"};
  Gaudi::Property<bool> m_showDataFlow{this, "ShowDataFlow", false,
      "Show the configuration of DataFlow between Algorithms"};
    Gaudi::Property<bool> m_showControlFlow{this, "ShowControlFlow", false,
      "Show the configuration of all Algorithms and Sequences"};

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

  /// Loop on algorithm in the slots and promote them to successive states (-1 means all slots, while empty string
  /// means skipping an update of the Control Flow state)
  StatusCode updateStates( int si = -1 );

  /// Algorithm promotion: Accepted by the control flow
  StatusCode promoteToControlReady( unsigned int iAlgo, int si );
  StatusCode promoteToDataReady( unsigned int iAlgo, int si );
  StatusCode promoteToScheduled( unsigned int iAlgo, int si );
  StatusCode promoteToExecuted( unsigned int iAlgo, int si, IAlgorithm* algo, EventContext* );
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

  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;

  // helper task to enqueue the scheduler's actions (closures)
  struct enqueueSchedulerActionTask: public tbb::task {

    std::function<StatusCode()> m_closure;
    SmartIF<ForwardSchedulerSvc> m_scheduler;

    enqueueSchedulerActionTask(ForwardSchedulerSvc* scheduler, std::function<StatusCode()> _closure) :
      m_closure(_closure), m_scheduler(scheduler) {}

    tbb::task* execute() override {
      m_scheduler->m_actionsQueue.push(m_closure);
      return nullptr;
    }
  };

  // ------------------------------------------------------------------------

  /// Member to take care of the control flow
  concurrency::ExecutionFlowManager m_efManager;

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
  concurrency::PrecedenceRulesGraph* m_efg;
};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

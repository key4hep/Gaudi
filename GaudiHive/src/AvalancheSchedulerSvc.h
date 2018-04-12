#ifndef GAUDIHIVE_AVALANCHESCHEDULERSVC_H
#define GAUDIHIVE_AVALANCHESCHEDULERSVC_H

// Local includes
#include "AlgsExecutionStates.h"
#include "EventSlot.h"
#include "PrecedenceSvc.h"

// Framework include files
#include "GaudiKernel/IAccelerator.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/ICondSvc.h"
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

class IAlgorithm;

typedef AlgsExecutionStates::State  State;
typedef std::function<StatusCode()> action;

//---------------------------------------------------------------------------

/**@class AvalancheSchedulerSvc AvalancheSchedulerSvc.h
 *
 *  # Introduction
 *
 *  The scheduler is named after its ability to generically maximize the average
 *  intra-event task occupancy by inducing avalanche-like concurrency disclosure
 *  waves in conditions of arbitrary intra-event task precedence constraints
 *  (see section 3.2 of http://cern.ch/go/7Jn7).
 *
 *
 *  # Task precedence management
 *
 *  The scheduler is driven by *graph-based* task precedence management. When
 *  compared to approach used in the ForwardSchedulerSvc, the following advantages
 *  can be emphasized:
 *
 *   (1) Faster decision making (thus lower concurrency disclosure downtime);
 *   (2) Capacity for proactive task scheduling decision making.
 *
 *  Point (2) allowed to implement a number of generic, non-intrusive intra-event
 *  throughput maximization scheduling strategies.
 *
 *
 *  # Scheduling principles
 *
 *   o Task scheduling prerequisites
 *
 *     A task is scheduled ASA all following conditions are met:
 *     - if a control flow (CF) graph traversal reaches the task;
 *     - when all data flow (DF) dependencies of the task are satisfied;
 *     - when the DF-ready task pool parsing mechanism (*) considers it, and:
 *       - a free (or re-entrant) algorithm instance to run within the task is
 *         available;
 *       - there is a free computational resource to run the task.
 *
 *   o (*) Avalanche induction strategies
 *
 *     The scheduler is able to maximize the intra-event throughput by applying
 *     several search strategies within the pool, prioritizing tasks according
 *     to the following types of precedence rules graph asymmetries:
 *
 *      (A) Local task-to-data asymmetry;
 *      (B) Local task-to-task asymmetry;
 *      (C) Global task-to-task asymmetry.
 *
 *
 *   o Other mechanisms of throughput maximization
 *
 *     The scheduler is able to maximize the overall throughput of data processing
 *     by scheduling the CPU-blocking tasks efficiently. The mechanism can be
 *     applied to the following types of tasks:
 *     - I/O-bound tasks;
 *     - tasks with computation offloading (accelerators, GPGPUs, clouds,
 *       quantum computing devices..joke);
 *     - synchronization-bound tasks.
 *
 *
 *  # Credits
 *  Historically, the AvalancheSchedulerSvc branched off the ForwardSchedulerSvc
 *  and in many ways built its success on ideas and code of the latter.
 *
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */
class AvalancheSchedulerSvc : public extends<Service, IScheduler>
{
public:
  /// Constructor
  using extends::extends;

  /// Destructor
  ~AvalancheSchedulerSvc() override = default;

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

  /// Method to inform the scheduler about event views
  virtual StatusCode scheduleEventView( EventContext const* sourceContext, std::string const& nodeName,
                                        EventContext* viewContext ) override;

private:
  enum ActivationState { INACTIVE = 0, ACTIVE = 1, FAILURE = 2 };

  Gaudi::Property<int> m_threadPoolSize{
      this, "ThreadPoolSize", -1,
      "Size of the threadpool initialised by TBB; a value of -1 gives TBB the freedom to choose"};
  Gaudi::Property<std::string>  m_whiteboardSvcName{this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name"};
  Gaudi::Property<std::string>  m_IOBoundAlgSchedulerSvcName{this, "IOBoundAlgSchedulerSvc", "IOBoundAlgSchedulerSvc"};
  Gaudi::Property<unsigned int> m_maxIOBoundAlgosInFlight{this, "MaxIOBoundAlgosInFlight", 0,
                                                          "Maximum number of simultaneous I/O-bound algorithms"};
  Gaudi::Property<bool> m_simulateExecution{
      this, "SimulateExecution", false,
      "Flag to perform single-pass simulation of execution flow before the actual execution"};
  Gaudi::Property<std::string> m_optimizationMode{this, "Optimizer", "",
                                                  "The following modes are currently available: PCE, COD, DRE,  E"};
  Gaudi::Property<bool> m_dumpIntraEventDynamics{this, "DumpIntraEventDynamics", false,
                                                 "Dump intra-event concurrency dynamics to csv file"};
  Gaudi::Property<bool> m_useIOBoundAlgScheduler{this, "PreemptiveIOBoundTasks", false,
                                                 "Turn on preemptive way of scheduling of I/O-bound algorithms"};

  Gaudi::Property<bool> m_checkDeps{this, "CheckDependencies", false, "Runtime check of Algorithm Data Dependencies"};

  Gaudi::Property<std::string> m_useDataLoader{this, "DataLoaderAlg", "",
                                               "Attribute unmet input dependencies to this DataLoader Algorithm"};

  Gaudi::Property<bool> m_enableCondSvc{this, "EnableConditions", false, "Enable ConditionsSvc"};

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

  /// A shortcut to the Precedence Service
  SmartIF<IPrecedenceSvc> m_precSvc;

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

  /// A shortcut to service for Conditions handling
  SmartIF<ICondSvc> m_condSvc;

  // States management ------------------------------------------------------

  /// Number of algoritms presently in flight
  unsigned int m_algosInFlight = 0;

  /// Number of algoritms presently in flight
  unsigned int m_IOBoundAlgosInFlight = 0;

  /// Loop on algorithm in the slots and promote them to successive states
  /// (-1 for algo_index means skipping an update of the Control Flow state)
  StatusCode updateStates( int si = -1, int algo_index = -1, EventContext* = nullptr );

  /// Algorithm promotion
  StatusCode promoteToScheduled( unsigned int iAlgo, int si, EventContext* );
  StatusCode promoteToAsyncScheduled( unsigned int iAlgo, int si, EventContext* ); // tests of an asynchronous scheduler
  StatusCode promoteToExecuted( unsigned int iAlgo, int si, IAlgorithm* algo, EventContext* );
  StatusCode promoteToAsyncExecuted( unsigned int iAlgo, int si, IAlgorithm* algo,
                                     EventContext* ); // tests of an asynchronous scheduler
  StatusCode promoteToFinished( unsigned int iAlgo, int si );

  /// Check if the scheduling is in a stall
  StatusCode isStalled( int si );

  /// Dump the state of the scheduler
  void dumpSchedulerState( int iSlot );

  // Algos Management -------------------------------------------------------
  /// Cache for the algorithm resource pool
  SmartIF<IAlgResourcePool> m_algResourcePool;

  /// Drain the actions present in the queue
  StatusCode m_drain();

  // Actions management -----------------------------------------------------

  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;

  // ------------------------------------------------------------------------

  // Service for thread pool initialization
  SmartIF<IThreadPoolSvc> m_threadPoolSvc;
  size_t                  m_maxEventsInFlight{0};
  size_t                  m_maxAlgosInFlight{1};
  bool                    m_first = true;
};

#endif // GAUDIHIVE_AVALANCHESCHEDULERSVC_H

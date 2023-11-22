/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIHIVE_AVALANCHESCHEDULERSVC_H
#define GAUDIHIVE_AVALANCHESCHEDULERSVC_H

// Local includes
#include "AlgsExecutionStates.h"
#include "EventSlot.h"
#include "FiberManager.h"
#include "PrecedenceSvc.h"

// Framework include files
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
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

// External libs
#include "tbb/concurrent_priority_queue.h"
#include "tbb/concurrent_queue.h"
#include "tbb/task_arena.h"

class IAlgorithm;

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
 *     by preemptive scheduling CPU-blocking tasks. The mechanism can be applied
 *     to the following types of tasks:
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
class AvalancheSchedulerSvc : public extends<Service, IScheduler> {

  friend class AlgTask;

public:
  /// Constructor
  using extends::extends;

  /// Initialise
  StatusCode initialize() override;

  /// Finalise
  StatusCode finalize() override;

  /// Make an event available to the scheduler
  StatusCode pushNewEvent( EventContext* eventContext ) override;

  // Make multiple events available to the scheduler
  StatusCode pushNewEvents( std::vector<EventContext*>& eventContexts ) override;

  /// Blocks until an event is available
  StatusCode popFinishedEvent( EventContext*& eventContext ) override;

  /// Try to fetch an event from the scheduler
  StatusCode tryPopFinishedEvent( EventContext*& eventContext ) override;

  /// Get free slots number
  unsigned int freeSlots() override;

  /// Dump scheduler state for all slots
  void dumpState() override;

  /// Method to inform the scheduler about event views
  virtual StatusCode scheduleEventView( const EventContext* sourceContext, const std::string& nodeName,
                                        std::unique_ptr<EventContext> viewContext ) override;

  /// Sample occupancy at fixed interval (ms)
  /// Negative value to deactivate, 0 to snapshot every change
  /// Each sample, apply the callback function to the result
  virtual void recordOccupancy( int samplePeriod, std::function<void( OccupancySnapshot )> callback ) override;

private:
  using AState = AlgsExecutionStates::State;
  using action = std::function<StatusCode()>;

  enum ActivationState { INACTIVE = 0, ACTIVE = 1, FAILURE = 2 };

  // Occupancy snapshot data
  std::chrono::duration<int64_t, std::milli> m_snapshotInterval = std::chrono::duration<int64_t, std::milli>::min();
  std::chrono::system_clock::time_point      m_lastSnapshot     = std::chrono::system_clock::now();
  std::function<void( OccupancySnapshot )>   m_snapshotCallback;

  Gaudi::Property<int> m_threadPoolSize{
      this, "ThreadPoolSize", -1,
      "Size of the global thread pool initialised by TBB; a value of -1 requests to use"
      "all available hardware threads; -100 requests to bypass TBB executing "
      "all algorithms in the scheduler's thread." };
  Gaudi::Property<int> m_maxParallelismExtra{
      this, "maxParallelismExtra", 0,
      "Allows to add some extra threads to the maximum parallelism set in TBB"
      "The TBB max parallelism is set as: ThreadPoolSize + maxParallelismExtra + 1" };
  Gaudi::Property<std::string>  m_whiteboardSvcName{ this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name" };
  Gaudi::Property<unsigned int> m_maxBlockingAlgosInFlight{
      this, "MaxBlockingAlgosInFlight", 0, "Maximum allowed number of simultaneously running CPU-blocking algorithms" };
  Gaudi::Property<bool> m_simulateExecution{
      this, "SimulateExecution", false,
      "Flag to perform single-pass simulation of execution flow before the actual execution" };
  Gaudi::Property<std::string> m_optimizationMode{ this, "Optimizer", "",
                                                   "The following modes are currently available: PCE, COD, DRE,  E" };
  Gaudi::Property<bool>        m_dumpIntraEventDynamics{ this, "DumpIntraEventDynamics", false,
                                                  "Dump intra-event concurrency dynamics to csv file" };
  Gaudi::Property<bool>        m_enablePreemptiveBlockingTasks{
      this, "PreemptiveBlockingTasks", false,
      "Enable preemptive scheduling of CPU-blocking algorithms. Blocking algorithms must be flagged accordingly." };
  Gaudi::Property<int> m_numOffloadThreads{
      this, "NumOffloadingThreads", 2,
      "Number of threads to use for CPU portion of accelerated algorithms. Accelerated algorithms must be flagged and "
      "use Boost Fiber functionality to suspend while waiting for offloaded work." };
  Gaudi::Property<bool>                     m_checkDeps{ this, "CheckDependencies", false,
                                     "Runtime check of Algorithm Input Data Dependencies" };
  Gaudi::Property<bool>                     m_checkOutput{ this, "CheckOutputUsage", false,
                                       "Runtime check of Algorithm Output Data usage" };
  Gaudi::Property<std::vector<std::string>> m_checkOutputIgnoreList{
      this,
      "CheckOutputUsageIgnoreList",
      {},
      "Ignore outputs of the Algorithms of this name when doing the check",
      "OrderedSet<std::string>" };

  Gaudi::Property<std::string> m_useDataLoader{ this, "DataLoaderAlg", "",
                                                "Attribute unmet input dependencies to this DataLoader Algorithm" };

  Gaudi::Property<bool> m_enableCondSvc{ this, "EnableConditions", false, "Enable ConditionsSvc" };

  Gaudi::Property<bool> m_showDataDeps{ this, "ShowDataDependencies", true,
                                        "Show the INPUT and OUTPUT data dependencies of Algorithms" };

  Gaudi::Property<bool> m_showDataFlow{ this, "ShowDataFlow", false,
                                        "Show the configuration of DataFlow between Algorithms" };

  Gaudi::Property<bool> m_showControlFlow{ this, "ShowControlFlow", false,
                                           "Show the configuration of all Algorithms and Sequences" };

  Gaudi::Property<bool> m_verboseSubSlots{ this, "VerboseSubSlots", false, "Dump algorithm states for all sub-slots" };

  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  void activate();

  /// Deactivate scheduler
  StatusCode deactivate();

  /// Flag to track if the scheduler is active or not
  std::atomic<ActivationState> m_isActive{ INACTIVE };

  /// The thread in which the activate function runs
  std::thread m_thread;

  /// Convert a name to an integer
  inline unsigned int algname2index( const std::string& algoname ) { return m_algname_index_map[algoname]; };

  /// Map to bookkeep the information necessary to the name2index conversion
  std::unordered_map<std::string, unsigned int> m_algname_index_map;

  /// Convert an integer to a name
  inline const std::string& index2algname( unsigned int index ) { return m_algname_vect[index]; };

  /// Vector to bookkeep the information necessary to the index2name conversion
  std::vector<std::string> m_algname_vect;

  /// A shortcut to the Precedence Service
  SmartIF<IPrecedenceSvc> m_precSvc;

  /// A shortcut to the whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard;

  /// Vector of events slots
  std::vector<EventSlot> m_eventSlots;

  /// Atomic to account for asyncronous updates by the scheduler wrt the rest
  std::atomic_int m_freeSlots{ 0 };

  /// Queue of finished events
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  /// Algorithm execution state manager
  SmartIF<IAlgExecStateSvc> m_algExecStateSvc;

  /// A shortcut to service for Conditions handling
  SmartIF<ICondSvc> m_condSvc;

  /// Number of algorithms presently in flight
  unsigned int m_algosInFlight = 0;

  /// Number of algorithms presently in flight
  unsigned int m_blockingAlgosInFlight = 0;

  // States management ------------------------------------------------------

  /// Loop on all slots to schedule DATAREADY algorithms and sign off ready events
  StatusCode iterate();

  // Update algorithm state and, optionally, revise states of other downstream algorithms
  StatusCode revise( unsigned int iAlgo, EventContext* contextPtr, AState state, bool iterate = false );

  /// Algorithm scheduling
  struct TaskSpec;
  StatusCode schedule( TaskSpec&& );
  StatusCode signoff( const TaskSpec& );

  /// Check if scheduling in a particular slot is in a stall
  bool isStalled( const EventSlot& ) const;
  /// Method to execute if an event failed
  void eventFailed( EventContext* eventContext );

  /// Dump the state of the scheduler
  void dumpSchedulerState( int iSlot );

  // Algos Management -------------------------------------------------------

  /// Cache for the algorithm resource pool
  SmartIF<IAlgResourcePool> m_algResourcePool;

  // Actions management -----------------------------------------------------

  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;

  /// Struct to hold entries in the alg queues
  struct TaskSpec {
    /// Default constructor
    TaskSpec(){};
    TaskSpec( IAlgorithm* algPtr, unsigned int algIndex, const std::string& algName, unsigned int algRank,
              bool blocking, bool accelerated, int slotIndex, EventContext* eventContext )
        : algPtr( algPtr )
        , algIndex( algIndex )
        , algName( algName )
        , algRank( algRank )
        , blocking( blocking )
        , accelerated ( accelerated )
        , slotIndex( slotIndex )
        , contextPtr( eventContext ){};
    /// Copy constructor (to keep a lambda capturing a TaskSpec storable as a std::function value)
    TaskSpec( const TaskSpec& ) = default;
    /// Assignment operator
    TaskSpec& operator=( const TaskSpec& ) = delete;
    /// Move constructor
    TaskSpec( TaskSpec&& ) = default;
    /// Move assignment
    TaskSpec& operator=( TaskSpec&& ) = default;

    IAlgorithm*      algPtr{ nullptr };
    unsigned int     algIndex{ 0 };
    std::string_view algName;
    unsigned int     algRank{ 0 };
    bool             blocking{ false };
    bool             accelerated{ false };
    int              slotIndex{ 0 };
    EventContext*    contextPtr{ nullptr };
  };

  /// Comparison operator to sort the queues
  struct AlgQueueSort {
    bool operator()( const TaskSpec& i, const TaskSpec& j ) const { return ( i.algRank < j.algRank ); }
  };

  /// Queues for scheduled algorithms
  tbb::concurrent_priority_queue<TaskSpec, AlgQueueSort> m_scheduledQueue;
  tbb::concurrent_priority_queue<TaskSpec, AlgQueueSort> m_scheduledBlockingQueue;
  tbb::concurrent_priority_queue<TaskSpec, AlgQueueSort> m_scheduledAcceleratedQueue;
  std::queue<TaskSpec>                                   m_retryQueue;

  // Prompt the scheduler to call updateStates
  std::atomic<bool> m_needsUpdate{ true };

  // ------------------------------------------------------------------------

  // Service for thread pool initialization
  SmartIF<IThreadPoolSvc> m_threadPoolSvc;
  tbb::task_arena*        m_arena{ nullptr };
  FiberManager*           m_fiberManager{ nullptr };

  size_t                  m_maxEventsInFlight{ 0 };
  size_t                  m_maxAlgosInFlight{ 1 };

public:
  // get next schedule-able TaskSpec
  bool next( TaskSpec& ts, bool blocking, bool accelerated ) {
    if ( accelerated ) { return m_scheduledAcceleratedQueue.try_pop( ts ); }
    if ( blocking ) { return m_scheduledBlockingQueue.try_pop( ts ); }
    return m_scheduledQueue.try_pop( ts );
  };
};

#endif // GAUDIHIVE_AVALANCHESCHEDULERSVC_H

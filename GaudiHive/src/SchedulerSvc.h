#ifndef GAUDIKERNEL_SCHEDULERSVC_H
#define GAUDIKERNEL_SCHEDULERSVC_H

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

// External libs
#include "tbb/concurrent_queue.h"

//---------------------------------------------------------------------------

  
class AlgsExecutionStates{
public:
  
  enum State {
    GROUND,
    CONTROLFLOWALLOWED,
    DATAFLOWALLOWED,
    MAXINFLIGHTALLOWED,
    SCHEDULED,
    FINISHED
    };     
    
  AlgsExecutionStates(unsigned int algsNumber, SmartIF<IMessageSvc> MS):
    m_states(algsNumber,GROUND),
    m_MS(MS){};    
  ~AlgsExecutionStates(){};
  StatusCode updateState(unsigned int iAlgo,State newState){
    MsgStream log(m_MS, "AlgExecutionStates");
    switch (newState) {
      case GROUND:
        log << MSG::ERROR << "A transition to GROUND is not defined.";
        return StatusCode::FAILURE;
      //
      case CONTROLFLOWALLOWED:
        if (m_states[iAlgo]!=GROUND){
          log << MSG::ERROR << "Transition to CONTROLFLOWALLOWED possible only from GROUND state!";
          return StatusCode::FAILURE;              
        }
      //
      case DATAFLOWALLOWED:
        if (m_states[iAlgo]!=CONTROLFLOWALLOWED){
          log << MSG::ERROR  << "Transition to DATAFLOWALLOWED possible only from CONTROLFLOWALLOWED state!";
          return StatusCode::FAILURE;              
        }
      //
      case MAXINFLIGHTALLOWED:
        if (m_states[iAlgo]!=DATAFLOWALLOWED){
          log << MSG::ERROR  << "Transition to MAXINFLIGHTALLOWED possible only from DATAFLOWALLOWED state!";
          return StatusCode::FAILURE;              
        }
      //
      case SCHEDULED:
        if (m_states[iAlgo]!=MAXINFLIGHTALLOWED){
          log << MSG::ERROR  << "Transition to SCHEDULED possible only from MAXINFLIGHTALLOWED state!";
          return StatusCode::FAILURE;              
        }          
      //
      case FINISHED:
        if (m_states[iAlgo]!=SCHEDULED){
          log << MSG::ERROR  << "Transition to FINISHED possible only from SCHEDULED state!";
          return StatusCode::FAILURE;              
        }                         
      //
      default:
        log << MSG::ERROR  << "Undefined state!";
        return StatusCode::FAILURE;             
    }        
  return StatusCode::SUCCESS; 
  };
  void reset(){m_states.assign(m_states.size(),GROUND);};
private:
  std::vector<State>  m_states;
  SmartIF<IMessageSvc> m_MS;  
  
};



//---------------------------------------------------------------------------

/**@class SchedulerSvc SchedulerSvc.h GaudiKernel/SchedulerSvc.h
 *
 *  The SchedulerSvc implements the IScheduler interface. It manages all the 
 *  execution states of the algorithms and interacts with the TBB runtime for 
 *  the algorithm tasks submission. A state machine takes care of the tracking 
 *  of the execution state of the algorithms.
 *
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class SchedulerSvc: public extends2<Service, IScheduler,IRunable> {
public:
  /// Constructor
  SchedulerSvc( const std::string& name, ISvcLocator* svc );
  
  /// Destructor
  ~SchedulerSvc();
  
  /// To start the scheduling in a different thread
  virtual StatusCode run ();
  
  /// Prepare the resources necessary for the scheduling
  virtual StatusCode initialize();
  
  /// Make an event available to the scheduler
  virtual StatusCode addEvent(EventContext* eventContext);
  
  /// Get number of in flight events
  virtual unsigned int eventsInFlight();
  
  /// Maximum number of events in flight
  virtual unsigned int maxEventsInFlight();
  
  /// Get number of completed events
  virtual unsigned int completedEvents();

private:

  // Utils and shortcuts ----------------------------------------------------
  SmartIF<IMessageSvc> m_MS;  
  
  //std::unordered_map<std::string,unsigned int> m_algname_index_map;
  //inline const unsigned int m_algname2index(const std::string& algoname) const ;
  
  std::vector<std::string> m_algname_vect;  
  inline const std::string& m_index2algname (unsigned int index) const;

  // Event slots management -------------------------------------------------
  int m_maxEventsInFlight;  
  struct EventSlot{
  public:
    EventSlot():eventContext(nullptr),complete(false){};
    void reset(EventContext* theeventContext){
      eventContext=theeventContext;
      complete=false;
    };
    EventContext* eventContext;
    bool complete;
    // DataFlowManager dataFlowMgr;
    // ControlFlowManager controlFlowMgr;
  };
  
  std::vector<EventSlot> m_eventSlots;  
  
  // States management ------------------------------------------------------
    
  unsigned int m_maxAlgosInFlight;
  std::atomic<unsigned int> m_algosInFlight;
 
  typedef unsigned int EventSlotIndex;
  typedef unsigned int AlgoSlotIndex;
  
  std::vector<AlgsExecutionStates> m_algsStates; 

  StatusCode m_updateStates();
  StatusCode m_promoteToControlFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToDataFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToMaxInFlightAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si); // here exit code for cflow to be handled
  
  typedef std::function<StatusCode(AlgoSlotIndex iAlgo, EventSlotIndex si)> TransitionMethod;
  std::map<std::string,TransitionMethod> m_statesTransitions;
  
  //const bool m_eventTerminated() const; 
  
  // Algos Management -------------------------------------------------------
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  // DP Super ugly, but will disappear when the deps are declared within the C++ code of the algos.
  typedef std::vector<std::vector<std::string>> algosDependenciesCollection;
  algosDependenciesCollection m_algosDependencies;
  
  // Actions management -----------------------------------------------------
  typedef std::function<StatusCode ()> action;
  tbb::concurrent_bounded_queue<action> m_actions_queue;       
  
  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;
  
};
#endif
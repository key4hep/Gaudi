#ifndef GAUDIKERNEL_SCHEDULERSVC_H
#define GAUDIKERNEL_SCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IRunable.h" 
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "DataFlowManager.h"

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

// External libs
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h" 

//---------------------------------------------------------------------------

  
class AlgsExecutionStates{
public:
  
  enum State : unsigned short {
    GROUND,
    CONTROLFLOWALLOWED,
    DATAFLOWALLOWED,
    MAXINFLIGHTALLOWED,
    SCHEDULED,
    EXECUTED,
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
      case EXECUTED:
        if (m_states[iAlgo]!=SCHEDULED){
          log << MSG::ERROR  << "Transition to FINISHED possible only from SCHEDULED state!";
          return StatusCode::FAILURE;   
        }
      //
      case FINISHED:
        if (m_states[iAlgo]!=EXECUTED){
          log << MSG::ERROR  << "Transition to FINISHED possible only from EXECUTED state!";
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
  bool algsPresent(State state) const{return std::find(m_states.begin(),m_states.end(),state)!=m_states.end();}
  bool allAlgsFinished(){
    int finishedAlgos=std::count_if(m_states.begin(),m_states.end(),[](State s) {return s == FINISHED;});
    return m_states.size() == (unsigned int)finishedAlgos;  };

  /* DP: For the scheduled2executed transition we use a concuirrent vector. 
   * We could also encapsulate in a transaction once gcc 47 will be supported.
   */    
  typedef tbb::concurrent_vector<State> states_vector;
  typedef states_vector::iterator vectIt;
  typedef states_vector::const_iterator const_vectIt;
  vectIt begin(){return m_states.begin();}; 
  vectIt end(){return m_states.end();};  
  const_vectIt begin() const{const_vectIt it = m_states.begin();return it;}; 
  const_vectIt end() const{const_vectIt it = m_states.begin();return it;}; 
  
private:
  states_vector m_states;
  SmartIF<IMessageSvc> m_MS;  
  
};

//---------------------------------------------------------------------------

class EventSlot{
public:
  EventSlot(const std::vector<std::vector<std::string>>& algoDependencies, 
            unsigned int thisSlotNumber,
            SmartIF<IMessageSvc> MS,
            SmartIF<IHiveWhiteBoard> WB):
              eventContext(nullptr),
              algsStates(algoDependencies.size(),MS),
              slotNumber(thisSlotNumber),
              complete(false),
              dataFlowMgr(WB,slotNumber,algoDependencies){};
              
  void reset(EventContext* theeventContext, unsigned int newSlotNumber){
    eventContext=theeventContext;
    algsStates.reset();
    dataFlowMgr.reset(newSlotNumber);
    complete=false;
  };
  
  // Members ----
  EventContext* eventContext;
  AlgsExecutionStates algsStates; 
  unsigned int slotNumber;
  bool complete;
  DataFlowManager dataFlowMgr;
  // ControlFlowManager controlFlowMgr;
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
class SchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  SchedulerSvc( const std::string& name, ISvcLocator* svc );
  
  /// Destructor
  ~SchedulerSvc();
  
  /// To start the scheduling in a different thread
  virtual StatusCode start ();
  
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
  
  std::vector<EventSlot> m_eventSlots;  
  
  // States management ------------------------------------------------------
    
  unsigned int m_maxAlgosInFlight;
  unsigned int m_algosInFlight;
 
  typedef unsigned int EventSlotIndex;
  typedef unsigned int AlgoSlotIndex;
  
  StatusCode m_updateStates();
  StatusCode m_promoteToControlFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToDataFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToMaxInFlightAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si);
  StatusCode m_promoteToExecuted(AlgoSlotIndex iAlgo, EventSlotIndex si, IAlgorithm* algo);
  StatusCode m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si);
  
  void m_isStalled(EventSlotIndex si);
  
  typedef std::function<StatusCode(AlgoSlotIndex iAlgo, EventSlotIndex si)> TransitionMethod;
  std::map<std::string,TransitionMethod> m_statesTransitions;
  
  //const bool m_eventTerminated() const; 
  
  // Algos Management -------------------------------------------------------
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  
  // DP Super ugly, but will disappear when the deps are declared within the C++ code of the algos.
  std::vector<std::vector<std::string>> m_algosDependencies;
  
  // Actions management -----------------------------------------------------
  typedef std::function<StatusCode ()> action;
  tbb::concurrent_bounded_queue<action> m_actions_queue;       
  
  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;
  
};
#endif
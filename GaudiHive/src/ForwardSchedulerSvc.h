#ifndef GAUDIHIVE_FORWARDSCHEDULERSVC_H
#define GAUDIHIVE_FORWARDSCHEDULERSVC_H

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
#include <thread>

// External libs
#include "tbb/concurrent_queue.h"


//---------------------------------------------------------------------------

/**
* Helper class to hold the states of the algorithms.s
*/
class AlgsExecutionStates{
public:

  /// Execution states of the algorithms
  enum State : unsigned short {
    INITIAL,
    CONTROLREADY,
    DATAREADY,
    SCHEDULED,
    EXECUTED,
    FINISHED,
    ERROR
    };

  AlgsExecutionStates(unsigned int algsNumber, SmartIF<IMessageSvc> MS):
    m_states(algsNumber,INITIAL),
    m_MS(MS){};    
  ~AlgsExecutionStates(){};
  StatusCode updateState(unsigned int iAlgo,State newState){
    MsgStream log(m_MS, "AlgExecutionStates");
    switch (newState) {
      case INITIAL:
        log << MSG::ERROR << "A transition to INITIAL is not defined.";
        return StatusCode::FAILURE;
      //
      case CONTROLREADY:
        if (m_states[iAlgo]!=INITIAL){
          log << MSG::ERROR << "Transition to CONTROLREADY possible only from INITIAL state!" << endmsg;
          return StatusCode::FAILURE;              
        } else {     
          m_states[iAlgo]=CONTROLREADY;
          return StatusCode::SUCCESS; 
        }
      //
      case DATAREADY:
        if (m_states[iAlgo]!=CONTROLREADY){
          log << MSG::ERROR  << "Transition to DATAREADY possible only from CONTROLREADY state!" << endmsg;
          return StatusCode::FAILURE;              
        } else {       
          m_states[iAlgo]=DATAREADY;
          return StatusCode::SUCCESS; 
        }
      case SCHEDULED:
        if (m_states[iAlgo]!=DATAREADY){
          log << MSG::ERROR  << "Transition to SCHEDULED possible only from DATAREADY state!" << endmsg;
          return StatusCode::FAILURE;              
        } else {
          m_states[iAlgo]=SCHEDULED;
          return StatusCode::SUCCESS; 
        }           
      //
      case EXECUTED:
        if (m_states[iAlgo]!=SCHEDULED){
          log << MSG::ERROR  << "Transition to EXECUTED possible only from SCHEDULED state!" << endmsg;
          return StatusCode::FAILURE;   
        } else {
          m_states[iAlgo]=EXECUTED;
          return StatusCode::SUCCESS; 
        } 
      //
      case FINISHED:
        if (m_states[iAlgo]!=EXECUTED){
          log << MSG::ERROR  << "Transition to FINISHED possible only from EXECUTED state!" << endmsg;
          return StatusCode::FAILURE;
        } else {
          m_states[iAlgo]=FINISHED;
          return StatusCode::SUCCESS; 
        }
      //
      default:
        log << MSG::ERROR  << "Undefined state!" << endmsg;
        return StatusCode::FAILURE;
    }
    return StatusCode::FAILURE;
    };

  void reset(){m_states.assign(m_states.size(),INITIAL);};
  bool algsPresent(State state) const{return std::find(m_states.begin(),m_states.end(),state)!=m_states.end();}
  bool allAlgsFinished(){
    int finishedAlgos=std::count_if(m_states.begin(),m_states.end(),[](State s) {return s == FINISHED;});
    return m_states.size() == (unsigned int)finishedAlgos;  };
  inline State algorithmState(unsigned int iAlgo){return iAlgo>=m_states.size()? ERROR : m_states[iAlgo];};

  typedef std::vector<State> states_vector;
  typedef states_vector::iterator vectIt;
  typedef states_vector::const_iterator const_vectIt;
  vectIt begin(){return m_states.begin();}; 
  vectIt end(){return m_states.end();};  
  const_vectIt begin() const{const_vectIt it = m_states.begin();return it;}; 
  const_vectIt end() const{const_vectIt it = m_states.end();return it;}; 

private:
  states_vector m_states;
  SmartIF<IMessageSvc> m_MS;
  
};

//---------------------------------------------------------------------------

class EventSlot{
public:
  EventSlot(const std::vector<std::vector<std::string>>& algoDependencies, 
            unsigned int numberOfAlgorithms,
            SmartIF<IMessageSvc> MS):
              eventContext(nullptr),
              algsStates(numberOfAlgorithms,MS),
              complete(false),
              dataFlowMgr(algoDependencies){};
    
  ~EventSlot(){};

  void reset(EventContext* theeventContext){
    eventContext=theeventContext;
    algsStates.reset();
    dataFlowMgr.reset();
    complete=false;
  };
  
  // Members ----
  EventContext* eventContext;
  AlgsExecutionStates algsStates; 
  bool complete;
  DataFlowManager dataFlowMgr;
  // ControlFlowManager controlFlowMgr;
};


//---------------------------------------------------------------------------

/**@class ForwardSchedulerSvc ForwardSchedulerSvc.h GaudiKernel/ForwardSchedulerSvc.h
 *
 *  The SchedulerSvc implements the IScheduler interface. It manages all the 
 *  execution states of the algorithms and interacts with the TBB runtime for 
 *  the algorithm tasks submission. A state machine takes care of the tracking 
 *  of the execution state of the algorithms.
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class ForwardSchedulerSvc: public extends1<Service, IScheduler> {
public:
  /// Constructor
  ForwardSchedulerSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~ForwardSchedulerSvc();

  /// Initialise
  virtual StatusCode initialize();

  /// Prepare the resources necessary for the scheduling and activate
  virtual StatusCode start();

  /// Deactivate the scheduler
  virtual StatusCode stop();
  
  /// Make an event available to the scheduler
  virtual StatusCode pushNewEvent(EventContext* eventContext);
  
  /// Blocks until an event is availble
  virtual StatusCode popFinishedEvent(EventContext*& eventContext);  

  /// Blocks until an event is availble
  virtual StatusCode tryPopFinishedEvent(EventContext*& eventContext);  

  /// Get free slots number
  virtual unsigned int freeSlots();


private:

  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  StatusCode m_activate();

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

  // Event slots management -------------------------------------------------
  int m_maxEventsInFlight;
  std::vector<EventSlot> m_eventSlots;  
  /// This is atomic to account for asyncronous updates by the scheduler wrt the rest
  std::atomic_uint m_freeSlots;
  tbb::concurrent_bounded_queue<EventContext*> m_finishedEvents;

  // States management ------------------------------------------------------

  unsigned int m_maxAlgosInFlight;
  unsigned int m_algosInFlight;

  typedef int EventSlotIndex;
  typedef unsigned int AlgoSlotIndex;

  /// Loop on algorithm in the slots and promote them to successive states
  StatusCode m_updateStates(EventSlotIndex si=-1 );
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

  // Actions management -----------------------------------------------------
  typedef std::function<StatusCode ()> action;
  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;  

  // Needed to queue actions on algorithm finishing and decrement algos in flight
  friend class AlgoExecutionTask;

};

#endif // GAUDIHIVE_FORWARDSCHEDULERSVC_H

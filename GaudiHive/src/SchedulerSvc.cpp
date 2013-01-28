// Framework includes
#include "SchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "AlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface

//===========================================================================
// Infrastructure methods

SchedulerSvc::SchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc) 
{
  declareProperty("maxEventsInFlight", m_maxEventsInFlight = 1 );
  declareProperty("maxAlgosInFlight", m_maxAlgosInFlight = 1 );
  // Will disappear when dependencies are properly propagated into the C++ code of the algos
  declareProperty("AlgosDependencies", m_algosDependencies);
  
  m_MS = svcLoc->service("IMessageSvc");
  
}

//---------------------------------------------------------------------------
SchedulerSvc::~SchedulerSvc(){}


//---------------------------------------------------------------------------

StatusCode SchedulerSvc::initialize(){

  // Get the list of algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();  
  
  // Set the states vectors, one per event slot
  const unsigned int algsNumber = algos.size();  
  
  // Prepare empty event slots
  SmartIF<IHiveWhiteBoard> WB(serviceLocator());
  StatusCode sc = WB->setNumberOfStores(m_maxEventsInFlight);
  if (sc !=StatusCode::SUCCESS)
    error() << "Could not properly set the number of slots in the WhiteBoard!";
  
  m_eventSlots.assign(m_maxEventsInFlight,EventSlot(m_algosDependencies,0,m_MS,WB));
  for (auto& slot: m_eventSlots)
    slot.complete=true; // to be able to insert new event
  
  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
    
  // Fill the containers to convert algo names to index
  m_algname_vect.reserve(algsNumber);
  unsigned int index=0;
  for (auto& algoPtr : algos){
    const std::string& name(algoPtr->name());
    //m_algname_index_map[name]=index;
    m_algname_vect.emplace_back(name);    
    index++;
  }
  
  // Fill a map of name / action using lambdas
  m_statesTransitions["Ground2ControlFlowAllowed"] = std::bind(&SchedulerSvc::m_promoteToControlFlowAllowed, 
                                                               this, 
                                                               std::placeholders::_1, 
                                                               std::placeholders::_2);
  m_statesTransitions["ControlFlowAllowed2DataFlowAllowed"] = std::bind(&SchedulerSvc::m_promoteToDataFlowAllowed, 
                                                                        this, 
                                                                        std::placeholders::_1, 
                                                                        std::placeholders::_2);
  m_statesTransitions["DataFlowAllowed2MaxInFlightAllowed"] = std::bind(&SchedulerSvc::m_promoteToMaxInFlightAllowed, 
                                                                        this, 
                                                                        std::placeholders::_1, 
                                                                        std::placeholders::_2);  
  m_statesTransitions["MaxInFlightAllowed2Scheduled"] = std::bind(&SchedulerSvc::m_promoteToScheduled, 
                                                                  this, 
                                                                  std::placeholders::_1, 
                                                                  std::placeholders::_2);
  
  // the task promotes the state to executed.  
  
  m_statesTransitions["Executed2Finished"] = std::bind(&SchedulerSvc::m_promoteToFinished, 
                                                                  this, 
                                                                  std::placeholders::_1, 
                                                                  std::placeholders::_2);  
  
  
  return StatusCode::SUCCESS;
  
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::start(){
 
  MsgStream log(m_MS);
  
  // Kick off the scheduling!
  m_updateStates();
  
  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;  
  StatusCode sc;
  while(true){
    m_actions_queue.pop(thisAction);
    sc = thisAction();
    if (sc!=StatusCode::SUCCESS)
      log << MSG::VERBOSE << "Action did not succed (which is not bad per se)." << endmsg;      
  }
  
  return StatusCode::SUCCESS;
}

//===========================================================================

//===========================================================================
// Utils and shortcuts 

inline const std::string& SchedulerSvc::m_index2algname (unsigned int index) const {
  return m_algname_vect[index];
}

//---------------------------------------------------------------------------

// inline const unsigned int SchedulerSvc::m_algname2index(const std::string& algoname) const {
//   return m_algname_index_map[algoname];
// }  

//===========================================================================
// EventSlot management

StatusCode SchedulerSvc::addEvent(EventContext* eventContext){
   
  // Look for the first finished event, if not return failure
  unsigned int newSlotNumber=0;
  for (auto& eventSlot: m_eventSlots){
    if (eventSlot.complete){
      eventSlot.reset(eventContext,newSlotNumber);
      return StatusCode::SUCCESS;
    }
    newSlotNumber++;
  }
    
  warning() << "Could not find a free processing slot.";
  return StatusCode::FAILURE;    
}

//---------------------------------------------------------------------------

unsigned int SchedulerSvc::eventsInFlight(){
  unsigned int n_eventsInFlight=0;
  for (auto& eventSlot: m_eventSlots)
    if (!eventSlot.complete)
      n_eventsInFlight++;
    
  return n_eventsInFlight;
}

//---------------------------------------------------------------------------

unsigned int SchedulerSvc::maxEventsInFlight(){
  return m_maxEventsInFlight;
}
  

//---------------------------------------------------------------------------
unsigned int SchedulerSvc::completedEvents(){
  unsigned int n_completedEvents=0;
  for (auto& eventSlot: m_eventSlots)
    if (eventSlot.complete)
      n_completedEvents++;
    
  return n_completedEvents;
}

//===========================================================================

//===========================================================================
// States Management

StatusCode SchedulerSvc::m_updateStates(){
 
  MsgStream log(m_MS);
  
  //unsigned int finishedEvents=0; needed when callback is ready
  StatusCode global_sc(StatusCode::SUCCESS);
  StatusCode partial_sc;
  for (EventSlotIndex iSlot=0;iSlot<m_eventSlots.size();iSlot++){
    for (AlgoSlotIndex iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){      
      for (auto& transition_Name_Action : m_statesTransitions){
        const std::string tr_name(transition_Name_Action.first);
        auto tr_action (transition_Name_Action.second);
        partial_sc = tr_action(iAlgo,iSlot);
        if (partial_sc != StatusCode::SUCCESS){
          log << MSG::INFO << "Could not apply transition " 
              << tr_name << " for algorithm " << m_index2algname(iAlgo)
              << " on processing slot " << iSlot << endmsg;
          global_sc=partial_sc;
        }
      } // end loop on transitions
    } // end loop on algos    
  
    // Now check if we are in a stall
    m_isStalled(iSlot);
    
    // Now check if we are done
    // DP needed for the callback to the evtprocessor
    //if (m_eventSlots[iSlot].algsStates.allAlgsFinished()) finishedEvents++;
  
  } // end loop on slots  
  
  // DP here put the calls to the evt processor: addEvent!
  
  return global_sc;
}

//---------------------------------------------------------------------------
/**
 * Checks if an event is in a stalled situation:
 *   - Event not complete
 *   - No algorithm is in flight
 *   - No algorithm is ready to be scheduled
 *   - No algorithm has finished
 *   
 */
void SchedulerSvc::m_isStalled(EventSlotIndex iSlot){
      
  // Get the slot
  const EventSlot& thisSlot = m_eventSlots[iSlot];
  
  // Get the states
  const AlgsExecutionStates& thisStates = thisSlot.algsStates;
  
  if (not thisSlot.complete and
      m_algosInFlight == 0 and
      not thisStates.algsPresent(AlgsExecutionStates::EXECUTED) and      
      not thisStates.algsPresent(AlgsExecutionStates::DATAFLOWALLOWED)){    
        
    std::string errorMessage("No algorithm is ready to run, "
                             "no algorithm is running, "
                             "no algorithm terminated, "
                             "event not complete: this is a stall.");
    fatal() << errorMessage << std::endl
            << "Algorithms that ran for event " << thisSlot.eventContext->m_evt_num << endmsg;
    unsigned int algoIndex=0;
    for (const auto& thisState : thisStates ){
      if (thisState == AlgsExecutionStates::EXECUTED or
          thisState == AlgsExecutionStates::FINISHED)
        fatal() << " o " << m_index2algname(algoIndex) << " could run";
      else
        fatal() << " o " << m_index2algname(algoIndex) << " could NOT run";
      algoIndex++;
    } // End ofloop on algos
    fatal() << endmsg;
    throw GaudiException (errorMessage,"HiveEventLoopMgr",StatusCode::FAILURE);      
  }
  
}

//---------------------------------------------------------------------------


StatusCode SchedulerSvc::m_promoteToControlFlowAllowed(AlgoSlotIndex /*iAlgo*/, EventSlotIndex /*si*/){
  
  // Do the control flow
  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToDataFlowAllowed(AlgoSlotIndex /*iAlgo*/, EventSlotIndex /*si*/){
  
  // Do the data flow
  return StatusCode::SUCCESS;  
  
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToMaxInFlightAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si){
  if ( m_algosInFlight < m_maxAlgosInFlight )
    return m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::MAXINFLIGHTALLOWED);
  return StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si){
  
  IAlgorithm* ialgoPtr=nullptr;
  StatusCode sc = m_algResourcePool->acquireAlgorithm(m_index2algname(iAlgo),ialgoPtr);
  
  if (sc == StatusCode::SUCCESS){
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?   
    algoPtr->setContext(m_eventSlots[si].eventContext);
    tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, this);    
    //TODO: dirty hack to make sequential root IO happy
    if (algoPtr->type() == "HiveReadAlgorithm") t->execute();
    else tbb::task::enqueue( *t);
    
    ++m_algosInFlight;
  }
  else{
    MsgStream log(m_MS);
    log << MSG::INFO << "Could not acquire instance for algorithm " 
        << m_index2algname(iAlgo) << " on slot " << si << endmsg;
  }
  
  return sc;
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToExecuted(AlgoSlotIndex iAlgo, EventSlotIndex si, IAlgorithm* algo){  
 
  // Put back the instance
  StatusCode sc = m_algResourcePool->releaseAlgorithm(algo->name(),algo);
  if (sc !=StatusCode::SUCCESS){
    EventContext* eventContext = dynamic_cast<Algorithm*>(algo)->getContext(); // DP: expose context getter in IAlgo?
    error() << "[Event " << eventContext->m_evt_num 
            << ", Slot " << eventContext->m_evt_slot  << "] "
            << "Instance of algorithm " << algo->name()
            << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
    }
  
  m_algosInFlight--;
  return m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::EXECUTED);
} 

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si){
  return m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::FINISHED);
} 

//===========================================================================






























































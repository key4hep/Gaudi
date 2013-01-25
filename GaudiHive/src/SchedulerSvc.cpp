// Framework includes
#include "SchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "AlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h"

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
  
  // Prepare empty event slots
  m_eventSlots.assign(m_maxEventsInFlight,EventSlot());
  
  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  
  // Get the list of algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  
  // Set the states vectors, one per event slot
  const unsigned int algsNumber = algos.size();  
  m_algsStates.assign(m_maxEventsInFlight,AlgsExecutionStates(algsNumber,m_MS));

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
  // the task promotes the state to finished.
  
  return StatusCode::SUCCESS;
  
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::run(){
 
  action thisAction;
  while(true){
    m_actions_queue.pop(thisAction);
    thisAction();
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
  for (auto& eventSlot: m_eventSlots){
    if (eventSlot.complete){
      eventSlot.reset(eventContext);
      return StatusCode::SUCCESS;
    }    
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
 
  StatusCode global_sc(StatusCode::SUCCESS);
  StatusCode partial_sc;
  for (EventSlotIndex iSlot=0;iSlot<m_eventSlots.size();iSlot++){
    for (AlgoSlotIndex iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){      
      for (auto& transition_Name_Action : m_statesTransitions){
        const std::string tr_name(transition_Name_Action.first);
        auto tr_action (transition_Name_Action.second);
        partial_sc = tr_action(iAlgo,iSlot);
        if (partial_sc != StatusCode::SUCCESS){
          error() << "Error occurred during the transition " 
                  << tr_name << " for algorithm " << m_algname_vect[iAlgo]
                  << " on processing slot " << iSlot;
          global_sc=partial_sc;
        }
      } // end loop on actions
    } // end loop on algos    
  } // end loop on slots
  
  return global_sc;
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToControlFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si){
  
  // Do the control flow
  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToDataFlowAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si){
  
  // Do the data flow
  return StatusCode::SUCCESS;  
  
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToMaxInFlightAllowed(AlgoSlotIndex iAlgo, EventSlotIndex si){
  if ( m_algosInFlight < m_maxAlgosInFlight )
    return m_algsStates[si].updateState(iAlgo,AlgsExecutionStates::MAXINFLIGHTALLOWED);
  return StatusCode::FAILURE;
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si){
  
  IAlgorithm* ialgoPtr=nullptr;
  StatusCode sc = m_algResourcePool->acquireAlgorithm(m_index2algname(iAlgo),ialgoPtr);
  
  if (sc == StatusCode::SUCCESS){
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr);    
    algoPtr->setContext(m_eventSlots[si].eventContext);
    tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, this);    
    //TODO: dirty hack to make sequential root IO happy
    if (algoPtr->type() == "HiveReadAlgorithm") t->execute();
    else tbb::task::enqueue( *t);
    
    ++m_algosInFlight;
  }
  
  return sc;
}

//---------------------------------------------------------------------------

StatusCode SchedulerSvc::m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si){
  return m_algsStates[si].updateState(iAlgo,AlgsExecutionStates::FINISHED);
} 
  
//===========================================================================






























































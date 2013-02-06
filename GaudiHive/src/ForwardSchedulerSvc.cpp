// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "tbb/task.h"

// C++
#include <unordered_set>
#include <algorithm>
#include <map>

// Local 
#include "ForwardSchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "AlgResourcePool.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(ForwardSchedulerSvc);

//===========================================================================
// Infrastructure methods

ForwardSchedulerSvc::ForwardSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_isActive(false)
{
  declareProperty("MaxEventsInFlight", m_maxEventsInFlight = 1 );
  declareProperty("MaxAlgosInFlight", m_maxAlgosInFlight = 1 );
  // Will disappear when dependencies are properly propagated into the C++ code of the algos
  declareProperty("AlgosDependencies", m_algosDependencies);    
}

//---------------------------------------------------------------------------
ForwardSchedulerSvc::~ForwardSchedulerSvc(){}
//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::initialize(){

  // Initialise mother class (read properties, ...)  
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess())
    warning () << "Base class could not be initialized" << endmsg;  
  
  // Set the number of slots
  m_freeSlots=m_maxEventsInFlight;
  
  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  if (!m_algResourcePool.isValid())
    error() << "Error retrieving AlgoResourcePool interface IAlgoResourcePool." << endmsg;
  
  return StatusCode::SUCCESS;
  
}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::start(){
  
  // Start the base class
  StatusCode sc(Service::start());
  if (!sc.isSuccess())
    warning () << "Base class could not be started" << endmsg;  

  // Get the list of algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();  
    
  // Set the states vectors, one per event slot
  
  // For the size, we have to take the number of unique algorithms, to avoid
  // double counting of clones
  std::unordered_set<std::string> algosNamesSet;
  std::vector<std::string> algoNamesVector; // an ordered container..
  for_each(algos.begin(), 
           algos.end(), 
           [&algosNamesSet,&algoNamesVector] (IAlgorithm* algo) {
             auto ret = algosNamesSet.insert(algo->name());
             if (ret.second) algoNamesVector.push_back(algo->name());});
  const unsigned int algsNumber = algosNamesSet.size();  

  info() << "Found " <<  algsNumber << " algorithms" << endmsg;

  // Prepare empty event slots
  m_whiteboard = serviceLocator()->service("EventDataSvc");
  if (!m_whiteboard.isValid())
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;

  info() << "Setting the whiteboard slots to " << m_maxEventsInFlight << endmsg;
  sc = m_whiteboard->setNumberOfStores(m_maxEventsInFlight);
  if (!sc.isSuccess())
    warning() << "Number of slots in the WhiteBoard could not be properly set!" << endmsg;

  info() << "Algodependecies size is " << m_algosDependencies.size() << endmsg;

  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc (serviceLocator());
  if (!messageSvc.isValid())
    error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;

  m_eventSlots.assign(m_maxEventsInFlight,EventSlot(m_algosDependencies,algsNumber,messageSvc));
  for (auto& slot: m_eventSlots)
    slot.complete=true; // to be able to insert new eventContext
   
  // Fill the containers to convert algo names to index
  m_algname_vect.reserve(algsNumber);
  unsigned int index=0;
  for (const auto& name : algoNamesVector){
    m_algname_index_map[name]=index;
    m_algname_vect.emplace_back(name);    
    index++;
  }

   // Activate the scheduler 
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread (std::bind(&ForwardSchedulerSvc::m_activate,
                                    this));  
  return sc;
  
}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::stop(){

  StatusCode sc(Service::stop());
  if (!sc.isSuccess())
    warning () << "Base class could not be stopped" << endmsg;
  
  sc = m_deactivate();
  if (!sc.isSuccess())
    warning () << "Scheduler could not be deactivated" << endmsg;
    
  info() << "Joining Scheduler thread" << endmsg;
  m_thread.join();  
  
  return sc;
  
}
      
//---------------------------------------------------------------------------  
/**
 * Activate the scheduler. From this moment on the queue of actions is 
 * checked. The checking will stop when the m_isActive flag is false and the 
 * queue is not empty. This will guarantee that all actions are executed and 
 * a stall is not created.
 */
StatusCode ForwardSchedulerSvc::m_activate(){  
  
  // Now it's running
  m_isActive=true;
  
  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;  
  StatusCode sc;
  
  // Continue to wait if the scheduler is running or there is something to do
  info() << "Start checking the actionsQueue" << endmsg;
  while(m_isActive or m_actionsQueue.size()!=0){
    m_actionsQueue.pop(thisAction);
    sc = thisAction();
    if (sc!=StatusCode::SUCCESS)
      verbose() << "Action did not succed (which is not bad per se)." << endmsg;
    else
      verbose() << "Action succed." << endmsg;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler untill all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode ForwardSchedulerSvc::m_deactivate(){
  
  // Drain the scheduler
  m_actionsQueue.push(std::bind(&ForwardSchedulerSvc::m_drain,
                                this));
  
  // This would be the last action
  m_actionsQueue.push([this]() -> StatusCode {m_isActive=false;return StatusCode::SUCCESS;});
  
  return StatusCode::SUCCESS;
}

//===========================================================================

//===========================================================================
// Utils and shortcuts 

inline const std::string& ForwardSchedulerSvc::m_index2algname (unsigned int index) {
  return m_algname_vect[index];
}

//---------------------------------------------------------------------------

inline unsigned int ForwardSchedulerSvc::m_algname2index(const std::string& algoname) {
  unsigned int index = m_algname_index_map[algoname];
  return index;
}  

//===========================================================================
// EventSlot management
/**
 * Add event to the scheduler. There are two cases possible:
 *  1) No slot is free. A StatusCode::FAILURE is returned.
 *  2) At least one slot is free. An action which resets the slot and kicks 
 * off its update is queued.
 */
StatusCode ForwardSchedulerSvc::pushNewEvent(EventContext* eventContext){
  
  if (!eventContext){
    fatal() << "Event context is nullptr" << endmsg;
    return StatusCode::FAILURE;
    }  
  
  if (m_freeSlots == 0){
    info() << "A free processing slot could not be found." << endmsg;
    return StatusCode::FAILURE;   
    }

  m_freeSlots--;

  auto action = [this,eventContext] () -> StatusCode {
    // Look for the first finished event, if not return failure
    unsigned int newProcessingSlotNumber=0;
    for (auto& eventSlot: this->m_eventSlots){
        if (eventSlot.complete){
          eventSlot.complete=false;
          info() << "A free processing slot was found." << endmsg;
          eventSlot.reset(eventContext);
          return this->m_updateStates(newProcessingSlotNumber);
        }
        newProcessingSlotNumber++;
    }
    return StatusCode::SUCCESS; // it should never arrive here
  }; // end of lambda

  // Kick off the scheduling!
  m_actionsQueue.push(action);

  // It should never reach this point...
  return StatusCode::SUCCESS;;
}
//---------------------------------------------------------------------------

unsigned int ForwardSchedulerSvc::freeSlots(){
  return m_freeSlots;
  }

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_drain(){
  
  unsigned int slotNum=0;
  for (auto& thisSlot : m_eventSlots){
    if (not thisSlot.algsStates.allAlgsFinished()){
      m_updateStates(slotNum);
    }
    slotNum++;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Get a finished event or block until one becomes available.
*/
StatusCode ForwardSchedulerSvc::popFinishedEvent(EventContext*& eventContext){
  m_finishedEvents.pop(eventContext);
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just fail
*/
StatusCode ForwardSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext){
  return m_finishedEvents.try_pop(eventContext);
}


//===========================================================================

//===========================================================================
// States Management

StatusCode ForwardSchedulerSvc::m_updateStates(EventSlotIndex si){

  // Fill a map of initial state / action using closures.
  // done to update the states w/o several if/elses
  // Posterchild for constexpr with gcc4.7 onwards!
  const std::map<AlgsExecutionStates::State, std::function<StatusCode(AlgoSlotIndex iAlgo, EventSlotIndex si)>> 
   statesTransitions = {
  {AlgsExecutionStates::INITIAL, std::bind(&ForwardSchedulerSvc::m_promoteToControlReady,
                                  this,
                                  std::placeholders::_1,
                                  std::placeholders::_2)},
  {AlgsExecutionStates::CONTROLREADY, std::bind(&ForwardSchedulerSvc::m_promoteToDataReady,
                                  this,
                                  std::placeholders::_1,  
                                  std::placeholders::_2)},
  {AlgsExecutionStates::DATAREADY, std::bind(&ForwardSchedulerSvc::m_promoteToScheduled,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2)}
  };

  info() << "Updating states." << endmsg;

  StatusCode global_sc(StatusCode::SUCCESS);
  StatusCode partial_sc;

  // Loop on the event slots
  // DP Maybe starting from the first slot may be bad for the backlog
  // Sort by event number
//   const unsigned int slotsNumber=m_eventSlots.size();
//   std::vector<std::pair<unsigned int, unsigned int>> slotIndex_evtNum_pairs(slotsNumber);
//   for (EventSlotIndex iSlot=0;iSlot< (int)slotsNumber; iSlot++){
//     unsigned int evtNum=-1;
//     if (m_eventSlots[iSlot].eventContext!=nullptr)
//       evtNum = m_eventSlots[iSlot].eventContext->m_evt_num;
//     slotIndex_evtNum_pairs[iSlot]=std::pair<unsigned int, unsigned int>(iSlot,evtNum);
//   }
// 
//   std::sort(slotIndex_evtNum_pairs.begin(),
//                  slotIndex_evtNum_pairs.end(),
//                  [](std::pair<unsigned int, unsigned int>a, std::pair<unsigned int, unsigned int>b){return a.first<b.first;});

   for (EventSlotIndex iSlot=0;iSlot< (int)m_eventSlots.size();iSlot++){
//  for (auto& slotIndex_evtNum_pair : slotIndex_evtNum_pairs){
//    EventSlotIndex iSlot = slotIndex_evtNum_pair.first;
    // If the slot is not what we want or if the event is finished let's skip the iteration
    if ( (si != iSlot && si >= 0 ) || m_eventSlots[iSlot].complete)  continue;

    debug() << "Found slot " << iSlot << "(event "<<m_eventSlots[iSlot].eventContext->m_evt_num << ")" << endmsg;

    // Cache the states of the algos to improve readability and performance
    AlgsExecutionStates& thisAlgsStates = m_eventSlots[iSlot].algsStates;

    for (unsigned int iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){
      const AlgsExecutionStates::State& algState =thisAlgsStates.algorithmState(iAlgo);
      if (algState==AlgsExecutionStates::ERROR)
        error() << " Algo " << m_index2algname(iAlgo) << " is in ERROR state." << endmsg;      
      // Loop on state transitions from the one suited to algo state up to the one for SCHEDULED.
      partial_sc=StatusCode::SUCCESS;
      for (auto state_transition = statesTransitions.find(algState);
           state_transition!=statesTransitions.end() && partial_sc.isSuccess();
           state_transition++){
        partial_sc = state_transition->second(iAlgo,iSlot);
        debug() << m_index2algname(iAlgo) << " state is " << algState << endmsg;
        if (!partial_sc.isSuccess()){
          debug() << "Could not apply transition from " 
              << thisAlgsStates.algorithmState(iAlgo) << " for algorithm " << m_index2algname(iAlgo)
              << " on processing slot " << iSlot << endmsg;
          global_sc=partial_sc;
          }
        } // end loop on transitions
    } // end loop on algos

    // Check if we are in a stall
    m_isStalled(iSlot);

    // Now it's possible to promote safely the EXECUTED algos to FINISHED
    for (AlgoSlotIndex iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo)
      if (thisAlgsStates.algorithmState(iAlgo)==AlgsExecutionStates::EXECUTED){
        StatusCode finished(m_promoteToFinished(iAlgo,iSlot));
        if (!finished.isSuccess())
          warning() << "Algorithm could not be transitioned to FINISHED" << endmsg;
      }

    // Check if the event is finished
    if (thisAlgsStates.allAlgsFinished()){
      EventSlot & thisSlot = m_eventSlots[iSlot];
      thisSlot.complete=true;
      m_finishedEvents.push(thisSlot.eventContext);
      m_freeSlots++;
      debug() << "Event " << thisSlot.eventContext->m_evt_num << " finished." << endmsg;
    }

  } // end loop on slots    

  info() << "States Updated." << endmsg;

  return global_sc;
}

//---------------------------------------------------------------------------
/**
 * Checks if an event is in a stalled situation:
 *   - Event not complete: first requirement for a stall.
 *   - No algorithm is in flight: an algorithm in flight could potentially 
 * change the destiny of the processing of the event for example writing some 
 * data object or could become available for running.
 *   - No algorithm is ready to be scheduled:
 *   - No algorithm has executed: there might be no algorithm in flight but 
 * if something is in the EXECUTED state, some products may have been written 
 * on the transient store. A further update is needed. The difference between 
 * EXECUTED and FINISHED is exploited here. Indeed it would be impossible to 
 * distinguish which algorithms acquired the FINISHED state since the last 
 * update.
 */
StatusCode ForwardSchedulerSvc::m_isStalled(EventSlotIndex iSlot){
  //DP the condition needs to be made more robust
  // Get the slot
  const EventSlot& thisSlot = m_eventSlots[iSlot];
  
  // Get the states
  const AlgsExecutionStates& thisStates = thisSlot.algsStates;
  
  if (not thisSlot.complete and
      m_algosInFlight == 0 and
      not thisStates.algsPresent(AlgsExecutionStates::DATAREADY) and 
      not thisStates.algsPresent(AlgsExecutionStates::SCHEDULED) and
      not thisStates.algsPresent(AlgsExecutionStates::EXECUTED)){
    const std::string errorMessage("No algorithm is ready to run, "
                            "no algorithm is running, "
                            "no algorithm terminated, "
                            "event not complete: this is a stall.");
    error() << errorMessage << endmsg;
    error() << "Algorithms states for event " << thisSlot.eventContext->m_evt_num << endmsg;
    unsigned int algoIndex=0;
    for (const auto& thisState : thisStates ){
      if (thisState == AlgsExecutionStates::EXECUTED or
          thisState == AlgsExecutionStates::FINISHED)
        error() << " o " << m_index2algname(algoIndex) << " could run"<< endmsg;
      else
        error() << " o " << m_index2algname(algoIndex) << " could NOT run"<< endmsg;
      algoIndex++;
    } // End of loop on algos
    throw GaudiException (errorMessage,"HiveEventLoopMgr",StatusCode::FAILURE);
  }
  return StatusCode::FAILURE;
  
}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_promoteToControlReady(AlgoSlotIndex iAlgo, EventSlotIndex si){

  // Do the control flow
  StatusCode sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::CONTROLREADY);
  if (sc.isSuccess())
    debug() << "Promoting " << m_index2algname(iAlgo) << " to CONTROLREADY" << endmsg;

  return sc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_promoteToDataReady(AlgoSlotIndex iAlgo, EventSlotIndex si){

  StatusCode sc (m_eventSlots[si].dataFlowMgr.canAlgorithmRun(iAlgo));
  StatusCode updateSc(StatusCode::FAILURE);
  if (sc == StatusCode::SUCCESS)
    updateSc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::DATAREADY);

  if (updateSc.isSuccess())
    debug() << "Promoting " << m_index2algname(iAlgo) << " to DATAREADY" << endmsg;

  return updateSc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_promoteToScheduled(AlgoSlotIndex iAlgo, EventSlotIndex si){
  
  if (m_algosInFlight == m_maxAlgosInFlight)
    return StatusCode::FAILURE;

  const std::string& algName(m_index2algname(iAlgo));
  
  IAlgorithm* ialgoPtr=nullptr;
  StatusCode sc = m_algResourcePool->acquireAlgorithm(algName,ialgoPtr);
  
  if (sc == StatusCode::SUCCESS){
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?   
    EventContext* eventContext = m_eventSlots[si].eventContext;
    if (!eventContext)
      fatal() << "Event context for algorithm " << algName 
              << " is a nullptr (slot " << si<< ")" << endmsg;

    algoPtr->setContext(m_eventSlots[si].eventContext);
    tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, this);
    tbb::task::enqueue( *t);
    ++m_algosInFlight;
    info() << "Algorithm " << algName << " was submitted. Algorithms scheduled are "
           << m_algosInFlight << endmsg;

    StatusCode updateSc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::SCHEDULED);
    if (updateSc.isSuccess())
      debug() << "Promoting " << m_index2algname(iAlgo) << " to SCHEDULED" << endmsg;
    return updateSc;
  }
  else{
    info() << "Could not acquire instance for algorithm " 
        << m_index2algname(iAlgo) << " on slot " << si << endmsg;
    return sc;
  }

}

//---------------------------------------------------------------------------
/// This method is called only from within the AlgoExecutionTask.
StatusCode ForwardSchedulerSvc::m_promoteToExecuted(AlgoSlotIndex iAlgo, EventSlotIndex si, IAlgorithm* algo){  

  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>(algo); // DP: expose context getter in IAlgo?
  if (!castedAlgo)
    fatal() << "The casting did not succed!" << endmsg;
  EventContext* eventContext = castedAlgo->getContext();
  StatusCode sc = m_algResourcePool->releaseAlgorithm(algo->name(),algo);

  if (!sc.isSuccess()){
    error() << "[Event " << eventContext->m_evt_num 
            << ", Slot " << eventContext->m_evt_slot  << "] "
            << "Instance of algorithm " << algo->name()
            << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
    }

  m_algosInFlight--;  

  // Update the catalog: some new products may be there
  m_whiteboard->selectStore(eventContext->m_evt_slot).ignore();

  // update prods in the dataflow
  std::vector<std::string> new_products;
  m_whiteboard->getNewDataObjects(new_products).ignore();
  m_eventSlots[si].dataFlowMgr.updateDataObjectsCatalog(new_products);

  // update controlflow
  // .......

  info() << "Algorithm " << algo->name() << " executed. Algorithms in flight are "
      << m_algosInFlight << endmsg;

   // Schedule an update of the stati of the algorithms
   auto updateAction = std::bind(&ForwardSchedulerSvc::m_updateStates,
                                 this,
                                 -1);
   m_actionsQueue.push(updateAction);

  sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::EXECUTED);
  if (sc.isSuccess())
    debug() << "Promoting " << m_index2algname(iAlgo) << " to EXECUTED" << endmsg;
  return sc;
} 

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_promoteToFinished(AlgoSlotIndex iAlgo, EventSlotIndex si){

  StatusCode sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::FINISHED);
  if (sc.isSuccess())
    debug() << "Promoting " << m_index2algname(iAlgo) << " on slot " << si << " to FINISHED" << endmsg;
  return sc;
} 


//===========================================================================

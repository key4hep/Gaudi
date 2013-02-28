// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "tbb/task.h"

// C++
#include <unordered_set>
#include <algorithm>
#include <map>
#include <sstream>

// Local 
#include "ForwardSchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "AlgResourcePool.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(ForwardSchedulerSvc)

std::map<ForwardSchedulerSvc::AlgsExecutionStates::State,std::string> ForwardSchedulerSvc::AlgsExecutionStates::stateNames= {
    {INITIAL,"INITIAL"},
    {CONTROLREADY,"CONTROLREADY"},
    {DATAREADY,"DATAREADY"},
    {SCHEDULED,"SCHEDULED"},
    {EXECUTED,"EXECUTED"},
    {ERROR,"ERROR"}
    };

//===========================================================================
// Infrastructure methods

ForwardSchedulerSvc::ForwardSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_isActive(false)
{
  declareProperty("MaxEventsInFlight", m_maxEventsInFlight = 1 );
  declareProperty("MaxAlgosInFlight", m_maxAlgosInFlight = 1 );
  declareProperty("WhiteboardSvc", m_whiteboardSvcName = "EventDataSvc" );
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

  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
  if (!m_algResourcePool.isValid())
    error() << "Error retrieving AlgoResourcePool" << endmsg;  
  
  // Get Whiteboard
  m_whiteboard = serviceLocator()->service(m_whiteboardSvcName);
  if (!m_whiteboard.isValid())
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;  
  
  // Set the number of slots
  m_freeSlots=m_maxEventsInFlight; 
  
  // Get the list of algorithms
  const std::list<IAlgorithm*>& algos = m_algResourcePool->getFlatAlgList();  
  const unsigned int algsNumber = algos.size(); 
  info() << "Found " <<  algsNumber << " algorithms" << endmsg;

  // Prepare empty event slots
  info() << "Setting the whiteboard slots to " << m_maxEventsInFlight << endmsg;
  sc = m_whiteboard->setNumberOfStores(m_maxEventsInFlight);
  if (!sc.isSuccess())
    warning() << "Number of stores in the WhiteBoard could not be properly set!" << endmsg;

  const unsigned int algosDependenciesSize=m_algosDependencies.size();
  info() << "Algodependecies size is " << algosDependenciesSize << endmsg;

  // If no dependencies given, just assume none are required
  if (algosDependenciesSize == 0){
    auto beginIt = m_algosDependencies.begin();
    std::vector<std::string> emptyDeps(0);
    m_algosDependencies.insert (beginIt,algsNumber,emptyDeps);
  }
  
  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc (serviceLocator());
  if (!messageSvc.isValid())
    error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;
    
  m_eventSlots.assign(m_maxEventsInFlight,EventSlot(m_algosDependencies,algsNumber,messageSvc));
  std::for_each(m_eventSlots.begin(),m_eventSlots.end(),[](EventSlot& slot){slot.complete=true;});
  //for (auto& slot: m_eventSlots) slot.complete=true; // to be able to insert new eventContext
   
  // Fill the containers to convert algo names to index
  m_algname_vect.reserve(algsNumber);
  unsigned int index=0;
  for (IAlgorithm* algo : algos){
    const std::string& name = algo->name();
    m_algname_index_map[name]=index;
    m_algname_vect.emplace_back(name);    
    index++;
  }  

  // Activate the scheduler 
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread (std::bind(&ForwardSchedulerSvc::m_activate,
                                    this));  
  return StatusCode::SUCCESS;
  
}
//---------------------------------------------------------------------------  

StatusCode ForwardSchedulerSvc::finalize(){

  StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
    warning () << "Base class could not be finalized" << endmsg;   
  
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
void ForwardSchedulerSvc::m_activate(){  
  
  // Now it's running
  m_isActive=true;
  
  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;  
  StatusCode sc(StatusCode::SUCCESS);
  
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
}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler untill all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode ForwardSchedulerSvc::m_deactivate(){
  
  if (m_isActive){
    // Drain the scheduler
    m_actionsQueue.push(std::bind(&ForwardSchedulerSvc::m_drain,
                                  this));    
    // This would be the last action
    m_actionsQueue.push([this]() -> StatusCode {m_isActive=false;return StatusCode::SUCCESS;});
  }
  
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
    // Event processing slot forced to be the same as the wb slot
    const unsigned int thisSlotNum = eventContext->m_evt_slot;
    EventSlot& thisSlot = m_eventSlots[thisSlotNum];
    if (!thisSlot.complete)
      fatal() << "The slot " << thisSlotNum << " is supposed to be a finished event but it's not" << endmsg;
    info() << "A free processing slot was found." << endmsg;
    thisSlot.reset(eventContext);
    return this->m_updateStates(thisSlotNum);
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
    if (not thisSlot.algsStates.allAlgsExecuted()){
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
  m_freeSlots++;
  debug() << "Popped slot " << eventContext->m_evt_slot << "(event " 
          << eventContext->m_evt_num << ")" << endmsg;
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just fail
*/
StatusCode ForwardSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext){
  if (m_finishedEvents.try_pop(eventContext)){
    debug() << "Try Pop successful slot " << eventContext->m_evt_slot 
            << "(event " << eventContext->m_evt_num << ")" << endmsg;
    m_freeSlots++;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//---------------------------------------------------------------------------
/// Stops scheduling and returns the failure
StatusCode ForwardSchedulerSvc::m_eventFailed(EventContext* eventContext){
  
  fatal() << "Event " << eventContext->m_evt_num << " on slot " 
          << eventContext->m_evt_slot << " failed" << endmsg;

  // Set the number of slots available to 0
  m_freeSlots=0;          
          
  // Empty queue and deactivate the service
  action thisAction;
  while(m_actionsQueue.try_pop(thisAction)){};
  m_deactivate();
  
  // Push into the finished events queue the failed context
  EventContext* thisEvtContext;
  while(m_finishedEvents.try_pop(thisEvtContext)){};
  m_finishedEvents.push(eventContext);
  
  return StatusCode::SUCCESS;
  

  
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

  StatusCode global_sc(StatusCode::FAILURE);
  StatusCode partial_sc;

   // Sort from the oldest to the newest event
   // Prepare a vector of pointers to the slots to avoid copies
   std::vector<EventSlot*> eventSlotsPtrs;

   // Consider all slots if si <0 or just one otherwhise
   if (si<0){
   const int eventsSlotsSize(m_eventSlots.size());
   eventSlotsPtrs.reserve(eventsSlotsSize);
    for (auto slotIt=m_eventSlots.begin();slotIt!=m_eventSlots.end();slotIt++){
      if (!slotIt->complete)
        eventSlotsPtrs.push_back(&(*slotIt));
      }
    std::sort(eventSlotsPtrs.begin(),
              eventSlotsPtrs.end(), 
              [](EventSlot* a, EventSlot* b){return a->eventContext->m_evt_num < b->eventContext->m_evt_num;});
    } else{
      eventSlotsPtrs.push_back(&m_eventSlots[si]);
    }

  for (EventSlot* thisSlotPtr : eventSlotsPtrs){
    EventSlotIndex iSlot = thisSlotPtr->eventContext->m_evt_slot;

    // Cache the states of the algos to improve readability and performance
    auto& thisSlot = m_eventSlots[iSlot];
    AlgsExecutionStates& thisAlgsStates = thisSlot.algsStates;

    for (unsigned int iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){
      const AlgsExecutionStates::State& algState = thisAlgsStates.algorithmState(iAlgo);
      if (algState==AlgsExecutionStates::ERROR)
        error() << " Algo " << m_index2algname(iAlgo) << " is in ERROR state." << endmsg;
      // Loop on state transitions from the one suited to algo state up to the one for SCHEDULED.
      partial_sc=StatusCode::SUCCESS;
      for (auto state_transition = statesTransitions.find(algState);
           state_transition!=statesTransitions.end() && partial_sc.isSuccess();
           state_transition++){
        partial_sc = state_transition->second(iAlgo,iSlot);
        if (partial_sc.isFailure()){
          debug() << "Could not apply transition from " 
              << thisAlgsStates.algorithmState(iAlgo) << " for algorithm " << m_index2algname(iAlgo)
              << " on processing slot " << iSlot << endmsg;
          }
        else{global_sc=partial_sc;}
        } // end loop on transitions
    } // end loop on algos

    // Not complete because this would mean that the slot is already free!
    if (!thisSlot.complete && thisSlot.algsStates.allAlgsExecuted()){
      thisSlot.complete=true;
      m_finishedEvents.push(thisSlot.eventContext);
      debug() << "Event " << thisSlot.eventContext->m_evt_num 
             << " finished (slot "<< thisSlot.eventContext->m_evt_slot 
             << ")." << endmsg;
      thisSlot.eventContext= nullptr;
    } else{
      m_isStalled(iSlot).ignore();
    }

  } // end loop on slots    

  info() << "States Updated." << endmsg;  

  return global_sc;
}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::m_isStalled(EventSlotIndex iSlot){
  // Get the slot
  EventSlot& thisSlot = m_eventSlots[iSlot];

  if (m_actionsQueue.empty() &&
      m_algosInFlight == 0 &&
      !thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY)){

    info() << "About to declare a stall"<< endmsg;

    std::stringstream errorMsg;
    errorMsg << "Stall detected!\n";
    errorMsg << "Algorithms states for event " << thisSlot.eventContext->m_evt_num << std::endl;
    unsigned int algoIndex=0;
    for (const AlgsExecutionStates::State& thisState : thisSlot.algsStates ){
        errorMsg << " o " << m_index2algname(algoIndex) 
                 << " was in state " << AlgsExecutionStates::stateNames[thisState] 
                 << ". Its data dependencies are ";
        auto deps (thisSlot.dataFlowMgr.dataDependencies(algoIndex));
        char separator=',';
        const int depsSize=deps.size();
        for (int i=0;i<depsSize;++i){
          if (i==depsSize-1)
            separator = ' ';
          errorMsg << deps[i] << separator;
           }
        errorMsg << std::endl;
        algoIndex++;
    }
    
    // Snapshot of the WhiteBoard
    errorMsg << "The content of the whiteboard for this event was:" << std::endl;
    const auto& wbSlotContent ( thisSlot.dataFlowMgr.content() );
    for (auto product : wbSlotContent ){
        errorMsg << " o " << product << std::endl;
    }

    throw GaudiException (errorMsg.str(),
                          "ForwardSchedulerSvc",
                          StatusCode::FAILURE);

    return StatusCode::FAILURE;
  }

return StatusCode::SUCCESS;

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
  StatusCode sc ( m_algResourcePool->acquireAlgorithm(algName,ialgoPtr) );
  
  if (sc.isSuccess()){
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?   
    EventContext* eventContext ( m_eventSlots[si].eventContext );
    if (!eventContext)
      fatal() << "Event context for algorithm " << algName 
              << " is a nullptr (slot " << si<< ")" << endmsg;

    algoPtr->setContext(m_eventSlots[si].eventContext);
    tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, serviceLocator(), this);
    tbb::task::enqueue( *t);
    ++m_algosInFlight;
    debug() << "Algorithm " << algName << " was submitted. Algorithms scheduled are "
           << m_algosInFlight << endmsg;

    StatusCode updateSc ( m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::SCHEDULED) );
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
/// The call to this method is triggered only from within the AlgoExecutionTask.
StatusCode ForwardSchedulerSvc::m_promoteToExecuted(AlgoSlotIndex iAlgo, EventSlotIndex si, IAlgorithm* algo){  

  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>(algo); // DP: expose context getter in IAlgo?
  if (!castedAlgo)
    fatal() << "The casting did not succed!" << endmsg;
  EventContext* eventContext = castedAlgo->getContext();

  // Checkif the execution failed
  if (eventContext->m_evt_failed)
    m_eventFailed(eventContext);
  
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

  debug() << "Algorithm " << algo->name() << " executed. Algorithms in flight are "
      << m_algosInFlight << endmsg;

   // Schedule an update of the stati of the algorithms
   auto updateAction = std::bind(&ForwardSchedulerSvc::m_updateStates,
                                 this,
                                 -1);
   m_actionsQueue.push(updateAction);

  debug() << "Trying to promote " << m_index2algname(iAlgo) << " to EXECUTED" << endmsg;
  sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::EXECUTED);
  if (sc.isSuccess())
    debug() << "Promoting " << m_index2algname(iAlgo) << " on slot " << si << " to EXECUTED" << endmsg;
  return sc;
}

//===========================================================================




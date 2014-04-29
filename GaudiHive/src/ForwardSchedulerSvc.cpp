// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include <GaudiAlg/GaudiAlgorithm.h>
#include <GaudiKernel/IDataManagerSvc.h>
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

// External libs
// DP waiting for the TBB service
#include "tbb/task_scheduler_init.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(ForwardSchedulerSvc)

//===========================================================================
// Infrastructure methods

ForwardSchedulerSvc::ForwardSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_isActive(false),
 m_algosInFlight(0),
 m_updateNeeded(true)
{
  declareProperty("MaxEventsInFlight", m_maxEventsInFlight = 0 );
  declareProperty("ThreadPoolSize", m_threadPoolSize = -1 );
  declareProperty("WhiteboardSvc", m_whiteboardSvcName = "EventDataSvc" );
  // Will disappear when dependencies are properly propagated into the C++ code of the algos
  declareProperty("AlgosDependencies", m_algosDependencies);
  declareProperty("MaxAlgosInFlight", m_maxAlgosInFlight = 0, "Taken from the whiteboard. Deprecated" );
  // XXX: CF tests. Temporary property to switch between ControlFlow implementations
  declareProperty("useGraphFlowManagement", m_CFNext = false );
  declareProperty("DataFlowManagerNext", m_DFNext = false );
}

//---------------------------------------------------------------------------
ForwardSchedulerSvc::~ForwardSchedulerSvc(){}
//---------------------------------------------------------------------------

/**
 * Here, among some "bureaucracy" operations, the scheduler is activated,
 * executing the activate() function in a new thread.
 * In addition the algorithms list is acquired from the algResourcePool.
**/
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

  // Check the MaxEventsInFlight parameters and react
  // Deprecated for the moment
  size_t numberOfWBSlots = m_whiteboard->getNumberOfStores();
  if (m_maxEventsInFlight!=0){
    warning() << "Property MaxEventsInFlight was set. This works but it's deprecated. "
              << "Please migrate your code options files." << endmsg;

    if (m_maxEventsInFlight != (int)numberOfWBSlots){
      warning() << "In addition, the number of events in flight ("
                << m_maxEventsInFlight << ") differs from the slots in the whiteboard ("
                << numberOfWBSlots << "). Setting the number of events in flight to "
                << numberOfWBSlots << endmsg;
    }
  }

  // Align the two quantities
  m_maxEventsInFlight = numberOfWBSlots;

  // Set the number of free slots
  m_freeSlots=m_maxEventsInFlight;

  // Get the list of algorithms
  const std::list<IAlgorithm*>& algos = m_algResourcePool->getFlatAlgList();
  const unsigned int algsNumber = algos.size();
  info() << "Found " <<  algsNumber << " algorithms" << endmsg;

  const unsigned int algosDependenciesSize=m_algosDependencies.size();
  info() << "Algodependecies size is " << algosDependenciesSize << endmsg;

  /* Dependencies
   0) Read deps from config file
   1) Look for handles in algo, if none
   2) Assume none are required
  */
  if (algosDependenciesSize == 0){
    for (IAlgorithm* ialgoPtr : algos){
      Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr);
      if (nullptr == algoPtr){
         fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;
      }

      const std::vector<MinimalDataObjectHandle*>& algoHandles(algoPtr->handles());
      std::vector<std::string> algoDependencies;
      if (!algoHandles.empty()){

        info() << "Algorithm " << algoPtr->name() << " data dependencies:" << endmsg;
        for (MinimalDataObjectHandle* handlePtr : algoHandles ){
          if (handlePtr->accessType() == MinimalDataObjectHandle::AccessType::READ && handlePtr->isValid()){
            const std::string& productName = handlePtr->dataProductName();
            info() << "  o READ Handle found for product " << productName << endmsg;
            algoDependencies.emplace_back(productName);
          }
        }
      } else {
        info() << "Algorithm " << algoPtr->name() << " has no data dependencies." << endmsg;
      }

      m_algosDependencies.emplace_back(algoDependencies);
    }
  }

  // Fill the containers to convert algo names to index
  m_algname_vect.reserve(algsNumber);
  unsigned int index=0;
  for (IAlgorithm* algo : algos){
    const std::string& name = algo->name();
    m_algname_index_map[name]=index;
    m_algname_vect.emplace_back(name);
    index++;
  }

  // prepare the control flow part
  if (m_CFNext) m_DFNext = true; //force usage of new data flow machinery when new control flow is used
  const AlgResourcePool* algPool = dynamic_cast<const AlgResourcePool*>(m_algResourcePool.get());
  sc = m_cfManager.initialize(algPool->getControlFlowGraph(), m_algname_index_map, m_eventSlots);
  unsigned int controlFlowNodeNumber = m_cfManager.getControlFlowGraph()->getControlFlowNodeCounter();
  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc (serviceLocator());
  if (!messageSvc.isValid())
    error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;

  m_eventSlots.assign(m_maxEventsInFlight,EventSlot(m_algosDependencies,algsNumber,controlFlowNodeNumber,messageSvc));
  std::for_each(m_eventSlots.begin(),m_eventSlots.end(),[](EventSlot& slot){slot.complete=true;});

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events in flight: " << m_maxEventsInFlight << endmsg;
  info() << " o Number of algorithms in flight: " << m_maxAlgosInFlight << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  // Simulating execution flow by analyzing the graph topology and logic only
  m_cfManager.simulateExecutionFlow();

  // Activate the scheduler in another thread.
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread (std::bind(&ForwardSchedulerSvc::activate,
                                    this));

  return sc;

}
//---------------------------------------------------------------------------

/**
 * Here the scheduler is deactivated and the thread joined.
**/
StatusCode ForwardSchedulerSvc::finalize(){

  StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
    warning () << "Base class could not be finalized" << endmsg;

  sc = deactivate();
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
 * The TBB pool must be initialised in the thread from where the tasks are
 * launched (http://threadingbuildingblocks.org/docs/doxygen/a00342.html)
 * The scheduler is initialised here since this method runs in a separate
 * thread and spawns the tasks (through the execution of the lambdas)
 **/
void ForwardSchedulerSvc::activate(){

  // Now it's running
  m_isActive=true;

  /// Activate a pool
  tbb::task_scheduler_init* TBBSchedInit = nullptr;

  // -100 prevents the creation of the pool and the scheduler directly executes
  // the tasks.
  if (-100 != m_threadPoolSize){
    debug() << "Initialising a TBB thread pool of size " << m_threadPoolSize << endmsg;
    // Leave -1 in case selected, increment otherwise
    int thePoolSize=m_threadPoolSize;
    if (thePoolSize!=-1)
      thePoolSize+=1;
    TBBSchedInit = new tbb::task_scheduler_init (thePoolSize);
    }
  else{
    debug() << "Thread pool size is one. Pool not initialised." << endmsg;
    }
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
      verbose() << "Action succeded." << endmsg;
  }

  if (TBBSchedInit)
    delete TBBSchedInit;
}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler until all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode ForwardSchedulerSvc::deactivate(){

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

inline const std::string& ForwardSchedulerSvc::index2algname (unsigned int index) {
  return m_algname_vect[index];
}

//---------------------------------------------------------------------------

inline unsigned int ForwardSchedulerSvc::algname2index(const std::string& algoname) {
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

  if (m_freeSlots.load() == 0){
    debug() << "A free processing slot could not be found." << endmsg;
    return StatusCode::FAILURE;
  }

  //no problem as push new event is only called from one thread (event loop manager)
  m_freeSlots--;

  auto action = [this,eventContext] () -> StatusCode {
    // Event processing slot forced to be the same as the wb slot
    const unsigned int thisSlotNum = eventContext->m_evt_slot;
    EventSlot& thisSlot = m_eventSlots[thisSlotNum];
    if (!thisSlot.complete)
      fatal() << "The slot " << thisSlotNum << " is supposed to be a finished event but it's not" << endmsg;
    info() << "A free processing slot was found." << endmsg;
    thisSlot.reset(eventContext);
    // XXX: CF tests
    if (m_CFNext) m_cfManager.promoteToControlReadyState(thisSlot.algsStates,thisSlot.controlFlowState,thisSlotNum);
    return this->updateStates(thisSlotNum);
  }; // end of lambda

  // Kick off the scheduling!
  verbose() << "Pushing the action to update the scheduler for slot " <<  eventContext->m_evt_slot << endmsg;
  verbose() << "Free slots available " <<  m_freeSlots.load() << endmsg;
  m_actionsQueue.push(action);

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode ForwardSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){
  StatusCode sc;
  for (auto context : eventContexts){
    sc = pushNewEvent(context);
    if (sc != StatusCode::SUCCESS) return sc;
  }
  return sc;
}

//---------------------------------------------------------------------------
unsigned int ForwardSchedulerSvc::freeSlots(){
  return std::max(m_freeSlots.load(),0);
}

//---------------------------------------------------------------------------
/**
 * Update the states for all slots until nothing is left to do.
*/
StatusCode ForwardSchedulerSvc::m_drain(){

  unsigned int slotNum=0;
  for (auto& thisSlot : m_eventSlots){
    if (not thisSlot.algsStates.allAlgsExecuted() and not thisSlot.complete){
      updateStates(slotNum);
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
  if (m_freeSlots.load() == m_maxEventsInFlight or
      !m_isActive) {
    return StatusCode::FAILURE;
  } else {
    m_finishedEvents.pop(eventContext);
    m_freeSlots++;
    debug() << "Popped slot " << eventContext->m_evt_slot << "(event "
            << eventContext->m_evt_num << ")" << endmsg;
    return StatusCode::SUCCESS;
  }
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just return a failure
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
/**
 * It can be possible that an event fails. In this case this method is called.
 * It dumps the state of the scheduler, drains the actions (without executing
 * them) and events in the queues and returns a failure.
*/
StatusCode ForwardSchedulerSvc::eventFailed(EventContext* eventContext){

  // Set the number of slots available to an error code
  m_freeSlots.store(0);

  fatal() << "*** Event " << eventContext->m_evt_num << " on slot "
          << eventContext->m_evt_slot << " failed! ***" << endmsg;

  //dumpSchedulerState(-1);

  // Empty queue and deactivate the service
  action thisAction;
  while(m_actionsQueue.try_pop(thisAction)){};
  deactivate();

  // Push into the finished events queue the failed context
  EventContext* thisEvtContext;
  while(m_finishedEvents.try_pop(thisEvtContext)) { m_finishedEvents.push(thisEvtContext); };
  m_finishedEvents.push(eventContext);

  return StatusCode::FAILURE;

}

//===========================================================================

//===========================================================================
// States Management

/**
 * Update the state of the algorithms.
 * The oldest events are checked before the newest, in order to reduce the
 * event backlog.
 * To check if the event is finished the algorithm checks if:
 * * No algorithms have been signed off by the control flow
 * * No algorithms have been signed off by the data flow
 * * No algorithms have been scheduled
*/
StatusCode ForwardSchedulerSvc::updateStates(int si, const std::string& algo_name){



  m_updateNeeded=true;

  // Fill a map of initial state / action using closures.
  // done to update the states w/o several if/elses
  // Posterchild for constexpr with gcc4.7 onwards!
  /*const std::map<AlgsExecutionStates::State, std::function<StatusCode(unsigned int iAlgo, int si)>>
   statesTransitions = {
  {AlgsExecutionStates::CONTROLREADY, std::bind(&ForwardSchedulerSvc::promoteToDataReady,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2)},
  {AlgsExecutionStates::DATAREADY, std::bind(&ForwardSchedulerSvc::promoteToScheduled,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2)}
  };*/

  StatusCode global_sc(StatusCode::FAILURE);
  StatusCode partial_sc;

   // Sort from the oldest to the newest event
   // Prepare a vector of pointers to the slots to avoid copies
   std::vector<EventSlot*> eventSlotsPtrs;

   // Consider all slots if si <0 or just one otherwise
   if (si<0) {
     const int eventsSlotsSize(m_eventSlots.size());
     eventSlotsPtrs.reserve(eventsSlotsSize);
     for (auto slotIt=m_eventSlots.begin();slotIt!=m_eventSlots.end();slotIt++) {
       if (!slotIt->complete)
         eventSlotsPtrs.push_back(&(*slotIt));
     }
     std::sort(eventSlotsPtrs.begin(),
               eventSlotsPtrs.end(),
               [](EventSlot* a, EventSlot* b) {return a->eventContext->m_evt_num < b->eventContext->m_evt_num;});
   } else {
     eventSlotsPtrs.push_back(&m_eventSlots[si]);
   }

  for (EventSlot* thisSlotPtr : eventSlotsPtrs) {
    int iSlot = thisSlotPtr->eventContext->m_evt_slot;

    // Cache the states of the algos to improve readability and performance
    auto& thisSlot = m_eventSlots[iSlot];
    AlgsExecutionStates& thisAlgsStates = thisSlot.algsStates;

    // Take care of the control ready update
    // XXX: CF tests
    if (!m_CFNext) {
      m_cfManager.updateEventState(thisAlgsStates,thisSlot.controlFlowState);
    } else {
      if (!algo_name.empty())
        m_cfManager.updateDecision(algo_name,iSlot,thisAlgsStates,thisSlot.controlFlowState);
    }


    //DF note: all this this is a loop over all algs and applies CR->DR and DR->SCHD transistions
    /*for (unsigned int iAlgo=0;iAlgo<m_algname_vect.size();++iAlgo){
    	const AlgsExecutionStates::State& algState = thisAlgsStates[iAlgo];
    	if (algState==AlgsExecutionStates::ERROR)
    		error() << " Algo " << index2algname(iAlgo) << " is in ERROR state." << endmsg;
    	// Loop on state transitions from the one suited to algo state up to the one for SCHEDULED.
    	partial_sc=StatusCode::SUCCESS;
    	for (auto state_transition = statesTransitions.find(algState);
    			state_transition!=statesTransitions.end() && partial_sc.isSuccess();
    			state_transition++){
    		partial_sc = state_transition->second(iAlgo,iSlot);
    		if (partial_sc.isFailure()){
    			debug() << "Could not apply transition from "
    					<< AlgsExecutionStates::stateNames[thisAlgsStates[iAlgo]]
    					                                   << " for algorithm " << index2algname(iAlgo)
    					                                   << " on processing slot " << iSlot << endmsg;
    		}
    		else{global_sc=partial_sc;}
    	} // end loop on transitions
    }*/ // end loop on algos


    StatusCode partial_sc;
    //first update CONTROLREADY to DATAREADY
    if (!m_CFNext) {
      for(auto it = thisAlgsStates.begin(AlgsExecutionStates::State::CONTROLREADY);
          it != thisAlgsStates.end(AlgsExecutionStates::State::CONTROLREADY); ++it) {

        uint algIndex = *it;
        partial_sc = promoteToDataReady(algIndex, iSlot);
        if (partial_sc.isFailure()) {
          debug() << "Could not apply transition from "
                  << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::CONTROLREADY]
                  << " for algorithm " << index2algname(algIndex) << " on processing slot " << iSlot << endmsg;
        }
      }
    }

    //now update DATAREADY to SCHEDULED
    for(auto it = thisAlgsStates.begin(AlgsExecutionStates::State::DATAREADY);
        it != thisAlgsStates.end(AlgsExecutionStates::State::DATAREADY); ++it) {

      uint algIndex = *it;
      partial_sc = promoteToScheduled(algIndex, iSlot);
      if (partial_sc.isFailure()) {
        debug() << "Could not apply transition from "
                << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY]
                << " for algorithm " << index2algname(algIndex) << " on processing slot " << iSlot << endmsg;
    	}
    }

    // Not complete because this would mean that the slot is already free!
    if (!thisSlot.complete &&
        m_cfManager.rootDecisionResolved(thisSlot.controlFlowState) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::CONTROLREADY) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY) &&
        !thisSlot.algsStates.algsPresent(AlgsExecutionStates::SCHEDULED)) {

      thisSlot.complete=true;
      // if the event did not fail, add it to the finished events
      // otherwise it is taken care of in the error handling already
      if (!thisSlot.eventContext->m_evt_failed) {
        m_finishedEvents.push(thisSlot.eventContext);
        debug() << "Event " << thisSlot.eventContext->m_evt_num << " finished (slot "
                << thisSlot.eventContext->m_evt_slot << ")." << endmsg;
      }
      // now let's return the fully evaluated result of the control flow
      std::stringstream ss;
      m_cfManager.printEventState(ss, thisSlot.algsStates, thisSlot.controlFlowState,0);
      debug() << ss.str() << endmsg;
      thisSlot.eventContext= nullptr;
    } else {
      StatusCode eventStalledSC = isStalled(iSlot);
      if (! eventStalledSC.isSuccess())
        eventFailed(thisSlot.eventContext);
    }
  } // end loop on slots

  verbose() << "States Updated." << endmsg;

  return global_sc;
}

//---------------------------------------------------------------------------

/**
 * Check if we are in present of a stall condition for a particular slot.
 * This is the case when no actions are present in the actionsQueue,
 * no algorithm is in flight and no algorithm has all of its dependencies
 * satisfied.
*/
StatusCode ForwardSchedulerSvc::isStalled(int iSlot) {
  // Get the slot
  EventSlot& thisSlot = m_eventSlots[iSlot];

  if (m_actionsQueue.empty() &&
      m_algosInFlight == 0 &&
      (!thisSlot.algsStates.algsPresent(AlgsExecutionStates::DATAREADY))) {

    info() << "About to declare a stall" << endmsg;
    fatal() << "*** Stall detected! ***\n" << endmsg;
    dumpSchedulerState(iSlot);
    //throw GaudiException ("Stall detected",name(),StatusCode::FAILURE);

    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

/**
 * Used for debugging purposes, the state of the scheduler is dumped on screen
 * in order to be inspected.
 * The dependencies of each algo are printed and the missing ones specified.
**/
void ForwardSchedulerSvc::dumpSchedulerState(int iSlot) {

  // To have just one big message
  std::stringstream outputMessageStream;

  int slotCount = -1;
  for (auto thisSlot : m_eventSlots){
    slotCount++;
    outputMessageStream.str(std::string());
    if ( thisSlot.complete )
      continue;

    outputMessageStream << "Dump of Scheduler State for slot " << thisSlot.eventContext->m_evt_num << std::endl;

    if ( 0 > iSlot or iSlot == slotCount) {
    	outputMessageStream << "Algorithms states for event " << thisSlot.eventContext->m_evt_num << std::endl;

      const std::vector<std::string>& wbSlotContent ( thisSlot.dataFlowMgr.content() );
      for (unsigned int algoIdx=0; algoIdx < thisSlot.algsStates.size(); ++algoIdx ) {
        outputMessageStream << " o " << index2algname(algoIdx)
                            << " was in state " << AlgsExecutionStates::stateNames[thisSlot.algsStates[algoIdx]]
                            << ". Its data dependencies are ";
        std::vector<std::string> deps (thisSlot.dataFlowMgr.dataDependencies(algoIdx));
        const int depsSize=deps.size();
        if (depsSize==0)
          outputMessageStream << " none.";

        for (int i=0;i<depsSize;++i)
          outputMessageStream << deps[i] << (i==(depsSize-1) ? "" :", ");

        // Now list what dependencies were available
        // With std::algorithms, move the ones which are missing at the beginning of the vector
        std::vector<std::string>::iterator missinngDepsEndIt =
                        std::remove_if(deps.begin(), // from the beginning of the deps
                                       deps.end(),   // to their end
                                       [&wbSlotContent] (std::string dep) { // remove if this lambda returns true
          return std::count(wbSlotContent.begin(),wbSlotContent.end(),dep)!=0; //look for dep in wb content
        });

        if (deps.begin() != missinngDepsEndIt) {
          outputMessageStream << ". The following are missing: ";
          for (std::vector<std::string>::iterator missingDep=deps.begin();missingDep!=missinngDepsEndIt;++missingDep)
            outputMessageStream << *missingDep << (missingDep==(missinngDepsEndIt-1)?"":", ");
        }

        outputMessageStream << std::endl;
      }

      fatal() << outputMessageStream.str() << endmsg;
      outputMessageStream.str(std::string());

      // Snapshot of the WhiteBoard
      outputMessageStream << "The content of the whiteboard for this event was:\n";
      for (auto& product : wbSlotContent )
        outputMessageStream << " o " << product << std::endl;

      fatal() << outputMessageStream.str()<< endmsg;
      outputMessageStream.str(std::string());

      // Snapshot of the ControlFlow
      outputMessageStream << "The status of the control flow for this event was:\n";
      std::stringstream cFlowStateStringStream;
      m_cfManager.printEventState(cFlowStateStringStream, thisSlot.algsStates, thisSlot.controlFlowState,0);

      outputMessageStream << cFlowStateStringStream.str();

      fatal() <<  outputMessageStream.str() << endmsg;
    }
  }

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToControlReady(unsigned int iAlgo, int si) {

  // Do the control flow
  StatusCode sc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::CONTROLREADY);
  if (sc.isSuccess())
    debug() << "Promoting " << index2algname(iAlgo) << " to CONTROLREADY" << endmsg;

  return sc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToDataReady(unsigned int iAlgo, int si) {

  StatusCode sc;
  if (!m_DFNext) {
    sc = m_eventSlots[si].dataFlowMgr.canAlgorithmRun(iAlgo);
  } else {
    sc = m_cfManager.algoDataDependenciesSatisfied(index2algname(iAlgo),si);
  }

  StatusCode updateSc(StatusCode::FAILURE);
  if (sc == StatusCode::SUCCESS)
    updateSc = m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::DATAREADY);

  if (updateSc.isSuccess())
    debug() << "Promoting " << index2algname(iAlgo) << " to DATAREADY" << endmsg;

  return updateSc;

}

//---------------------------------------------------------------------------

StatusCode ForwardSchedulerSvc::promoteToScheduled(unsigned int iAlgo, int si) {

  if (m_algosInFlight == m_maxAlgosInFlight)
    return StatusCode::FAILURE;

  const std::string& algName(index2algname(iAlgo));

  IAlgorithm* ialgoPtr=nullptr;
  StatusCode sc ( m_algResourcePool->acquireAlgorithm(algName,ialgoPtr) );

  if (sc.isSuccess()) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?
    EventContext* eventContext ( m_eventSlots[si].eventContext );
    if (!eventContext)
      fatal() << "Event context for algorithm " << algName << " is a nullptr (slot " << si<< ")" << endmsg;

    algoPtr->setContext(m_eventSlots[si].eventContext);
    ++m_algosInFlight;
    // Avoid to use tbb if the pool size is 1 and run in this thread
    if (-100 != m_threadPoolSize) {
      tbb::task* t = new( tbb::task::allocate_root() ) AlgoExecutionTask(ialgoPtr, iAlgo, serviceLocator(), this);
      tbb::task::enqueue( *t);
    } else {
      AlgoExecutionTask theTask(ialgoPtr, iAlgo, serviceLocator(), this);
      theTask.execute();
    }

    debug() << "Algorithm " << algName << " was submitted on event " << eventContext->m_evt_num
            << ". Algorithms scheduled are " << m_algosInFlight << endmsg;

    StatusCode updateSc ( m_eventSlots[si].algsStates.updateState(iAlgo,AlgsExecutionStates::SCHEDULED) );
    if (updateSc.isSuccess())
      debug() << "Promoting " << index2algname(iAlgo) << " to SCHEDULED" << endmsg;
    return updateSc;
  } else {
    debug() << "Could not acquire instance for algorithm " << index2algname(iAlgo) << " on slot " << si << endmsg;
    return sc;
  }

}

//---------------------------------------------------------------------------
/**
 * The call to this method is triggered only from within the AlgoExecutionTask.
*/
StatusCode ForwardSchedulerSvc::promoteToExecuted(unsigned int iAlgo, int si, IAlgorithm* algo) {

  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>(algo); // DP: expose context getter in IAlgo?
  if (!castedAlgo)
    fatal() << "The casting did not succeed!" << endmsg;
  EventContext* eventContext = castedAlgo->getContext();

  // Check if the execution failed
  if (eventContext->m_evt_failed)
    eventFailed(eventContext);

  StatusCode sc = m_algResourcePool->releaseAlgorithm(algo->name(),algo);

  if (!sc.isSuccess()) {
    error() << "[Event " << eventContext->m_evt_num << ", Slot " << eventContext->m_evt_slot  << "] "
            << "Instance of algorithm " << algo->name() << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
    }

  m_algosInFlight--;

  EventSlot& thisSlot = m_eventSlots[si];
  // XXX: CF tests
  if (!m_DFNext) {
    // Update the catalog: some new products may be there
    m_whiteboard->selectStore(eventContext->m_evt_slot).ignore();

    // update prods in the dataflow
    // DP: Handles could be used. Just update what the algo wrote
    std::vector<std::string> new_products;
    m_whiteboard->getNewDataObjects(new_products).ignore();
    for (const auto& new_product : new_products)
      debug() << "Found in WB: " << new_product << endmsg;
    thisSlot.dataFlowMgr.updateDataObjectsCatalog(new_products);
  }

  debug() << "Algorithm " << algo->name() << " executed. Algorithms scheduled are " << m_algosInFlight << endmsg;

  // Limit number of updates
  if (m_CFNext) m_updateNeeded = true; // XXX: CF tests: with the new CF traversal the if clause below has to be removed
  if (m_updateNeeded) {
    // Schedule an update of the status of the algorithms
    auto updateAction = std::bind(&ForwardSchedulerSvc::updateStates, this, -1, algo->name());
    m_actionsQueue.push(updateAction);
    m_updateNeeded = false;
  }

  debug() << "Trying to handle execution result of " << index2algname(iAlgo) << "." << endmsg;
  State state;
  if (algo->filterPassed()) {
    state = State::EVTACCEPTED;
  } else {
    state = State::EVTREJECTED;
  }

  sc = thisSlot.algsStates.updateState(iAlgo,state);

  if (sc.isSuccess())
    debug() << "Promoting " << index2algname(iAlgo) << " on slot " << si << " to "
            << AlgsExecutionStates::stateNames[state] << endmsg;

  return sc;
}

//===========================================================================

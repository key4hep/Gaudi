// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/IDataManagerSvc.h"

// C++
#include <list>
#include <thread>

// Local 
#include "ParallelSequentialSchedulerSvc.h"
#include "AlgResourcePool.h"
#include "RetCodeGuard.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(ParallelSequentialSchedulerSvc)

//===========================================================================
// Infrastructure methods

ParallelSequentialSchedulerSvc::ParallelSequentialSchedulerSvc(
		const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_eventContext(nullptr){

	// Will disappear when dependencies are properly propagated into the C++ code of the algos
	declareProperty("AlgosDependencies", m_algosDependencies);
	declareProperty("UseTopAlgList", m_useTopAlgList=false);
	declareProperty("ThreadPoolSize", m_threadPoolSize = -1 );
	declareProperty("WhiteboardSvc", m_whiteboardSvcName = "EventDataSvc" );

}

//---------------------------------------------------------------------------
ParallelSequentialSchedulerSvc::~ParallelSequentialSchedulerSvc(){}
//---------------------------------------------------------------------------

StatusCode ParallelSequentialSchedulerSvc::initialize(){

  // Initialise mother class (read properties, ...)  
	StatusCode sc(Service::initialize());
	if (!sc.isSuccess())
		warning () << "Base class could not be initialized" << endmsg;

	// Get the algo resource pool
	m_algResourcePool =  serviceLocator()->service("AlgResourcePool");
	if (!m_algResourcePool.isValid()){
		error() << "Error retrieving AlgResourcePool" << endmsg;
		return StatusCode::FAILURE;
	}

	// Get the list of algorithms
	m_algList = m_useTopAlgList ? m_algResourcePool->getTopAlgList() : m_algResourcePool->getFlatAlgList();
	info() << "Found " <<  m_algList.size() << " algorithms" << endmsg;

	// Get Whiteboard
	m_whiteboard = serviceLocator()->service(m_whiteboardSvcName);
	if (!m_whiteboard.isValid())
		fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;

	// Check the MaxEventsInFlight parameters and react
	// Deprecated for the moment
	size_t numberOfWBSlots = m_whiteboard->getNumberOfStores();

	// Set the number of free slots
	m_freeSlots=numberOfWBSlots;

	info() << "Allowing " << m_freeSlots << " events in flight" << endmsg;

	if(m_threadPoolSize == -1)
		m_threadPoolSize = numberOfWBSlots;

	debug() << "Initialising a TBB thread pool of size " << m_threadPoolSize << endmsg;
	m_tbb_sched = new tbb::task_scheduler_init (m_threadPoolSize);

	// Fill the containers to convert algo names to index
	m_algname_index_map.reserve(m_algList.size());
	m_algname_vect.reserve(m_algList.size());
	unsigned int index=0;
	for (IAlgorithm* algo : m_algList){
		const std::string& name = algo->name();
		m_algname_index_map[name]=index;
		m_algname_vect.emplace_back(name);
		index++;
	}

	//initialize control flow manager
	const AlgResourcePool* algPool = dynamic_cast<const AlgResourcePool*>(m_algResourcePool.get());

	m_controlFlow.initialize(algPool->getControlFlow(), m_algname_index_map);

	const unsigned int algosDependenciesSize=m_algosDependencies.size();
	info() << "Algodependecies size is " << algosDependenciesSize << endmsg;

	//get algorithm dependencies
	/* Dependencies
	   0) Read deps from config file
	   1) Look for handles in algo, if none
	   2) Assume none are required
	 */
	if (algosDependenciesSize == 0){
		// Get the event root from the IDataManagerSvc interface of the WhiteBoard
		SmartIF<IDataManagerSvc> dataMgrSvc (m_whiteboard);
		std::string rootInTESName(dataMgrSvc->rootName());
		if ("" != rootInTESName && '/'!=rootInTESName[rootInTESName.size()-1]){
			rootInTESName = rootInTESName+"/";
		}

		for (IAlgorithm* ialgoPtr : m_algList){
			Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr);
			if (nullptr == algoPtr){
				fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;
			}

			const std::vector<MinimalDataObjectHandle*>& algoHandles(algoPtr->handles());
			std::vector<std::string> algoDependencies;
			if (!algoHandles.empty()){

				info() << "Algorithm " << algoPtr->name() << " data dependencies:" << endmsg;
				for (MinimalDataObjectHandle* handlePtr : algoHandles ){
					if (handlePtr->accessType() == IDataObjectHandle::AccessType::READ){
						const std::string& productName = rootInTESName + handlePtr->dataProductName();
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

	return StatusCode::SUCCESS;
  
}
//---------------------------------------------------------------------------  

StatusCode ParallelSequentialSchedulerSvc::finalize(){

	delete m_tbb_sched;

	StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
    warning () << "Base class could not be finalized" << endmsg;     
  return sc;  
}

/** Make an event available to the scheduler. Immediately the algortihms are 
 * executed.
 */
StatusCode ParallelSequentialSchedulerSvc::pushNewEvent(EventContext* eventContext){
  std::vector<EventContext*> eventContexts;
  eventContexts.push_back(eventContext);
  eventContext->m_evt_failed = false;
  return pushNewEvents(eventContexts);
}

StatusCode ParallelSequentialSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){

	for(auto evt : eventContexts){
		if(m_freeSlots.load() > 0){
			//only one thread executes scheduler --> m_freeSlots can only grow if other thread finishes
			m_freeSlots--;

			debug() << "Enqueuing event " << evt->m_evt_num << " @ " << evt->m_evt_slot << endmsg;

			tbb::task* t = new( tbb::task::allocate_root() )
					SequentialTask(serviceLocator(), evt, this, m_algResourcePool);
			tbb::task::enqueue( *t);
		} else {
			return StatusCode::FAILURE;
		}
	}

	return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------
/**
* Get a finished event or block until one becomes available.
*/
StatusCode ParallelSequentialSchedulerSvc::popFinishedEvent(EventContext*& eventContext){

	m_finishedEvents.pop(eventContext);
    debug() << "Popped slot " << eventContext->m_evt_slot << "(event "
            << eventContext->m_evt_num << ")" << endmsg;
    m_freeSlots++;
    return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just return a failure
*/
StatusCode ParallelSequentialSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext){
  if (m_finishedEvents.try_pop(eventContext)){
    debug() << "Try Pop successful slot " << eventContext->m_evt_slot
            << "(event " << eventContext->m_evt_num << ")" << endmsg;
    m_freeSlots++;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}
  
//---------------------------------------------------------------------------  

/** Get free slots number. Given that the scheduler is sequential and its 
 * methods non reentrant, this is always 1.
 */
unsigned int ParallelSequentialSchedulerSvc::freeSlots(){return m_freeSlots;}

//---------------------------------------------------------------------------

tbb::task* SequentialTask::execute() {

	// Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
	const SmartIF<IProperty> appmgr(m_serviceLocator);
	SmartIF<IMessageSvc> messageSvc (m_serviceLocator);
	MsgStream log(messageSvc, "SequentialAlgoExecutionTask");
	log.activate();

	StatusCode sc;

	//initialize control algorithm states and decisions
	AlgsExecutionStates algStates(m_scheduler->m_algList.size(), messageSvc);
	const AlgResourcePool* algPool = dynamic_cast<const AlgResourcePool*>(m_scheduler->m_algResourcePool.get());
	std::vector<int> nodeDecisions(algPool->getControlFlowNodeCounter(), -1);

	m_scheduler->m_controlFlow.updateEventState(algStates.m_states, nodeDecisions);
	m_scheduler->m_controlFlow.promoteToControlReadyState(algStates.m_states, nodeDecisions);

	//initialize data flow manager
	//DataFlowManager dataFlow(m_scheduler->m_algosDependencies);

	//intitialize context
	m_eventContext->m_thread_id = pthread_self();
	bool eventFailed = false;

	// loop while algorithms are controlFlowReady and event has not failed
	while(!eventFailed && algStates.algsPresent(AlgsExecutionStates::State::CONTROLREADY) ){

		//std::cout << "[" << m_eventContext->m_evt_num << "] algorithms left" << std::endl;

		std::for_each(m_scheduler->m_algList.begin(), m_scheduler->m_algList.end(),

				[&] (IAlgorithm* ialgorithm) {

					uint algIndex = m_scheduler->m_algname_index_map[ialgorithm->name()];

					if(AlgsExecutionStates::State::CONTROLREADY == algStates.algorithmState(algIndex)){
							//&& dataFlow.canAlgorithmRun(algIndex)){

						//promote algorithm to data ready
						algStates.updateState(algIndex,AlgsExecutionStates::DATAREADY);

						//std::cout << "Running algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num << std::endl;
						log << MSG::DEBUG << "Running algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num << endmsg;

						IAlgorithm* ialgoPtr=nullptr;
						sc = m_algPool->acquireAlgorithm(ialgorithm->name(),ialgoPtr, true); //blocking call

						if(sc.isFailure() || ialgoPtr == nullptr){
							log << MSG::ERROR << "Could not acquire algorithm " << ialgorithm->name() << endmsg;
							m_eventContext->m_evt_failed=true;
						} else { // we got an algorithm

							//promote algorithm to scheduled
							algStates.updateState(algIndex,AlgsExecutionStates::SCHEDULED);

							Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?
							algoPtr->setContext(m_eventContext);

							// Call the execute() method
							try {
								RetCodeGuard rcg(appmgr, Gaudi::ReturnCode::UnhandledException);
								sc = algoPtr->sysExecute();
								if (UNLIKELY(!sc.isSuccess()))  {
									log << MSG::WARNING  << "Execution of algorithm " << ialgorithm->name() << " failed" << endmsg;
									eventFailed = true;
								}
								rcg.ignore(); // disarm the guard
							} catch ( const GaudiException& Exception ) {
								log << MSG::ERROR << ".executeEvent(): Exception with tag=" << Exception.tag()
																				   << " thrown by " << ialgorithm->name() << endmsg;
								log << MSG::ERROR << Exception << endmsg;
							} catch ( const std::exception& Exception ) {
								log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by "
										<< ialgorithm->name() << endmsg;
								log << MSG::ERROR <<  Exception.what()  << endmsg;
							} catch(...) {
								log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by "
										<< ialgorithm->name() << endmsg;
							}

							if(sc.isFailure()){
								eventFailed = true;
							}

							//std::cout << "Algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num
							//		<< (eventFailed ? " failed" : " succeeded") << std::endl;
							log << MSG::DEBUG << "Algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num
									<< (eventFailed ? " failed" : " succeded") << endmsg;

							AlgsExecutionStates::State state;
							if (ialgoPtr->filterPassed()){
							    state = AlgsExecutionStates::State::EVTACCEPTED;
							} else {
							    state = AlgsExecutionStates::State::EVTREJECTED;
							}

							//std::cout << "Algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num
							//		<< (ialgoPtr->filterPassed() ? " passed" : " rejected") << std::endl;
							log << MSG::DEBUG << "Algorithm [" << algIndex << "] " << ialgorithm->name() << " for event " << m_eventContext->m_evt_num
									<< (ialgoPtr->filterPassed() ? " passed" : " rejected") << endmsg;

							sc = m_algPool->releaseAlgorithm(ialgorithm->name(),ialgoPtr);

							algStates.updateState(algIndex,state);

							//just for debug: look at products -- not thread safe
							// Update the catalog: some new products may be there
							/*m_scheduler->m_whiteboard->selectStore(m_eventContext->m_evt_slot).ignore();

							// update prods in the dataflow
							// DP: Handles could be used. Just update what the algo wrote
							std::vector<std::string> new_products;
							m_scheduler->m_whiteboard->getNewDataObjects(new_products).ignore();
							for (const auto& new_product : new_products)
								std::cout << "Found in WB: " << new_product << std::endl;
							//dataFlow.updateDataObjectsCatalog(new_products);*/
						}

					}
			});

		m_scheduler->m_controlFlow.updateEventState(algStates.m_states, nodeDecisions);
		m_scheduler->m_controlFlow.promoteToControlReadyState(algStates.m_states, nodeDecisions);

		if(eventFailed){
			m_eventContext->m_evt_failed=eventFailed;
			//std::cout << "ERROR: " << "event " << m_eventContext->m_evt_num << " failed" << std::endl;
			break;
		}

		if(!algStates.algsPresent(AlgsExecutionStates::State::CONTROLREADY) && !algStates.allAlgsExecuted()){
			//std::cout << "WARNING: " << " not all algorithms executed for event " << m_eventContext->m_evt_num << std::endl;

			/*std::for_each(m_scheduler->m_algList.begin(), m_scheduler->m_algList.end(),

					[&] (IAlgorithm* ialgorithm) {
						uint algIndex = m_scheduler->m_algname_index_map[ialgorithm->name()];

						if(AlgsExecutionStates::State::SCHEDULED >= algStates.algorithmState(algIndex))
							std::cout << "Event [" << m_eventContext->m_evt_num << "] algorithm " << ialgorithm->name()
								<< " NOT executed" << std::endl;

			});*/
		}
	}

	m_scheduler->m_finishedEvents.push(m_eventContext);

	return nullptr;

}

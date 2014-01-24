// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/AppReturnCode.h"

#include "GaudiKernel/ContextSpecificPtr.h"

// C++
#include <list>
#include <thread>
#include <csignal>

// Local 
#include "RoundRobinSchedulerSvc.h"
#include "AlgResourcePool.h"
#include "RetCodeGuard.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(RoundRobinSchedulerSvc)

//===========================================================================
// Infrastructure methods

RoundRobinSchedulerSvc::RoundRobinSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc){
  declareProperty("UseTopAlgList", m_useTopAlgList=true);
  declareProperty("SimultaneousEvents", m_freeSlots=1);
}

//---------------------------------------------------------------------------
RoundRobinSchedulerSvc::~RoundRobinSchedulerSvc(){}
//---------------------------------------------------------------------------

StatusCode RoundRobinSchedulerSvc::initialize(){

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

  	m_controlFlow.initialize(algPool->getControlFlowGraph(), m_algname_index_map);

  return StatusCode::SUCCESS;

  // prepare the event slots
  // TODO !  

}
//---------------------------------------------------------------------------  

StatusCode RoundRobinSchedulerSvc::finalize(){
  StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
	  warning () << "Base class could not be finalized" << endmsg;
  return sc;  
}

//---------------------------------------------------------------------------  

/** Make an event available to the scheduler. Immediately the algortihms are 
 * executed.
 */
StatusCode RoundRobinSchedulerSvc::pushNewEvent(EventContext* eventContext){

	// consistency check
	if (!(m_freeSlots > 0)) {
		fatal() << "More contexts than slots provided" << m_freeSlots << endmsg;
		return StatusCode::FAILURE;
	}

	--m_freeSlots;
	m_evtCtx_buffer.push_back(eventContext);
	eventContext->m_evt_failed = false;

	return m_freeSlots > 0 ? StatusCode::SUCCESS : processEvents();
}

StatusCode RoundRobinSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){
	// consistency check
	if (eventContexts.size() > m_freeSlots) {
		fatal() << "More contexts than slots provided" << m_freeSlots << endmsg;
		return StatusCode::FAILURE;
	}
	m_freeSlots -= eventContexts.size();

	m_evtCtx_buffer.insert(m_evtCtx_buffer.end(), eventContexts.begin(), eventContexts.end());

	return m_freeSlots > 0 ? StatusCode::SUCCESS : processEvents();
}

//---------------------------------------------------------------------------
StatusCode RoundRobinSchedulerSvc::processEvents(){
	StatusCode sc(StatusCode::SUCCESS);

	// Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
	const SmartIF<IProperty> appmgr(serviceLocator());
	SmartIF<IMessageSvc> messageSvc (serviceLocator());

	//initialize control algorithm states and decisions
	AlgsExecutionStates algStates(m_algList.size(), messageSvc);
	const AlgResourcePool* algPool = dynamic_cast<const AlgResourcePool*>(m_algResourcePool.get());
	std::vector<int> nodeDecisions(algPool->getControlFlowGraph()->getControlFlowNodeCounter(), -1);


	m_controlFlow.updateEventState(algStates, nodeDecisions);
	m_controlFlow.promoteToControlReadyState(algStates, nodeDecisions);

	//initialize data flow manager
	//DataFlowManager dataFlow(m_scheduler->m_algosDependencies);

	info() << "Got " << m_evtCtx_buffer.size() << " events, starting loop" << endmsg;

	while(algStates.algsPresent(AlgsExecutionStates::State::CONTROLREADY) ){

		debug() << "algorithms left" << endmsg;

		//std::for_each(algStates.begin(AlgsExecutionStates::State::CONTROLREADY), algStates.end(AlgsExecutionStates::State::CONTROLREADY),

				//[&] (uint algIndex) {
		for(auto it = algStates.begin(AlgsExecutionStates::State::CONTROLREADY); it != algStates.end(AlgsExecutionStates::State::CONTROLREADY); ++it){

				uint algIndex = *it;

				std::string algName = m_algname_vect[algIndex];

				debug() << "Running algorithm [" << algIndex << "] " << algName << endmsg;

				std::vector<AlgsExecutionStates::State> algResults(m_evtCtx_buffer.size());

				//promote algorithm to data ready
				algStates.updateState(algIndex,AlgsExecutionStates::DATAREADY);

				IAlgorithm* ialgoPtr=nullptr;
				m_algResourcePool->acquireAlgorithm(algName, ialgoPtr);
				//promote algorithm to scheduled
				algStates.updateState(algIndex,AlgsExecutionStates::SCHEDULED);

				Algorithm* algoPtr = dynamic_cast<Algorithm*> (ialgoPtr); // DP: expose the setter of the context?
				algoPtr->resetExecuted();

				for (uint i = 0; i < m_evtCtx_buffer.size(); ++i) {
					if (false == m_evtCtx_buffer[i]->m_evt_failed) {
						bool eventfailed=false;

						m_evtCtx_buffer[i]->m_thread_id = pthread_self();
						algoPtr->resetExecuted();
						algoPtr->setContext(m_evtCtx_buffer[i]);
						Gaudi::Hive::setCurrentContextId(m_evtCtx_buffer[i]->m_evt_slot);
						// Call the execute() method
						try {
							RetCodeGuard rcg(appmgr, Gaudi::ReturnCode::UnhandledException);
							sc = ialgoPtr->sysExecute();
							if (UNLIKELY(!sc.isSuccess()))  {
								warning() << "Execution of algorithm " << algName << " failed for event " << m_evtCtx_buffer[i]->m_evt_num << endmsg;
								eventfailed = true;
							}
							rcg.ignore(); // disarm the guard
						} catch ( const GaudiException& Exception ) {
							error() << ".executeEvent(): Exception with tag=" << Exception.tag()
        						   << " thrown by " << algName << endmsg;
							error() << Exception << endmsg;
						} catch ( const std::exception& Exception ) {
							fatal() << ".executeEvent(): Standard std::exception thrown by "
									<< algName << endmsg;
							error() <<  Exception.what()  << endmsg;
						} catch(...) {
							fatal() << ".executeEvent(): UNKNOWN Exception thrown by "
									<< algName << endmsg;
						}
						m_evtCtx_buffer[i]->m_evt_failed=eventfailed;
					}

					if (ialgoPtr->filterPassed()){
						algResults[i] = AlgsExecutionStates::State::EVTACCEPTED;
					} else {
						algResults[i] = AlgsExecutionStates::State::EVTREJECTED;
					}

				}

				m_algResourcePool->releaseAlgorithm(algName,ialgoPtr);

				AlgsExecutionStates::State result = algResults[0];
				bool unanimous = true;
				for(uint i = 1; i < algResults.size(); ++i)
					if(result != algResults[i])
						unanimous = false;

				if(unanimous)
					algStates.updateState(algIndex,result);
				else{
					fatal() << "divergent algorithm execution" << endmsg;
					fatal() << "Algorithm results: ";
					for(uint i =0; i < algResults.size(); ++i){
						fatal() << i << ": " << (algResults[i] ==  AlgsExecutionStates::State::EVTACCEPTED ? "A" : "R") << "\t";
						if(algResults[i] ==  AlgsExecutionStates::State::EVTREJECTED){
							//std::cerr << m_evtCtx_buffer[i]->m_evt_num << std::endl;
						}
					}
					fatal() << endmsg;

					sc = StatusCode::FAILURE;
				}
		}
		//});

		if(sc.isFailure())
			break; //abort execution of events, something went wrong

		m_controlFlow.updateEventState(algStates, nodeDecisions);
		m_controlFlow.promoteToControlReadyState(algStates, nodeDecisions);
	}
	for (EventContext* eventContext : m_evtCtx_buffer) {
		m_finishedEvents.push(eventContext);
	}

	m_evtCtx_buffer.clear();

	return sc; //TODO: define proper return value
}

//---------------------------------------------------------------------------   
/// Blocks until an event is availble
StatusCode RoundRobinSchedulerSvc::popFinishedEvent(EventContext*& eventContext){

	if(m_finishedEvents.empty() && !m_evtCtx_buffer.empty())
		processEvents();

	m_finishedEvents.pop(eventContext);
  m_freeSlots++;
  debug() << "Popped slot " << eventContext->m_evt_slot << "(event "
          << eventContext->m_evt_num << ")" << endmsg;
  return StatusCode::SUCCESS;
}
  
//---------------------------------------------------------------------------  
/// Try to get a finished event, if not available just return a failure 
StatusCode RoundRobinSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext){
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
unsigned int RoundRobinSchedulerSvc::freeSlots(){return m_freeSlots;}

//---------------------------------------------------------------------------

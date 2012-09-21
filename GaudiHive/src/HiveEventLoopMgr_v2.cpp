#include <algorithm>
#include <tuple>

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/DataSvc.h"

#include "HistogramAgent.h"

// For concurrency
#include "GaudiHive/HiveEventLoopMgr_v2.h"
#include "GaudiHive/HiveAlgorithmManager.h"
#include "GaudiHive/EventSchedulingState.h"
#include "HiveEventRegistryEntry.h"

#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/Algorithm.h"

#include <pthread.h> // only for the tID!

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(HiveEventLoopMgr_v2)


#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

/////////////////////////////////////////////////
/// *dirty* place for adding an AlgoTask wrapper
class HiveAlgoTask_v2 : public tbb::task {
public:
  HiveAlgoTask_v2(IAlgorithm* algorithm,
		          EventSchedulingState* scheduler,
		          HiveEventLoopMgr_v2* eventloopmanager): m_algorithm(algorithm),
		          	  	  	  	  	  	                	  m_scheduler(scheduler),
		          	  	  	  	  	  	  	                  m_eventloopmanager(eventloopmanager){};
  tbb::task* execute();
  IAlgorithm* m_algorithm;
  EventSchedulingState* m_scheduler;
  HiveEventLoopMgr_v2* m_eventloopmanager;
};

tbb::task* HiveAlgoTask_v2::execute() {
  Algorithm* this_algo = dynamic_cast<Algorithm*>(m_algorithm);
  this_algo->getContext()->m_thread_id = pthread_self();
  m_algorithm->sysExecute();
  m_scheduler->algoFinished();
  // put back the algo into the hive algorithm manager
  m_eventloopmanager->taskFinished(m_algorithm); // TODO do this with index: put index in context?
  return NULL;
}

/////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
HiveEventLoopMgr_v2::HiveEventLoopMgr_v2(const std::string& nam, ISvcLocator* svcLoc)
: MinimalEventLoopMgr(nam, svcLoc) 
{
  m_histoDataMgrSvc   = 0;
  m_histoPersSvc      = 0;
  m_evtDataMgrSvc     = 0;
  m_evtDataSvc        = 0;
  m_evtSelector       = 0;
  m_evtContext        = 0;
  m_endEventFired     = true;
  m_total_algos_in_flight=0;
  m_max_parallel = 1;
  m_evts_parallel = 1;
  m_num_threads = 1;
  m_DumpQueues = true;

  // Declare properties
  declareProperty("HistogramPersistency", m_histPersName = "");
  declareProperty("EvtSel", m_evtsel );
  declareProperty("Warnings",m_warnings=true,
		  "Set this property to false to suppress warning messages");
  declareProperty("MaxAlgosParallel", m_max_parallel );
  declareProperty("MaxEventsParallel", m_evts_parallel);
  declareProperty("NumThreads", m_num_threads);
  declareProperty("DumpQueues", m_DumpQueues);
  declareProperty("CloneAlgorithms", m_CloneAlgorithms= false);
}

//--------------------------------------------------------------------------------------------
// Standard Destructor
//--------------------------------------------------------------------------------------------
HiveEventLoopMgr_v2::~HiveEventLoopMgr_v2()   {
  if( m_histoDataMgrSvc ) m_histoDataMgrSvc->release();
  if( m_histoPersSvc ) m_histoPersSvc->release();
  if( m_evtDataMgrSvc ) m_evtDataMgrSvc->release();
  if( m_evtDataSvc ) m_evtDataSvc->release();
  if( m_evtSelector ) m_evtSelector->release();
  if( m_evtContext ) delete m_evtContext;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::initialize
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::initialize()    {
  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::initialize();
  if( !sc.isSuccess() ) {
    DEBMSG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }
  
  find_dependencies();

  // Setup access to event data services
  m_evtDataMgrSvc = serviceLocator()->service("EventDataSvc");
  if( !m_evtDataMgrSvc.isValid() )  {
    fatal() << "Error retrieving EventDataSvc interface IDataManagerSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  m_evtDataSvc = serviceLocator()->service("EventDataSvc");
  if( !m_evtDataSvc.isValid() )  {
    fatal() << "Error retrieving EventDataSvc interface IDataProviderSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  // Obtain the IProperty of the ApplicationMgr
  m_appMgrProperty = serviceLocator();
  if ( ! m_appMgrProperty.isValid() )   {
    fatal() << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  }

  // We do not expect a Event Selector necessarily being declared
  setProperty(m_appMgrProperty->getProperty("EvtSel")).ignore();

  if( m_evtsel != "NONE" || m_evtsel.length() == 0) {
    m_evtSelector = serviceLocator()->service("EventSelector");
    if( m_evtSelector.isValid() ) {
      // Setup Event Selector
      sc=m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() )   {
        fatal() << "Can not create the event selector Context." << endmsg;
        return sc;
      }
    }
    else {
      fatal() << "EventSelector not found." << endmsg;
      return sc;
    }
  }
  else {
    m_evtSelector = 0;
    m_evtContext = 0;
    if ( m_warnings ) {
      warning() << "Unable to locate service \"EventSelector\" " << endmsg;
      warning() << "No events will be processed from external input." << endmsg;
    }
  }

  // Setup access to histogramming services
  m_histoDataMgrSvc = serviceLocator()->service("HistogramDataSvc");
  if( !m_histoDataMgrSvc.isValid() )  {
    fatal() << "Error retrieving HistogramDataSvc." << endmsg;
    return sc;
  }
  // Setup histogram persistency
  m_histoPersSvc = serviceLocator()->service("HistogramPersistencySvc");
  if( !m_histoPersSvc.isValid() ) {
    warning() << "Histograms cannot not be saved - though required." << endmsg;
    return sc;
  }

  // Setup tbb task scheduler
  // TODO: shouldn't be in this case
   m_tbb_scheduler_init = new tbb::task_scheduler_init(m_num_threads);

  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::reinitialize() {

  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::reinitialize();
  if( !sc.isSuccess() ) {
    DEBMSG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }

  // Check to see whether a new Event Selector has been specified
  setProperty(m_appMgrProperty->getProperty("EvtSel"));
  if( m_evtsel != "NONE" || m_evtsel.length() == 0) {
    SmartIF<IService> theSvc(serviceLocator()->service("EventSelector"));
    SmartIF<IEvtSelector> theEvtSel(theSvc);
    if( theEvtSel.isValid() && ( theEvtSel.get() != m_evtSelector.get() ) ) {
      // Setup Event Selector
      if ( m_evtSelector.get() && m_evtContext ) {
        // Need to release context before switching to new event selector
        m_evtSelector->releaseContext(m_evtContext);
        m_evtContext = 0;
      }
      m_evtSelector = theEvtSel;
      if (theSvc->FSMState() == Gaudi::StateMachine::INITIALIZED) {
        sc = theSvc->reinitialize();
        if( !sc.isSuccess() ) {
          error() << "Failure Reinitializing EventSelector "
                  << theSvc->name( ) << endmsg;
          return sc;
        }
      }
      else {
        sc = theSvc->sysInitialize();
        if( !sc.isSuccess() ) {
          error() << "Failure Initializing EventSelector "
                  << theSvc->name( ) << endmsg;
          return sc;
        }
      }
      sc = m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() ) {
        error() << "Can not create Context " << theSvc->name( ) << endmsg;
        return sc;
      }
      info() << "EventSelector service changed to "
             << theSvc->name( ) << endmsg;
    }
    else if ( m_evtSelector.isValid() ) {
      if ( m_evtContext ) {
        m_evtSelector->releaseContext(m_evtContext);
        m_evtContext = 0;
      }
      sc = m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() ) {
        error() << "Can not create Context " << theSvc->name( ) << endmsg;
        return sc;
      }
    }
  }
  else if ( m_evtSelector.isValid() && m_evtContext ) {
    m_evtSelector->releaseContext(m_evtContext);
    m_evtSelector = 0;
    m_evtContext = 0;
  }
  return StatusCode::SUCCESS;
}


//--------------------------------------------------------------------------------------------
// implementation of IService::stop
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::stop()    {
  if ( ! m_endEventFired ) {
    // Fire pending EndEvent incident
    m_incidentSvc->fireIncident(Incident(name(),IncidentType::EndEvent));
    m_endEventFired = true;
  }
  return MinimalEventLoopMgr::stop();
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::finalize
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::finalize()    {
  StatusCode sc;

  // Finalize base class
  sc = MinimalEventLoopMgr::finalize();
  if (! sc.isSuccess()) {
    error() << "Error finalizing base class" << endmsg;
    return sc;
  }

  // Save Histograms Now
  if ( m_histoPersSvc != 0 )    {
    HistogramAgent agent;
    sc = m_histoDataMgrSvc->traverseTree( &agent );
    if( sc.isSuccess() )   {
      IDataSelector* objects = agent.selectedObjects();
      // skip /stat entry!
      if ( objects->size() > 0 )    {
        IDataSelector::iterator i;
        for ( i = objects->begin(); i != objects->end(); i++ )    {
          IOpaqueAddress* pAddr = 0;
          StatusCode iret = m_histoPersSvc->createRep(*i, pAddr);
          if ( iret.isSuccess() )     {
            (*i)->registry()->setAddress(pAddr);
          }
          else  {
            sc = iret;
          }
        }
        for ( i = objects->begin(); i != objects->end(); i++ )    {
          IRegistry* reg = (*i)->registry();
          StatusCode iret = m_histoPersSvc->fillRepRefs(reg->address(), *i);
          if ( !iret.isSuccess() )    {
            sc = iret;
          }
        }
      }
      if ( sc.isSuccess() )    {
        info() << "Histograms converted successfully according to request." << endmsg;
      }
      else  {
        error() << "Error while saving Histograms." << endmsg;
      }
    }
    else {
      error() << "Error while traversing Histogram data store" << endmsg;
    }
  }

  // Release event selector context
  if ( m_evtSelector && m_evtContext )   {
    m_evtSelector->releaseContext(m_evtContext).ignore();
    m_evtContext = 0;
  }

  // Release all interfaces...
  m_histoDataMgrSvc = 0;
  m_histoPersSvc    = 0;

  m_evtSelector     = 0;
  m_evtDataSvc      = 0;
  m_evtDataMgrSvc   = 0;

  delete m_tbb_scheduler_init;

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::executeEvent(void* /*par*/)    {

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident(Incident(name(),IncidentType::BeginEvent));
  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  if ( m_scheduledStop ) {
    always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute Algorithms
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginProcessing));

  // Prepare the event context for concurrency


  // Call the resetExecuted() method of ALL "known" algorithms
  // (before we were reseting only the topalgs)
  SmartIF<IAlgManager> algMan(serviceLocator());
  if (LIKELY(algMan.isValid())) {
    const ListAlgPtrs& allAlgs = algMan->getAlgorithms() ;
    for( ListAlgPtrs::const_iterator ialg = allAlgs.begin() ; allAlgs.end() != ialg ; ++ialg ) {
      if (LIKELY(0 != *ialg)) (*ialg)->resetExecuted();
    }
  }

  // Some debug
  if(m_DumpQueues){
	  HiveAlgorithmManager* hivealgman = dynamic_cast<HiveAlgorithmManager*> (algMan.get());
	  if (hivealgman) hivealgman->dump();
  	  }

  bool eventfailed = run_parallel();

  // ensure that the abortEvent flag is cleared before the next event
  if (UNLIKELY(m_abortEvent)) {
    DEBMSG << "AbortEvent incident fired by " << m_abortEventSource << endmsg;
    m_abortEvent = false;
  }

  // Call the execute() method of all output streams
  for (ListAlg::iterator ito = m_outStreamList.begin(); ito != m_outStreamList.end(); ito++ ) {
    (*ito)->resetExecuted();
    StatusCode sc;
    sc = (*ito)->sysExecute();
    if (UNLIKELY(!sc.isSuccess())) {
      warning() << "Execution of output stream " << (*ito)->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  m_incidentSvc->fireIncident(Incident(name(), IncidentType::EndProcessing));

  // Check if there was an error processing current event
  if (UNLIKELY(eventfailed)){
    error() << "Error processing event loop." << endmsg;
    return StatusCode(StatusCode::FAILURE,true);
  }
  return StatusCode(StatusCode::SUCCESS,true);

}

//--------------------------------------------------------------------------------------------
// implementation of IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr_v2::executeRun( int maxevt )    {
  StatusCode  sc;
  // initialize the base class
  sc = MinimalEventLoopMgr::executeRun(maxevt);
  return sc;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
// Here the loop on the events takes place.
// This is also the natural place to put the preparation of the algorithms
// contexts, which contain the event specific data.

StatusCode HiveEventLoopMgr_v2::nextEvent(int maxevt)   {
  // Collapse executeEvent and run_parallel in the same method
  // TODO _very_ sporty on conditions and checks!!

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "Running with " << m_evts_parallel << " parallel events and "
	  << m_max_parallel << " algorithms." << endmsg;

  DataObject*       pObject = 0;
  StatusCode        sc(StatusCode::SUCCESS, true);

  // Reset the application return code.
  Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::Success, true).ignore();

  // The loop on events which considers several events simultaneously
  // Algorithms are scheduled for different events on the same footing
  // To be made members

  // Maybe too many jumps in memory: deque? vector?
  typedef std::tuple<EventContext*,EventSchedulingState*> contextSchedState_tuple;
  std::list<contextSchedState_tuple> events_in_flight;

  // Lambda to check if an event has finished
  auto has_finished = [] // acquire nothing
       		          (contextSchedState_tuple evtContext_evtstate) // argument is a tuple
       		          { return std::get<1>(evtContext_evtstate)->hasFinished();}; // true if finished

  int n_processed_events = 0;

  // Create the root event into which we put all events
  sc = m_evtDataMgrSvc->setRoot ("/Event", new DataObject());
  if( !sc.isSuccess() )  {
    warning() << "Error declaring event root DataObject" << endmsg;
  }

  // Get the algorithm Manager
  SmartIF<IAlgManager> algMan(serviceLocator());
  HiveAlgorithmManager* hivealgman = dynamic_cast<HiveAlgorithmManager*> (algMan.get());

  // Loop until no more evts are there
  while (maxevt == -1 ? true : n_processed_events<maxevt){// TODO Fix the condition in case of -1

	  // Get the number of events in flight
	  const unsigned int n_events_in_flight = events_in_flight.size();

	  const unsigned int n_evts_to_process = maxevt -n_processed_events - n_events_in_flight;

	  // Now calculate how many are acquirable
	  log << MSG::INFO << "Evts in flight: " <<  n_events_in_flight << endmsg;
	  log << MSG::INFO << "Evts processed: " <<  n_processed_events << endmsg;
	  log << MSG::INFO << "Evts parallel: " << m_evts_parallel << endmsg;


	  unsigned int n_acquirable_events = m_evts_parallel - n_events_in_flight;
	  if (n_acquirable_events > n_evts_to_process)
		  n_acquirable_events = n_evts_to_process;

	  log << MSG::INFO << "Acquirable Events are " << n_acquirable_events << endmsg;
	  // TODO Adjust the n_acquirable_events according to the available evts....

	  // Initialisation section ------------------------------------------------

	  // Retrieve the event (even outside the loop?)
	  m_evtDataSvc->retrieveObject("/Event",pObject);
	  typedef DataSvcHelpers::RegistryEntry regEntry;
	  regEntry* rootRegistry = dynamic_cast<regEntry*>(pObject->registry()); //TODO: an interface in the evtDataSvc for it would come handy

	  // Loop on events to be initialised
	  for (unsigned int offset=0; offset< n_acquirable_events; ++offset){
		  log << MSG::DEBUG << "Acquiring events and preparing ctxts" << endmsg;
		  // Prepare the event context.
		  EventContext* evtContext(new EventContext);

		  // Put an evt number inside
		  const int evt_num =  n_processed_events + offset + n_events_in_flight;

		  evtContext->m_evt_num = evt_num;

		  // Put a registry entry inside the evt
		  std::string evtname ("Evt");evtname+=evt_num;
		  Hive::HiveEventRegistryEntry* evt_registry = new Hive::HiveEventRegistryEntry(evtname,rootRegistry);
		  log << MSG::DEBUG << "Regentry is " << evt_registry << endmsg;
		  rootRegistry->add(evt_registry);
		  evtContext->m_registry = evt_registry;

		  // Now save the context in flight events
		  EventSchedulingState* event_state = new EventSchedulingState(m_topAlgList.size());
		  events_in_flight.push_back(std::make_tuple(evtContext,event_state));
		  log << MSG::INFO << "Event " << evt_num << " created. "
				  << "Now evts in flight are: " << events_in_flight.size() << endmsg;
	  }// End initialisation loop on acquired events
	  // End initialisation session --------------------------------------------

	  // Scheduling session ----------------------------------------------------
	  while (events_in_flight.end() ==
			  find_if(events_in_flight.begin(), events_in_flight.end(),has_finished)){ // loop until at least one evt finished
		  bool eventfailed=false;

		  // loop on algorithms indices
		  for (unsigned int algo_counter=0; algo_counter<m_topAlgList.size(); algo_counter++) {
			  // loop on events
			  for (auto& evtContext_evtstate : events_in_flight){
				  //std::cout << "Loop on evts in flight\n";
				  // Just to be clear
				  EventContext*& event_Context = std::get<0>(evtContext_evtstate);
				  EventSchedulingState*& event_state = std::get<1>(evtContext_evtstate);

				  // check whether all requirements/dependencies for the algorithm are fulfilled...
				  const state_type& algo_requirements = m_all_requirements[algo_counter];
				  state_type dependencies_missing = (event_state->state() & algo_requirements) ^ algo_requirements;

				  // ...and whether the algorithm was already started and if it can be started
				  if ( (dependencies_missing == 0) &&
						  (event_state->hasStarted(algo_counter) ) == false &&
						  (m_total_algos_in_flight < m_max_parallel )) {
					  // Pick the algorithm if available and if not create one!
					  IAlgorithm* ialgo=NULL;
					  if(hivealgman->acquireAlgorithm(algo_counter,ialgo,m_CloneAlgorithms)){
						  log << MSG::INFO << "Launching algo " << algo_counter<< endmsg;
						  // Attach context to the algo (a tuple instead of a member?)
						  Algorithm* algo = dynamic_cast<Algorithm*> (ialgo); // because of old interface
						  algo->setContext(event_Context);

						  tbb::task* t = new( tbb::task::allocate_root() ) HiveAlgoTask_v2(ialgo, event_state, this);
						  tbb::task::enqueue( *t);
						  event_state->algoStarts(algo_counter);
						  // Decremented by the task which before exiting calls HiveEventLoopMgr_v2::taskFinished
						  ++m_total_algos_in_flight;
						  log << MSG::INFO << "Algos in flight: " <<  m_total_algos_in_flight << endmsg;
					  }
				  }

				  // update the event state with what has been put into the DataSvc
				  bool queue_full(false);
				  std::string product_name;

				  Hive::HiveEventRegistryEntry* hiveregistryentry= dynamic_cast<Hive::HiveEventRegistryEntry*>(event_Context->m_registry);
				  tbb::concurrent_queue<std::string>& new_products = hiveregistryentry->new_products();
				  do {
					  queue_full = new_products.try_pop(product_name);
					  if (queue_full && m_product_indices.count( product_name ) == 1) { // only products with dependencies upon need to be announced to other algos
						  event_state->update_state(m_product_indices[product_name]);
						  std::cout << "State updated" << std::endl;
					  }
				  } while (queue_full);
			  }// end loop on evts in flight
		  }// end loop on algo indices

		  // Call the execute() method of all output streams
		  for (ListAlg::iterator ito = m_outStreamList.begin(); ito != m_outStreamList.end(); ito++ ) {
			  (*ito)->resetExecuted();
			  StatusCode sc;
			  sc = (*ito)->sysExecute();
			  if (UNLIKELY(!sc.isSuccess())) {
				  warning() << "Execution of output stream " << (*ito)->name() << " failed" << endmsg;
				  eventfailed = true;
			  }
		  }
	  }// end loop until at least one evt in flight finished

	  // Remove from the in flight events the finished ones
	  std::list<contextSchedState_tuple>::iterator it;
	  for (it=events_in_flight.begin();it!=events_in_flight.end();++it){
		  if (std::get<1>(*it)->hasFinished()){
			  //			  delete std::get<0>(*it);
			  //			  delete std::get<1>(*it);
			  events_in_flight.erase(it++) ; // remove the value of it as it was before the increment
			  n_processed_events++;
			  log << MSG::INFO << "One event finished. Events in fight are "
					  << events_in_flight.size() << ". Processed events are "
					  <<  n_processed_events << endmsg;
			  // Some debug
			  if(m_DumpQueues){
				  if (hivealgman) hivealgman->dump();
			  }

		  }
	  }

	  // End scheduling session ------------------------------------------------

  } // End while loop on events

  return StatusCode::SUCCESS;

}

/// Create event address using event selector
StatusCode HiveEventLoopMgr_v2::getEventRoot(IOpaqueAddress*& refpAddr)  {
  refpAddr = 0;
  StatusCode sc = m_evtSelector->next(*m_evtContext);
  if ( !sc.isSuccess() )  {
    return sc;
  }
  // Create root address and assign address to data service
  sc = m_evtSelector->createAddress(*m_evtContext,refpAddr);
  if( !sc.isSuccess() )  {
    sc = m_evtSelector->next(*m_evtContext);
    if ( sc.isSuccess() )  {
      sc = m_evtSelector->createAddress(*m_evtContext,refpAddr);
      if ( !sc.isSuccess() )  {
        warning() << "Error creating IOpaqueAddress." << endmsg;
      }
    }
  }
  return sc;
}

// Here because temporary
#include <iostream>


/// Compute dependencies between the algorithms
void
HiveEventLoopMgr_v2::find_dependencies() {
  
    /**
     * This is not very simple, but here you have the reasons:
     * o We input the inputs and outputs as "vectors" in the config as PROPERTIES
     * o The modules store this as vector of strings
     * o We need to massage them:(
     * We opt for testing the scheduler and then properly change the interfaces.
     */
    auto tokenize_gaudi_string_vector = 
      [] (std::string s) -> const std::vector<std::string> {
        for (const char c: {'\'',']','['})
          replace(s.begin(), s.end(), c, ' ');
        // remove spaces
        s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
        // replace commas with spaces
        replace(s.begin(), s.end(), ',', ' ');
        // tokenize
        std::vector<std::string> tokens;
        std::stringstream os(s);
        std::string tmp;
        while( os >> tmp )
          tokens.push_back(tmp);
        return tokens;
        };
        
    auto get_algo_collections = 
     [tokenize_gaudi_string_vector] (IAlgorithm* algo, const std::string & type) -> const std::vector<std::string> {
       // This is how you can get the properties from the Ialgo and not the algo!
       SmartIF<IProperty> algo_properties(algo);
       return tokenize_gaudi_string_vector (algo_properties->getProperty(type).toString());
      };

    // the lambdas above will disappear ----------------- 
      
    const unsigned int n_algos = m_topAlgList.size();
    std::vector<state_type> all_requirements(n_algos);
   
    // Let's loop through all algos and their required inputs
    unsigned int algo_counter(0); 
    unsigned int input_counter(0);
    for (IAlgorithm* algo: m_topAlgList) {
      const std::vector<std::string>& inputs = get_algo_collections(algo,"Inputs");
      state_type requirements(0);
      for (const std::string& input: inputs){
	std::pair<std::map<std::string,unsigned int>::iterator,bool> ret;
        ret = m_product_indices.insert(std::pair<std::string, unsigned int>("/Event/"+input,input_counter));
        // insert successful means == wasn't known before. So increment counter
        if (ret.second==true) {
          ++input_counter;
        };
        // in any case the return value holds the proper product index 
        requirements[ret.first->second] = true;
      }
      all_requirements[algo_counter] = requirements;
      ++algo_counter;
    }
    m_numberOfAlgos = algo_counter;
    m_all_requirements = all_requirements;
}  

//--------------------------------------------------------------------------------------------
// bool run_parallel 
//--------------------------------------------------------------------------------------------
bool HiveEventLoopMgr_v2::run_parallel(){
  // Prepare the event context.
  // A ctor will come when the members are clearer.
  EventContext_shared_ptr evtContext(new EventContext);
  evtContext->m_evt_num = 42; //TODO: use nevt;
  // Assign the context to the algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  const ListAlgPtrs& allAlgs = algMan->getAlgorithms() ;
  for (IAlgorithm* ialgo : allAlgs) {
    Algorithm* algo = dynamic_cast<Algorithm*>(ialgo);
    algo->setContext(evtContext);
  }
  bool eventfailed = false;
  // Create object containing all scheduling specific information
  // for a single event
  // TODO: will have to be transformed into a vector of these states 
  // for multi-event case.
  EventSchedulingState event_state(m_topAlgList.size());

  DataObject* pObject;
  m_evtDataSvc->retrieveObject("/Event",pObject);
  DataSvcHelpers::RegistryEntry* rootRegistry = dynamic_cast<DataSvcHelpers::RegistryEntry*>(pObject->registry()); //TODO: an interface in the evtDataSvc for it would come handy

  // create the single event
  Hive::HiveEventRegistryEntry* evt_registry = new Hive::HiveEventRegistryEntry("NameDoesntMatter",rootRegistry);
  rootRegistry->add(evt_registry);
  evtContext->m_registry = evt_registry;

//  // Test the new pool
//  HiveAlgorithmManager* hivealgman = dynamic_cast<HiveAlgorithmManager*> (algMan.get());
//  IAlgorithm* tmpalg;
//  for (ListAlg::iterator ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ){
//	  const std::string& name = (*ita)->name();
//	  while( hivealgman->acquireAlgorithm(name,tmpalg) );
//	  hivealgman->createAlgorithm(name,tmpalg);
//  }
//  hivealgman->dump();
  do {
    unsigned int algo_counter(0);
    for (ListAlg::iterator ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
      StatusCode sc(StatusCode::SUCCESS); //TODO: disabled the failure part

      try {
        if (UNLIKELY(m_abortEvent)) {
          DEBMSG << "AbortEvent incident fired by "
                 << m_abortEventSource << endmsg;
          m_abortEvent = false;
          sc.ignore();
          break;
        }
        // check whether all requirements/dependencies for the algorithm are fulfilled...
        state_type dependencies_missing = (event_state.state() & m_all_requirements[algo_counter]) ^ m_all_requirements[algo_counter];  
        // ...and whether the algorithm was already started and it can be started
        if ( (dependencies_missing == 0) &&
        	 (event_state.hasStarted(algo_counter) ) == false &&
        	 (m_total_algos_in_flight < m_max_parallel )) {
          tbb::task* t = new( tbb::task::allocate_root() ) HiveAlgoTask_v2((*ita), &event_state, this);
          tbb::task::enqueue( *t);
          event_state.algoStarts(algo_counter);
          //++m_total_algos_in_flight;
        }
        ++algo_counter;
      } catch ( const GaudiException& Exception ) {
        fatal() << ".executeEvent(): Exception with tag=" << Exception.tag()
                << " thrown by " << (*ita)->name() << endmsg;
        error() << Exception << endmsg;
      } catch ( const std::exception& Exception ) {
        fatal() << ".executeEvent(): Standard std::exception thrown by "
                << (*ita)->name() << endmsg;
        error() << Exception.what()  << endmsg;
      } catch(...) {
        fatal() << ".executeEvent(): UNKNOWN Exception thrown by "
                << (*ita)->name() << endmsg;
      }

      if (UNLIKELY(!sc.isSuccess())) {
        warning() << "Execution of algorithm " << (*ita)->name() << " failed" << endmsg;
        eventfailed = true;
      }
    }  

    // update the event state with what has been put into the DataSvc
    bool queue_full(false);
    std::string product_name;

    // TODO: in the multi-event case here a loop through the individual event registries
    tbb::concurrent_queue<std::string>& new_products = evt_registry->new_products();
    do {
      queue_full = new_products.try_pop(product_name);
      if (queue_full && m_product_indices.count( product_name ) == 1) { // only products with dependencies upon need to be announced to other algos
        event_state.update_state(m_product_indices[product_name]); 
      }
    } while (queue_full);  

  } while (!event_state.hasFinished());

  return eventfailed;
}

//------------------------------------------------------------------------------

void HiveEventLoopMgr_v2::taskFinished(IAlgorithm*& algo){
	--m_total_algos_in_flight;
	SmartIF<IAlgManager> algMan(serviceLocator());
	HiveAlgorithmManager* hivealgman = dynamic_cast<HiveAlgorithmManager*> (algMan.get());
	hivealgman->releaseAlgorithm(algo->name(),algo);
	MsgStream log(msgSvc(), name());
	log << MSG::DEBUG << "Algos in flight: " <<  m_total_algos_in_flight << endmsg;
}

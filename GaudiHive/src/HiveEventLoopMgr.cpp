#define  GAUDIHIVE_HIVEEVENTLOOPMGR_CPP

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

#include "HistogramAgent.h"
#include "GaudiHive/HiveEventLoopMgr.h"

#include "tbb/task_scheduler_init.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(HiveEventLoopMgr)


#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
HiveEventLoopMgr::HiveEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc)
: MinimalEventLoopMgr(nam, svcLoc)
{
  m_histoDataMgrSvc   = 0;
  m_histoPersSvc      = 0;
  m_evtDataMgrSvc     = 0;
  m_evtDataSvc        = 0;
  m_evtSelector       = 0;
  m_evtContext        = 0;
  m_endEventFired     = true;

  // Declare properties
  declareProperty("HistogramPersistency", m_histPersName = "");
  declareProperty("EvtSel", m_evtsel );
  declareProperty("Warnings",m_warnings=true,
		  "Set this property to false to suppress warning messages");
}

//--------------------------------------------------------------------------------------------
// Standard Destructor
//--------------------------------------------------------------------------------------------
HiveEventLoopMgr::~HiveEventLoopMgr()   {
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
StatusCode HiveEventLoopMgr::initialize()    {
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
//   m_tbb_scheduler_init = new tbb::task_scheduler_init(3);

  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr::reinitialize() {

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
StatusCode HiveEventLoopMgr::stop()    {
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
StatusCode HiveEventLoopMgr::finalize()    {
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

//   delete m_tbb_scheduler_init;

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr::executeEvent(void* par)    {

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident(Incident(name(),IncidentType::BeginEvent));
  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  if ( m_scheduledStop ) {
    always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute Algorithms
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginProcessing));

  // Call the resetExecuted() method of ALL "known" algorithms
  // (before we were reseting only the topalgs)
  SmartIF<IAlgManager> algMan(serviceLocator());
  if (LIKELY(algMan.isValid())) {
    const ListAlgPtrs& allAlgs = algMan->getAlgorithms() ;
    for( ListAlgPtrs::const_iterator ialg = allAlgs.begin() ; allAlgs.end() != ialg ; ++ialg ) {
      if (LIKELY(0 != *ialg)) (*ialg)->resetExecuted();
    }
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
// IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr::executeRun( int maxevt )    {
  StatusCode  sc;
  // initialize the base class
  sc = MinimalEventLoopMgr::executeRun(maxevt);
  return sc;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
StatusCode HiveEventLoopMgr::nextEvent(int maxevt)   {
  static int        total_nevt = 0;
  DataObject*       pObject = 0;
  StatusCode        sc(StatusCode::SUCCESS, true);

  // Reset the application return code.
  Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::Success, true).ignore();

  // loop over events if the maxevt (received as input) if different from -1.
  // if evtmax is -1 it means infinite loop
  for( int nevt = 0; (maxevt == -1 ? true : nevt < maxevt);  nevt++, total_nevt++) {

    // Check if there is a scheduled stop issued by some algorithm/service
    if ( m_scheduledStop ) {
      m_scheduledStop = false;
      always() << "Terminating event processing loop due to scheduled stop" << endmsg;
      break;
    }
    // Clear the event store, if used in the event loop
    if( 0 != total_nevt ) {

      if ( ! m_endEventFired ) {
        // Fire EndEvent "Incident" (it is considered part of the clearing of the TS)
        m_incidentSvc->fireIncident(Incident(name(),IncidentType::EndEvent));
        m_endEventFired = true;
      }
      sc = m_evtDataMgrSvc->clearStore();
      if( !sc.isSuccess() )  {
        DEBMSG << "Clear of Event data store failed" << endmsg;
      }
    }

    // Setup event in the event store
    if( m_evtContext ) {
      IOpaqueAddress* addr = 0;
      // Only if there is a EventSelector
      sc = getEventRoot(addr);
      if( !sc.isSuccess() )  {
        info() << "No more events in event selection " << endmsg;
        break;
      }
      // Set root clears the event data store first
      sc = m_evtDataMgrSvc->setRoot ("/Event", addr);
      if( !sc.isSuccess() )  {
        warning() << "Error declaring event root address." << endmsg;
        continue;
      }
      sc = m_evtDataSvc->retrieveObject("/Event", pObject);
      if( !sc.isSuccess() ) {
        warning() << "Unable to retrieve Event root object" << endmsg;
        break;
      }
    }
    else {
      sc = m_evtDataMgrSvc->setRoot ("/Event", new DataObject());
      if( !sc.isSuccess() )  {
        warning() << "Error declaring event root DataObject" << endmsg;
      }
    }
    // Execute event for all required algorithms
    sc = executeEvent(NULL);
    m_endEventFired = false;
    if( !sc.isSuccess() ){
      error() << "Terminating event processing loop due to errors" << endmsg;
      Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::AlgorithmFailure).ignore();
      return sc;
    }
  }
  return StatusCode::SUCCESS;
}

/// Create event address using event selector
StatusCode HiveEventLoopMgr::getEventRoot(IOpaqueAddress*& refpAddr)  {
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
#include <algorithm>

/// Compute dependencies between the algorithms
void
HiveEventLoopMgr::find_dependencies() {
  
    /**
     * This is not very simple, but here you have the reasons:
     * o ATM the Algo and IAlgo were not modified
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
    // create the mapping productname : index
    std::map<std::string,unsigned int> product_indices;
   
    unsigned int algo_counter=0;
    for (IAlgorithm* algo: m_topAlgList) {
        const std::vector<std::string>& outputs = get_algo_collections(algo,"Outputs");
        for (const std::string& output: outputs){
            product_indices[output] = algo_counter;
        }
        algo_counter++;
    }
    // use the mapping to create a bit pattern of input requirements
    state_type termination_requirement(0);
    algo_counter=0;
    for (IAlgorithm* algo : m_topAlgList) {
        always() << " " << algo_counter << ": " << algo->name().c_str() << endmsg;
        state_type requirements(0);
        const std::vector<std::string>& inputs = get_algo_collections(algo,"Inputs");
        for (const std::string& input: inputs){
            const unsigned int input_index = product_indices[input];
            requirements[input_index] = true;
            always() << "\tconnecting to " <<  algo->name().c_str() 
                     << "(via '" << input.c_str() << "')" << endmsg;
        }
        all_requirements[algo_counter] = requirements;
        termination_requirement[algo_counter] = true;
    
      algo_counter++;
    }
    m_termination_requirement = termination_requirement;
    m_all_requirements = all_requirements;
}  

bool HiveEventLoopMgr::run_parallel(){
  bool eventfailed = false;
  
  // Call the execute() method of all top algorithms
  for (ListAlg::iterator ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    StatusCode sc(StatusCode::FAILURE);
    try {
      if (UNLIKELY(m_abortEvent)) {
        DEBMSG << "AbortEvent incident fired by "
               << m_abortEventSource << endmsg;
        m_abortEvent = false;
        sc.ignore();
        break;
      }
      sc = (*ita)->sysExecute();
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
  
  return eventfailed;
  
/**
 Started to adapt for Gaudi
   o Removed multiple events
 
    //get the bit patterns and sort by node id (like the available algos)
    std::vector<state_type> bits = m_all_requirements;
    // some book keeping vectors
    const size_t size = m_topAlgList.size();
    std::vector<EventState*> event_states(0);
    unsigned int in_flight(0), processed(0); 
    

    // now schedule whatever can be scheduled
    // loop through the entire vector of algo bits
    for (unsigned int algo = 0; algo < size; ++algo) {


      EventState*& event_state = event_states[event_id];
      // extract event_id specific quantities
      state_type& current_event_bits = event_state->state;
      // check whether all dependencies for the algorithm are fulfilled...
      state_type tmp = (current_event_bits & bits[algo]) ^ bits[algo];
      /// ...whether all required products are there...
        
      // ... and whether the algo was previously started
      tbb::concurrent_vector<bool>& algo_has_run = event_state->algos_started_;
      if ((tmp==0) && (algo_has_run[algo] == false)) {
          // is there an available Algo instance one can use?
          AlgoBase* algo_instance(0);
          bool algo_free(0);
          algo_free = algo_pool_.acquire(algo_instance, algo);
          if (algo_free) {
            AlgoTaskId* task = new AlgoTaskId(algos_[algo],algo,event_state);
            tbb::task* t = new( tbb::task::allocate_root() ) AlgoTask(task, this);
            tbb::task::enqueue( *t);
            algo_has_run[algo] = true;
          }
        }

    }
    
    task_cleanup();
    
    // check for finished events and clean up
    for (std::vector<EventState*>::iterator i = event_states.begin(), end = event_states.end(); i != end; ++i){
        if ((*i)->state == termination_requirement_) {
        Context*& context = (*i)->context;
        //printf("Finished event\n");
        wb_.release_context(context);
        event_loop_manager_->event_done();
        ++processed; //to be removed
        --in_flight; //to be removed
        delete (*i);
        i = event_states.erase(i);
        }
    }
 
**/  
  
}

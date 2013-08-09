// Framework includes
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/AppReturnCode.h"

// C++
#include <list>
#include <thread>

// Local 
#include "RoundRobinSchedulerSvc.h"
#include "AlgResourcePool.h"
#include "RetCodeGuard.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(RoundRobinSchedulerSvc)

//===========================================================================
// Infrastructure methods

RoundRobinSchedulerSvc::RoundRobinSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_freeSlots(1){ 
  declareProperty("UseTopAlgList", m_useTopAlgList=true);    
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
  SmartIF<IAlgResourcePool> algResourcePool (serviceLocator()->service("AlgResourcePool"));
  if (!algResourcePool.isValid()){
    error() << "Error retrieving AlgResourcePool" << endmsg;  
    return StatusCode::FAILURE;
  }
  
  // Get the list of algorithms
  m_algList = m_useTopAlgList ? algResourcePool->getTopAlgList() : algResourcePool->getFlatAlgList();
  info() << "Found " <<  m_algList.size() << " algorithms" << endmsg;    

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
  std::vector<EventContext*> eventContexts;
  eventContexts.push_back(eventContext);
  eventContext->m_evt_failed = false;
  return pushNewEvents(eventContexts);  
}

//---------------------------------------------------------------------------
StatusCode RoundRobinSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){
  StatusCode sc(StatusCode::SUCCESS);

  // consistency check
  if (eventContexts.size() > m_freeSlots) {
    fatal() << "More contexts than slots provided" << m_freeSlots << endmsg;
    return StatusCode::FAILURE;
  }
  m_freeSlots--;

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard                                     
  const SmartIF<IProperty> appmgr(serviceLocator());

  // Loop through all algos and then through all contexts
  for (IAlgorithm* ialgorithm : m_algList){
    Algorithm* this_algo = dynamic_cast<Algorithm*>(ialgorithm);
    if (!this_algo){
      throw GaudiException ("Cast to Algorithm failed!",
                            "RoundRobinSchedulerSvc",
                            StatusCode::FAILURE);
    }

    for (EventContext* eventContext : eventContexts) {
      if (false == eventContext->m_evt_failed) {
        bool eventfailed=false;  
        // TODO: this is non-elegant but required for GaudiSequencers
        // because we don't know which algorithms need to be reset 
        // very likely to execute the same algorithm multiple times
        // if it appears in multiple sequencers
        SmartIF<IAlgManager> algMan(serviceLocator());
        if (LIKELY(algMan.isValid())) {
          const std::list<IAlgorithm*>& allAlgs = algMan->getAlgorithms() ;
          for( IAlgorithm* ialg : allAlgs ) {
	    if (LIKELY(nullptr != ialg))ialg->resetExecuted();
	  }
        }
        eventContext->m_thread_id = pthread_self();
        this_algo->setContext(eventContext);
	// Call the execute() method
        try {
          RetCodeGuard rcg(appmgr, Gaudi::ReturnCode::UnhandledException);
          sc = ialgorithm->sysExecute();
          if (UNLIKELY(!sc.isSuccess()))  {
             warning() << "Execution of algorithm " << ialgorithm->name() << " failed" << endmsg;
             eventfailed = true;
          }
          rcg.ignore(); // disarm the guard
        } catch ( const GaudiException& Exception ) {
          error() << ".executeEvent(): Exception with tag=" << Exception.tag()
           << " thrown by " << ialgorithm->name() << endmsg;
          error() << Exception << endmsg;
        } catch ( const std::exception& Exception ) {
          fatal() << ".executeEvent(): Standard std::exception thrown by "
            << ialgorithm->name() << endmsg;
          error() <<  Exception.what()  << endmsg;
        } catch(...) {
          fatal() << ".executeEvent(): UNKNOWN Exception thrown by "
             << ialgorithm->name() << endmsg;
        }
        eventContext->m_evt_failed=eventfailed;      
      }
    }
  }
  for (EventContext* eventContext : eventContexts) {
    m_finishedEvents.push(eventContext);
  }

  return sc; //TODO: define proper return value
}

//---------------------------------------------------------------------------   
/// Blocks until an event is availble
StatusCode RoundRobinSchedulerSvc::popFinishedEvent(EventContext*& eventContext){
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

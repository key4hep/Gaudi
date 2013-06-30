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
#include "SequentialSchedulerSvc.h"
#include "AlgResourcePool.h"
#include "RetCodeGuard.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(SequentialSchedulerSvc)

//===========================================================================
// Infrastructure methods

SequentialSchedulerSvc::SequentialSchedulerSvc( const std::string& name, ISvcLocator* svcLoc ):
 base_class(name,svcLoc),
 m_eventContext(nullptr),
 m_freeSlots(1){ 
  declareProperty("UseTopAlgList", m_useTopAlgList=true);    
}

//---------------------------------------------------------------------------
SequentialSchedulerSvc::~SequentialSchedulerSvc(){}
//---------------------------------------------------------------------------

StatusCode SequentialSchedulerSvc::initialize(){

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
  
}
//---------------------------------------------------------------------------  

StatusCode SequentialSchedulerSvc::finalize(){
  StatusCode sc(Service::finalize());
  if (!sc.isSuccess())
    warning () << "Base class could not be finalized" << endmsg;     
  return sc;  
}

//---------------------------------------------------------------------------  

/** Make an event available to the scheduler. Immediately the algortihms are 
 * executed.
 */
StatusCode SequentialSchedulerSvc::pushNewEvent(EventContext* eventContext){

  m_freeSlots--;
  
  debug() << "[pushNewEvent] Free slots are now: " << m_freeSlots << endmsg;

  // Call the resetExecuted() method of ALL "known" algorithms
  // (From the MinimalEventLoopMgr)
  SmartIF<IAlgManager> algMan(serviceLocator());
  if (LIKELY(algMan.isValid())) {
    const std::list<IAlgorithm*>& allAlgs = algMan->getAlgorithms() ;
    for( IAlgorithm* ialg : allAlgs ) {
      if (LIKELY(nullptr != ialg))ialg->resetExecuted();
    }
  }
    
  m_eventContext= eventContext;  
  bool eventfailed=false;  
  
  for (IAlgorithm* ialgorithm : m_algList){
    
    Algorithm* this_algo = dynamic_cast<Algorithm*>(ialgorithm);  
    if (!this_algo){
      throw GaudiException ("Cast to Algorithm failed!",
                            "SequentialSchedulerSvc",
                            StatusCode::FAILURE);
    }

    m_eventContext->m_thread_id = pthread_self();  
    this_algo->setContext(m_eventContext);
    
    // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
    const SmartIF<IProperty> appmgr(serviceLocator());
      
    // Call the execute() method of all top algorithms
    StatusCode sc(StatusCode::FAILURE);
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

    // DP it is important to propagate the failure of an event.
    // We need to stop execution when this happens so that execute run can 
    // then receive the FAILURE
    m_eventContext->m_evt_failed=eventfailed;
    
    if (eventfailed)
      return StatusCode::FAILURE;    
  }  
  return StatusCode::SUCCESS;
  
}

//---------------------------------------------------------------------------
StatusCode SequentialSchedulerSvc::pushNewEvents(std::vector<EventContext*>& eventContexts){
  StatusCode sc;
  for (auto context : eventContexts){
    sc = pushNewEvent(context);
    if (sc != StatusCode::SUCCESS) return sc;
  }
  return sc;
}

//---------------------------------------------------------------------------   
/// Blocks until an event is availble
StatusCode SequentialSchedulerSvc::popFinishedEvent(EventContext*& eventContext){
  m_freeSlots++;
  eventContext = m_eventContext;
  debug() << "[popFinishedEvent] Free slots are now: " << m_freeSlots << endmsg;
  return StatusCode::SUCCESS;  
}
  
//---------------------------------------------------------------------------  
/** The scheduler is sequential. Therefore pop and try/pop are factually the 
 * same.
 */
StatusCode SequentialSchedulerSvc::tryPopFinishedEvent(EventContext*& eventContext){
  return m_freeSlots == 1 ? StatusCode::FAILURE : popFinishedEvent(eventContext);
}
//---------------------------------------------------------------------------  

/** Get free slots number. Given that the scheduler is sequential and its 
 * methods non reentrant, this is always 1.
 */
unsigned int SequentialSchedulerSvc::freeSlots(){return m_freeSlots;}

//---------------------------------------------------------------------------

// FW includes
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
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IIncidentListener.h"

#include "HistogramAgent.h"


#include "GaudiHive/HiveSlimEventLoopMgr.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/Algorithm.h"

#include <GaudiKernel/GaudiException.h>

#include <GaudiKernel/IScheduler.h>

// External libraries
#include "tbb/tick_count.h"


// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(HiveSlimEventLoopMgr)


#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
HiveSlimEventLoopMgr::HiveSlimEventLoopMgr(const std::string& name, ISvcLocator* svcLoc)
:  base_class(name, svcLoc), m_appMgrUI(svcLoc)
{
  m_histoDataMgrSvc   = 0;
  m_histoPersSvc      = 0;
  m_evtDataMgrSvc     = 0;
  m_evtDataSvc        = 0;
  m_evtSelector       = 0;
  m_evtContext        = 0;

  // Declare properties
  declareProperty("HistogramPersistency", m_histPersName = "");
  declareProperty("EvtSel", m_evtsel );
  declareProperty("Warnings",m_warnings=true,
      "Set this property to false to suppress warning messages");
  declareProperty("SchedulerName", m_schedulerName="ForwardSchedulerSvc",
                  "Name of the scheduler to be used");
  
  m_scheduledStop = false;
  
}

//--------------------------------------------------------------------------------------------
// Standard Destructor
//--------------------------------------------------------------------------------------------
HiveSlimEventLoopMgr::~HiveSlimEventLoopMgr()   {
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
StatusCode HiveSlimEventLoopMgr::initialize()    {
 
    if ( !m_appMgrUI.isValid() ) {
        return StatusCode::FAILURE;
    }

    StatusCode sc = Service::initialize();
    if ( !sc.isSuccess() )   {
        error() << "Failed to initialize Service Base class." << endmsg;
        return StatusCode::FAILURE;
    }

    // Get the references to the services that are needed by the ApplicationMgr itself
    m_incidentSvc = serviceLocator()->service("IncidentSvc");
    if( !m_incidentSvc.isValid() )  {
        fatal() << "Error retrieving IncidentSvc." << endmsg;
        return StatusCode::FAILURE;
    }

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
    m_whiteboard = serviceLocator()->service("EventDataSvc");
    if( !m_evtDataSvc.isValid() )  {
        fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;
        return StatusCode::FAILURE;
    }
    m_schedulerSvc = serviceLocator()->service("ForwardSchedulerSvc");
    if ( !m_schedulerSvc.isValid()){
        fatal() << "Error retrieving SchedulerSvc interface ISchedulerSvc." << endmsg;
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
    
    // Setup algorithm resource pool
    m_algResourcePool = serviceLocator()->service("AlgResourcePool");
    if( !m_algResourcePool.isValid() ) {
        fatal() << "Error retrieving AlgResourcePool" << endmsg;
        return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::reinitialize() {


  // Check to see whether a new Event Selector has been specified
  setProperty(m_appMgrProperty->getProperty("EvtSel"));
  if( m_evtsel != "NONE" || m_evtsel.length() == 0) {
    SmartIF<IService> theSvc(serviceLocator()->service("EventSelector"));
    SmartIF<IEvtSelector> theEvtSel(theSvc);
    StatusCode sc;
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
// Removed call to MinimalEventLoopMgr!
StatusCode HiveSlimEventLoopMgr::stop()    {
  if ( ! m_endEventFired ) {
    // Fire pending EndEvent incident
    m_incidentSvc->fireIncident(Incident(name(),IncidentType::EndEvent));
    m_endEventFired = true;
  }
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::finalize
//--------------------------------------------------------------------------------------------
// Removed call to MinimalEventLoopMgr!
StatusCode HiveSlimEventLoopMgr::finalize()    {
  
  StatusCode scRet;
  StatusCode sc = Service::finalize();

  if (sc.isFailure()) {
    scRet = StatusCode::FAILURE;
    error() << "Problems finalizing Service base class" << endmsg;
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

  scRet=sc;
  
  // Release event selector context
  if ( m_evtSelector && m_evtContext )   {
    m_evtSelector->releaseContext(m_evtContext).ignore();
    m_evtContext = 0;
  }

  m_incidentSvc = 0; // release
  m_appMgrUI = 0; // release
  
  // Release all interfaces...
  m_histoDataMgrSvc = 0;
  m_histoPersSvc    = 0;

  m_evtSelector     = 0;
  m_evtDataSvc      = 0;
  m_evtDataMgrSvc   = 0;

  return scRet;
}

//--------------------------------------------------------------------------------------------
// implementation of executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::executeEvent(void* createdEvts_IntPtr)    {

  // Leave the interface intact and swallow this C trick.
  int& createdEvts = *((int*)createdEvts_IntPtr);
  
  info() << "Beginning to process event " <<  createdEvts << endmsg;

  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  // DP have to find out who shoots this
/*  if ( m_scheduledStop ) {
    always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }*/      

  EventContext* evtContext(nullptr);
  
  if ( m_createEventContext(evtContext,createdEvts).isFailure() ){
    fatal() << "Impossible to create event context" << endmsg;
    return StatusCode::FAILURE;
  }

  StatusCode declEvtRootSc = m_declareEventRootAddress();
  if (declEvtRootSc.isFailure()) { // We ran out of events!
    createdEvts = -1;  // Set created event to a negative value: we finished!
    return StatusCode::SUCCESS;
    }

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident(Incident(name(),IncidentType::BeginEvent));    
    
  // Now add event to the scheduler 
  info() << "Adding event " << evtContext->m_evt_num 
          << ", slot " << evtContext->m_evt_slot
          << " to the scheduler" << endmsg;
  
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginProcessing));
  StatusCode addEventStatus = m_schedulerSvc->pushNewEvent(evtContext);

  // If this fails, we need to wait for something to complete
  if (!addEventStatus.isSuccess()){
      fatal() << "An event processing slot should be now free in the scheduler, but it appears not to be the case." << endmsg;
  }

  createdEvts++;
  return StatusCode::SUCCESS;

}

//--------------------------------------------------------------------------------------------
// implementation of IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::executeRun( int maxevt )    {
  StatusCode  sc;
  bool eventfailed = false;
  
  sc = m_algResourcePool->beginRun();
  if (sc.isFailure()) 
    eventfailed=true;

  // Call now the nextEvent(...)
  sc = nextEvent(maxevt);
  if (!sc.isSuccess())
    eventfailed = true;

  sc = m_algResourcePool->endRun();
  if (sc.isFailure())
    eventfailed=true;

  if (eventfailed) 
    return StatusCode::FAILURE;
  else 
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::stopRun()
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::stopRun() {
  // Set the application return code
  SmartIF<IProperty> appmgr(serviceLocator());
  if(Gaudi::setAppReturnCode(appmgr, Gaudi::ReturnCode::ScheduledStop).isFailure()) {
    error() << "Could not set return code of the application ("
            << Gaudi::ReturnCode::ScheduledStop << ")" << endmsg;
  }
  m_scheduledStop = true;
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
// Here the loop on the events takes place.
// This is also the natural place to put the preparation of the algorithms
// contexts, which contain the event specific data.

StatusCode HiveSlimEventLoopMgr::nextEvent(int maxevt)   {

  // Calculate runtime
  auto start_time = tbb::tick_count::now();
  auto secsFromStart = [&start_time]()->double{
    return (tbb::tick_count::now()-start_time).seconds();
  };

  // Reset the application return code.
  Gaudi::setAppReturnCode(m_appMgrProperty, Gaudi::ReturnCode::Success, true).ignore();  
  
  int finishedEvts =0;
  int createdEvts =0;
  info() << "Starting loop on events" << endmsg;
  // Loop until the finished events did not reach the maxevt number
  while (finishedEvts < maxevt){
    // if the created events did not reach maxevt, create an event    
    if (createdEvts >= 0 && // The events are not finished with an unlimited number of events
        createdEvts < maxevt &&  // The events are not finished with a limited number of events
        m_schedulerSvc->freeSlots()>0){ // There are still free slots in the scheduler

      debug() << "createdEvts: " << createdEvts << ", freeslots: " << m_schedulerSvc->freeSlots() << endmsg;
        
      StatusCode sc = executeEvent(&createdEvts);
      if (sc.isFailure())
        return StatusCode::FAILURE;

    } // end if condition createdEvts < maxevt
    else{ 
      // all the events were created but not all finished or the slots were 
      // all busy: the scheduler should finish its job

      debug() << "Draining the scheduler" << endmsg;

      // Pull out of the scheduler the finished events
      if (m_drainScheduler(finishedEvts).isFailure()){
        return StatusCode::FAILURE;     
      }
    }
  } // end main loop on finished events  

  info() << "---> Loop Finished (seconds): " << secsFromStart() <<endmsg;

  return StatusCode::SUCCESS;
  
}

//---------------------------------------------------------------------------

/// Create event address using event selector
StatusCode HiveSlimEventLoopMgr::getEventRoot(IOpaqueAddress*& refpAddr)  {
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

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::m_declareEventRootAddress(){
  
  StatusCode sc;
  if( m_evtContext ) {
      //---This is the "event iterator" context from EventSelector
      IOpaqueAddress* pAddr = 0;
      sc = getEventRoot(pAddr);
      if( !sc.isSuccess() )  {
          info() << "No more events in event selection " << endmsg;
          return StatusCode::FAILURE;
      }
      sc = m_evtDataMgrSvc->setRoot ("/Event", pAddr);
      if( !sc.isSuccess() )  {
          warning() << "Error declaring event root address." << endmsg;
      }
  }
  else {
      //---In case of no event selector----------------
      sc = m_evtDataMgrSvc->setRoot ("/Event", new DataObject());
      if( !sc.isSuccess() )  {
          warning() << "Error declaring event root DataObject" << endmsg;
      }
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode  HiveSlimEventLoopMgr::m_createEventContext(EventContext*& evtContext, int createdEvts){
  evtContext = new EventContext;

  evtContext->m_evt_num = createdEvts;
  evtContext->m_evt_slot = m_whiteboard->allocateStore(createdEvts);
  
  StatusCode sc = m_whiteboard->selectStore(evtContext->m_evt_slot);
  if (sc.isFailure()){
    warning() << "Slot " << evtContext->m_evt_slot 
              << " could not be selected for the WhiteBoard" << endmsg;
  }  
  return sc;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::m_drainScheduler(int& finishedEvts){

  // maybe we can do better
  std::vector<EventContext*> finishedEvtContexts;

  EventContext* finishedEvtContext(nullptr);

  // Here we wait not to loose cpu resources
  debug() << "Waiting for a context" << endmsg;
  m_schedulerSvc->popFinishedEvent(finishedEvtContext).ignore();
  debug() << "Context obtained" << endmsg;

  // We got past it: cache the pointer
  finishedEvtContexts.push_back(finishedEvtContext);

  // Let's see if we can pop other event contexts
  while (m_schedulerSvc->tryPopFinishedEvent(finishedEvtContext).isSuccess())
    finishedEvtContexts.push_back(finishedEvtContext);

  // Now we flush them
  for (auto& thisFinishedEvtContext : finishedEvtContexts){
    if (!thisFinishedEvtContext)
      fatal() << "Detected nullptr ctxt while clearing WB!"<< endmsg;

    if (thisFinishedEvtContext->m_evt_failed){
      fatal() << "Failed event detected"<< endmsg;
      delete thisFinishedEvtContext;
      return StatusCode::FAILURE;
    }
    
    info() << "Clearing slot " << thisFinishedEvtContext->m_evt_slot 
          << " (event " << thisFinishedEvtContext->m_evt_num
          << ") of the whiteboard" << endmsg;
    
    StatusCode sc = m_clearWBSlot(thisFinishedEvtContext->m_evt_slot);
    if (!sc.isSuccess())
        error() << "Whiteboard slot " << thisFinishedEvtContext->m_evt_slot 
                << " could not be properly cleared";

    
    delete thisFinishedEvtContext;
    
    finishedEvts++;
    
    m_incidentSvc->fireIncident(Incident(name(), IncidentType::EndProcessing));
    
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::m_clearWBSlot(int evtSlot)  {
  StatusCode sc = m_whiteboard->clearStore(evtSlot);
  if( !sc.isSuccess() )  {
    warning() << "Clear of Event data store failed" << endmsg;    
  }
  return m_whiteboard->freeStore(evtSlot);  
}
//---------------------------------------------------------------------------











#define  GAUDISVC_EVENTLOOPMGR_CPP

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/AppReturnCode.h"

#include "HistogramAgent.h"
#include "EventLoopMgr.h"


// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT(EventLoopMgr)


#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
EventLoopMgr::EventLoopMgr(const std::string& nam, ISvcLocator* svcLoc)
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
EventLoopMgr::~EventLoopMgr()   {
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
StatusCode EventLoopMgr::initialize()    {
  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::initialize();
  if( !sc.isSuccess() ) {
    DEBMSG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
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

  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode EventLoopMgr::reinitialize() {

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
StatusCode EventLoopMgr::stop()    {
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
StatusCode EventLoopMgr::finalize()    {
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

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode EventLoopMgr::executeEvent(void* par)    {

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident(Incident(name(),IncidentType::BeginEvent));
  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  if ( m_scheduledStop ) {
    always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute Algorithms
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::BeginProcessing));
  StatusCode sc = MinimalEventLoopMgr::executeEvent(par);
  m_incidentSvc->fireIncident(Incident(name(), IncidentType::EndProcessing));

  // Check if there was an error processing current event
  if( UNLIKELY(!sc.isSuccess()) ){
    error() << "Terminating event processing loop due to errors" << endmsg;
  }
  return sc;
}

//--------------------------------------------------------------------------------------------
// IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode EventLoopMgr::executeRun( int maxevt )    {
  StatusCode  sc;
  // initialize the base class
  sc = MinimalEventLoopMgr::executeRun(maxevt);
  return sc;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
StatusCode EventLoopMgr::nextEvent(int maxevt)   {
  static int        total_nevt = 0;
  DataObject*       pObject = 0;
  StatusCode        sc(StatusCode::SUCCESS, true);

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
StatusCode EventLoopMgr::getEventRoot(IOpaqueAddress*& refpAddr)  {
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

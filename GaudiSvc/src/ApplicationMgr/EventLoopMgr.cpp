// $Id: EventLoopMgr.cpp,v 1.26 2008/10/09 13:40:18 marcocle Exp $
#define  GAUDISVC_EVENTLOOPMGR_CPP

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IConversionSvc.h"

#include "HistogramAgent.h"
#include "EventLoopMgr.h"

#include "GaudiKernel/WatchdogThread.h"

#include <csignal>

namespace {
  /// Small (singleton) helper class to keep track of occurred signals.
  /// The signals to be recorded must be declared with monitorSignal() and the
  /// check can be disabled with ignoreSignal().
  class SignalHandler {
  public:
#ifdef _WIN32
    typedef void (__cdecl *handler_t)(int);
#else
    typedef struct sigaction handler_t;
#endif

    /// Method to get the singleton instance.
    static SignalHandler &instance() {
      static SignalHandler sh;
      return sh;
    }

    /// Declare a signal to be monitored.
    /// It installs a signal handler for the requested signal.
    void monitorSignal(int signum) {
      if (!m_monitored[signum]) {
        handler_t sa;
        handler_t oldact;
#ifdef _WIN32
        sa = SignalHandler::dispatcher;
	oldact = signal(signum, sa);
#else
        sa.sa_handler = SignalHandler::dispatcher;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(signum, &sa, &oldact);
#endif
	m_oldActions[signum] = oldact;
        m_monitored[signum] = true;
      }
    }

    /// Remove the specific signal handler for the requested signal, restoring
    /// the previous signal handler.
    void ignoreSignal(int signum) {
      if (m_monitored[signum]) {
#ifdef _WIN32
        (void) signal(signum, m_oldActions[signum]);
#else
        sigaction(signum, &m_oldActions[signum], 0);
#endif
        m_oldActions[signum] = m_defaultAction;
        m_monitored[signum] = false;
      }
    }

    /// Check if the given signal has been received.
    bool gotSignal(int signum) const {
      return m_caught[signum] != 0;
    }

    /// Set the flag for the given signal, as if the signal was received.
    void setSignal(int signum) {
      m_caught[signum] = 1;
    }

    /// Clear the flag for the given signal, so that a new occurrence can be identified.
    void clearSignal(int signum) {
      m_caught[signum] = 0;
    }

  private:
    /// Constructor.
    SignalHandler(){
#ifdef _WIN32
      m_defaultAction = SIG_DFL;
#else
      m_defaultAction.sa_handler = SIG_DFL;
      sigemptyset(&m_defaultAction.sa_mask);
      m_defaultAction.sa_flags = 0;
#endif
      for(int i = 0; i < NSIG; ++i){
        m_caught[i] = 0;
        m_monitored[i] = false;
        m_oldActions[i] = m_defaultAction;
      }
    }

    /// Array of flags to keep track of monitored signals.
    bool             m_monitored[NSIG];
    /// Array of flags for received signals.
    sig_atomic_t     m_caught[NSIG];
    /// Helper variable for default signal action.
    handler_t        m_defaultAction;
    /// List of replaced signal actions (for the recovery when disable the monitoring).
    handler_t        m_oldActions[NSIG];

    /// Signal handler function.
    static void dispatcher(int signum);
  };

  // Implementation of the signal handler function.
  void SignalHandler::dispatcher(int signum){
    instance().m_caught[signum] = 1;
  }

  /// Specialized watchdog to monitor the event loop and spot possible infinite loops.
  class EventWatchdog: public WatchdogThread {
  public:
    EventWatchdog(const SmartIF<IMessageSvc> &msgSvc,
                  const std::string &name,
                  boost::posix_time::time_duration timeout,
                  bool autostart = false):
        WatchdogThread(timeout, autostart),
        log(msgSvc, name),
        m_counter(0) {}
    virtual ~EventWatchdog() {}
  private:
    MsgStream log;
    long m_counter;
    void action() {
      if (!m_counter) {
        log << MSG::WARNING << "More than " << getTimeout().seconds()
            << "s to process an event." << endmsg;
      }
      else if (m_counter < 2) {
        log << MSG::WARNING << "Other " << getTimeout().seconds()
            << "s and we are still on the same event." << endmsg;
      }
      else if (m_counter < 3) {
        log << MSG::WARNING << "We are still at the same point:" << endmsg;
        // log << MSG::WARNING << "*** stack trace ***" << endmsg;
        log << MSG::WARNING << "I'm not going to print other messages for this event." << endmsg;
      }
      ++m_counter;
    }
    void onPing() {
      if (m_counter) {
        if (m_counter >= 3)
          log << MSG::INFO << "Starting a new event after ~"
              << m_counter * getTimeout().seconds() << "s" << endmsg;
        m_counter = 0;
      }
    }
    void onStop() {
      if (m_counter >= 3)
        log << MSG::INFO << "The last event took ~"
        << m_counter * getTimeout().seconds() << "s" << endmsg;
    }
  };
}


// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(EventLoopMgr)

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

  declareProperty("HandleSIGINT", m_handleSIGINT = true,
                  "Intercept the SIGINT signal (CTRL-C) and stop the event loop.");
  declareProperty("HandleSIGXCPU", m_handleSIGXCPU = true,
                  "Intercept the SIGXCPU signal and stop the event loop (ignored on Windows).");
  declareProperty("EventTimeout", m_eventTimeout = 600,
                  "Number of seconds allowed to process a single event (0 to disable the check)");
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
  MsgStream log(msgSvc(), name());
  if( !sc.isSuccess() ) {
    log << MSG::DEBUG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }

  // Setup access to event data services
  m_evtDataMgrSvc = serviceLocator()->service("EventDataSvc");
  if( !m_evtDataMgrSvc.isValid() )  {
    log << MSG::FATAL << "Error retrieving EventDataSvc interface IDataManagerSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  m_evtDataSvc = serviceLocator()->service("EventDataSvc");
  if( !m_evtDataSvc.isValid() )  {
    log << MSG::FATAL << "Error retrieving EventDataSvc interface IDataProviderSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  // Obtain the IProperty of the ApplicationMgr
  SmartIF<IProperty> prpMgr(serviceLocator());
  if ( ! prpMgr.isValid() )   {
    log << MSG::FATAL << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  }
  else {
    m_appMgrProperty = prpMgr;
  }

  // We do not expect a Event Selector necessarily being declared
  setProperty(m_appMgrProperty->getProperty("EvtSel")).ignore();

  if( m_evtsel != "NONE" || m_evtsel.length() == 0) {
    m_evtSelector = serviceLocator()->service("EventSelector");
    if( m_evtSelector.isValid() ) {
      // Setup Event Selector
      sc=m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() )   {
        log << MSG::FATAL << "Can not create the event selector Context." << endmsg;
        return sc;
      }
    }
    else {
      log << MSG::FATAL << "EventSelector not found." << endmsg;
      return sc;
    }
  }
  else {
    m_evtSelector = 0;
    m_evtContext = 0;
    if ( m_warnings ) {
      log << MSG::WARNING << "Unable to locate service \"EventSelector\" " << endmsg;
      log << MSG::WARNING << "No events will be processed from external input." << endmsg;
    }
  }

  // Setup access to histogramming services
  m_histoDataMgrSvc = serviceLocator()->service("HistogramDataSvc");
  if( !m_histoDataMgrSvc.isValid() )  {
    log << MSG::FATAL << "Error retrieving HistogramDataSvc." << endmsg;
    return sc;
  }
  // Setup histogram persistency
  m_histoPersSvc = serviceLocator()->service("HistogramPersistencySvc");
  if( !m_histoPersSvc.isValid() ) {
    log << MSG::WARNING << "Histograms cannot not be saved - though required." << endmsg;
    return sc;
  }

  if (m_eventTimeout) {
    m_watchdog = std::auto_ptr<WatchdogThread>(
        new EventWatchdog(msgSvc(),
            "EventWatchdog",
            boost::posix_time::seconds(m_eventTimeout)));
  }

  if (m_handleSIGINT) SignalHandler::instance().monitorSignal(SIGINT);
#ifndef _WIN32
  if (m_handleSIGXCPU) SignalHandler::instance().monitorSignal(SIGXCPU);
#endif
  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode EventLoopMgr::reinitialize() {
  MsgStream log(msgSvc(), name());

  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::reinitialize();
  if( !sc.isSuccess() ) {
    log << MSG::DEBUG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }

  // Check to see whether a new Event Selector has been specified
  setProperty(m_appMgrProperty->getProperty("EvtSel"));
  if( m_evtsel != "NONE" || m_evtsel.length() == 0) {
    SmartIF<IService> theSvc(serviceLocator()->service("EventSelector"));
    SmartIF<IEvtSelector> theEvtSel(theSvc);
    if( theEvtSel.isValid() && ( theEvtSel.get() != m_evtSelector.get() ) ) {
      // Setup Event Selector
      m_evtSelector = theEvtSel;
      if (theSvc->FSMState() == Gaudi::StateMachine::INITIALIZED) {
        sc = theSvc->reinitialize();
        if( !sc.isSuccess() ) {
          log << MSG::ERROR << "Failure Reinitializing EventSelector "
              << theSvc->name( ) << endmsg;
          return sc;
        }
      }
      else {
        sc = theSvc->sysInitialize();
        if( !sc.isSuccess() ) {
          log << MSG::ERROR << "Failure Initializing EventSelector "
              << theSvc->name( ) << endmsg;
          return sc;
        }
      }
      sc = m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() ) {
        log << MSG::ERROR << "Can not create Context "
            << theSvc->name( ) << endmsg;
        return sc;
      }
      log << MSG::INFO << "EventSelector service changed to "
          << theSvc->name( ) << endmsg;
    }
    else if ( m_evtSelector.isValid() && m_evtContext ) {
      m_evtSelector->releaseContext(m_evtContext);
      m_evtContext = 0;
      sc = m_evtSelector->createContext(m_evtContext);
      if( !sc.isSuccess() ) {
        log << MSG::ERROR << "Can not create Context "
            << theSvc->name( ) << endmsg;
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
  MsgStream log(msgSvc(), name());

  // Finalize base class
  sc = MinimalEventLoopMgr::finalize();
  if (! sc.isSuccess()) {
    log << MSG::ERROR << "Error finalizing base class" << endmsg;
    return sc;
  }

  if (m_handleSIGINT) SignalHandler::instance().ignoreSignal(SIGINT);
#ifndef _WIN32
  if (m_handleSIGXCPU) SignalHandler::instance().ignoreSignal(SIGXCPU);
#endif

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
        log << MSG::INFO << "Histograms converted successfully according to request." << endmsg;
      }
      else  {
        log << MSG::ERROR << "Error while saving Histograms." << endmsg;
      }
    }
    else {
      log << MSG::ERROR << "Error while traversing Histogram data store" << endmsg;
    }
  }

  // Release evemt selector context
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
    MsgStream  log( msgSvc(), name() );
    log << MSG::ALWAYS << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute Algorithms
  StatusCode sc = MinimalEventLoopMgr::executeEvent(par);

  // Check if there was an error processing current event
  if( !sc.isSuccess() ){
    MsgStream log( msgSvc(), name() );
    log << MSG::ERROR << "Terminating event processing loop due to errors" << endmsg;
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
  MsgStream         log( msgSvc(), name() );

  if (m_watchdog.get()) m_watchdog->start();

  // loop over events if the maxevt (received as input) if different from -1.
  // if evtmax is -1 it means infinite loop
  for( int nevt = 0; (maxevt == -1 ? true : nevt < maxevt);  nevt++, total_nevt++) {
    if (m_watchdog.get()) m_watchdog->ping();

    if (
        SignalHandler::instance().gotSignal(SIGINT)
#ifndef _WIN32		    
        or SignalHandler::instance().gotSignal(SIGXCPU)
#endif
        ){
      log << MSG::ALWAYS << "Stop of event loop after receiving a ";
      if (SignalHandler::instance().gotSignal(SIGINT)) {
        log << "SIGINT";
      }
#ifndef _WIN32
      else {
        log << "SIGXCPU";
      }
#endif
      log << " signal" << endmsg;
      // Clear the signal flags
      if (SignalHandler::instance().gotSignal(SIGINT))
        SignalHandler::instance().clearSignal(SIGINT);
#ifndef _WIN32
      if (SignalHandler::instance().gotSignal(SIGXCPU))
        SignalHandler::instance().clearSignal(SIGXCPU);
#endif
      break;
    }

    // Check if there is a scheduled stop issued by some algorithm/service
    if ( m_scheduledStop ) {
      m_scheduledStop = false;
      log << MSG::ALWAYS << "Terminating event processing loop due to scheduled stop" << endmsg;
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
        log << MSG::DEBUG << "Clear of Event data store failed" << endmsg;
      }
    }

    // Setup event in the event store
    if( m_evtContext ) {
      IOpaqueAddress* addr = 0;
      // Only if there is a EventSelector
      sc = getEventRoot(addr);
      if( !sc.isSuccess() )  {
        log << MSG::INFO << "No more events in event selection " << endmsg;
        break;
      }
      // Set root clears the event data store first
      sc = m_evtDataMgrSvc->setRoot ("/Event", addr);
      if( !sc.isSuccess() )  {
        log << MSG::WARNING << "Error declaring event root address." << endmsg;
        continue;
      }
      sc = m_evtDataSvc->retrieveObject("/Event", pObject);
      if( !sc.isSuccess() ) {
        log << MSG::WARNING << "Unable to retrieve Event root object" << endmsg;
        break;
      }
    }
    else {
      sc = m_evtDataMgrSvc->setRoot ("/Event", new DataObject());
      if( !sc.isSuccess() )  {
        log << MSG::WARNING << "Error declaring event root DataObject" << endmsg;
      }
    }
    // Execute event for all required algorithms
    sc = executeEvent(NULL);
    m_endEventFired = false;
    if( !sc.isSuccess() ){
      log << MSG::ERROR << "Terminating event processing loop due to errors" << endmsg;
      break;
    }
  }
  if (m_watchdog.get()) m_watchdog->stop();
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
        MsgStream log( msgSvc(), name() );
        log << MSG::WARNING << "Error creating IOpaqueAddress." << endmsg;
      }
    }
  }
  return sc;
}

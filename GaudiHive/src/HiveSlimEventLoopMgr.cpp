// FW includes
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SvcFactory.h"

#include "HistogramAgent.h"

#include "GaudiHive/HiveSlimEventLoopMgr.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/EventContext.h"

#include <GaudiKernel/GaudiException.h>

#include <GaudiKernel/IScheduler.h>

// External libraries
#include <chrono>

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY( HiveSlimEventLoopMgr )

#define ON_DEBUG if ( UNLIKELY( outputLevel() <= MSG::DEBUG ) )
#define ON_VERBOSE if ( UNLIKELY( outputLevel() <= MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
HiveSlimEventLoopMgr::HiveSlimEventLoopMgr( const std::string& name, ISvcLocator* svcLoc )
    : base_class( name, svcLoc ), m_appMgrUI( svcLoc )
{
}

//--------------------------------------------------------------------------------------------
// Standard Destructor
//--------------------------------------------------------------------------------------------
HiveSlimEventLoopMgr::~HiveSlimEventLoopMgr()
{
  m_histoDataMgrSvc.reset();
  m_histoPersSvc.reset();
  m_evtDataMgrSvc.reset();
  m_whiteboard.reset();
  m_evtSelector.reset();
  delete m_evtContext;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::initialize
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::initialize()
{

  if ( !m_appMgrUI ) return StatusCode::FAILURE;

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) {
    error() << "Failed to initialize Service Base class." << endmsg;
    return StatusCode::FAILURE;
  }

  // Get the references to the services that are needed by the ApplicationMgr itself
  m_incidentSvc = serviceLocator()->service( "IncidentSvc" );
  if ( !m_incidentSvc ) {
    fatal() << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  // Setup access to event data services
  m_evtDataMgrSvc = serviceLocator()->service( "EventDataSvc" );
  if ( !m_evtDataMgrSvc ) {
    fatal() << "Error retrieving EventDataSvc interface IDataManagerSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  m_whiteboard = serviceLocator()->service( "EventDataSvc" );
  if ( !m_whiteboard ) {
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;
    return StatusCode::FAILURE;
  }
  m_schedulerSvc = serviceLocator()->service( m_schedulerName );
  if ( !m_schedulerSvc ) {
    fatal() << "Error retrieving SchedulerSvc interface IScheduler." << endmsg;
    return StatusCode::FAILURE;
  }
  // Obtain the IProperty of the ApplicationMgr
  m_appMgrProperty = serviceLocator();
  if ( !m_appMgrProperty ) {
    fatal() << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  }

  // We do not expect a Event Selector necessarily being declared
  setProperty( m_appMgrProperty->getProperty( "EvtSel" ) ).ignore();

  if ( m_evtsel != "NONE" || m_evtsel.length() == 0 ) {
    m_evtSelector = serviceLocator()->service( "EventSelector" );
    if ( m_evtSelector ) {
      // Setup Event Selector
      sc = m_evtSelector->createContext( m_evtContext );
      if ( !sc.isSuccess() ) {
        fatal() << "Can not create the event selector Context." << endmsg;
        return sc;
      }
    } else {
      fatal() << "EventSelector not found." << endmsg;
      return sc;
    }
  } else {
    m_evtSelector = 0;
    m_evtContext  = 0;
    if ( m_warnings ) {
      warning() << "Unable to locate service \"EventSelector\" " << endmsg;
      warning() << "No events will be processed from external input." << endmsg;
    }
  }

  // Setup access to histogramming services
  m_histoDataMgrSvc = serviceLocator()->service( "HistogramDataSvc" );
  if ( !m_histoDataMgrSvc ) {
    fatal() << "Error retrieving HistogramDataSvc." << endmsg;
    return sc;
  }
  // Setup histogram persistency
  m_histoPersSvc = serviceLocator()->service( "HistogramPersistencySvc" );
  if ( !m_histoPersSvc ) {
    warning() << "Histograms cannot not be saved - though required." << endmsg;
    return sc;
  }

  // Setup algorithm resource pool
  m_algResourcePool = serviceLocator()->service( "AlgResourcePool" );
  if ( !m_algResourcePool ) {
    fatal() << "Error retrieving AlgResourcePool" << endmsg;
    return StatusCode::FAILURE;
  }

  m_algExecStateSvc = serviceLocator()->service( "AlgExecStateSvc" );
  if ( !m_algExecStateSvc ) {
    fatal() << "Error retrieving AlgExecStateSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  std::sort( m_eventNumberBlacklist.begin(), m_eventNumberBlacklist.end() );
  info() << "Found " << m_eventNumberBlacklist.size() << " events in black list" << endmsg;

  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::reinitialize()
{

  // Check to see whether a new Event Selector has been specified
  setProperty( m_appMgrProperty->getProperty( "EvtSel" ) );
  if ( m_evtsel != "NONE" || m_evtsel.empty() ) {
    auto theSvc    = serviceLocator()->service( "EventSelector" );
    auto theEvtSel = theSvc.as<IEvtSelector>();
    StatusCode sc;
    if ( theEvtSel && ( theEvtSel.get() != m_evtSelector.get() ) ) {
      // Setup Event Selector
      if ( m_evtSelector.get() && m_evtContext ) {
        // Need to release context before switching to new event selector
        m_evtSelector->releaseContext( m_evtContext );
        m_evtContext = nullptr;
      }
      m_evtSelector = theEvtSel;
      if ( theSvc->FSMState() == Gaudi::StateMachine::INITIALIZED ) {
        sc = theSvc->reinitialize();
        if ( !sc.isSuccess() ) {
          error() << "Failure Reinitializing EventSelector " << theSvc->name() << endmsg;
          return sc;
        }
      } else {
        sc = theSvc->sysInitialize();
        if ( !sc.isSuccess() ) {
          error() << "Failure Initializing EventSelector " << theSvc->name() << endmsg;
          return sc;
        }
      }
      sc = m_evtSelector->createContext( m_evtContext );
      if ( !sc.isSuccess() ) {
        error() << "Can not create Context " << theSvc->name() << endmsg;
        return sc;
      }

    } else if ( m_evtSelector ) {
      if ( m_evtContext ) {
        m_evtSelector->releaseContext( m_evtContext );
        m_evtContext = nullptr;
      }
      sc = m_evtSelector->createContext( m_evtContext );
      if ( !sc.isSuccess() ) {
        error() << "Can not create Context " << theSvc->name() << endmsg;
        return sc;
      }
    }
  } else if ( m_evtSelector && m_evtContext ) {
    m_evtSelector->releaseContext( m_evtContext );
    m_evtSelector = nullptr;
    m_evtContext  = nullptr;
  }
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of IService::stop
//--------------------------------------------------------------------------------------------
// Removed call to MinimalEventLoopMgr!
StatusCode HiveSlimEventLoopMgr::stop()
{
  if ( !m_endEventFired ) {
    // Fire pending EndEvent incident
    m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndEvent ) );
    m_endEventFired = true;
  }
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::finalize
//--------------------------------------------------------------------------------------------
// Removed call to MinimalEventLoopMgr!
StatusCode HiveSlimEventLoopMgr::finalize()
{

  StatusCode scRet;
  StatusCode sc = Service::finalize();

  if ( sc.isFailure() ) {
    scRet = StatusCode::FAILURE;
    error() << "Problems finalizing Service base class" << endmsg;
  }

  // Save Histograms Now
  if ( m_histoPersSvc ) {
    HistogramAgent agent;
    sc = m_histoDataMgrSvc->traverseTree( &agent );
    if ( sc.isSuccess() ) {
      const IDataSelector& objects = agent.selectedObjects();
      // skip /stat entry!
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode s, const auto& i ) {
        IOpaqueAddress* pAddr = nullptr;
        StatusCode iret       = m_histoPersSvc->createRep( i, pAddr );
        if ( iret.isSuccess() ) i->registry()->setAddress( pAddr );
        return s.isFailure() ? s : iret;
      } );
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode s, const auto& i ) {
        IRegistry* reg  = i->registry();
        StatusCode iret = m_histoPersSvc->fillRepRefs( reg->address(), i );
        return s.isFailure() ? s : iret;
      } );
      if ( sc.isSuccess() ) {
        info() << "Histograms converted successfully according to request." << endmsg;
      } else {
        error() << "Error while saving Histograms." << endmsg;
      }
    } else {
      error() << "Error while traversing Histogram data store" << endmsg;
    }
  }

  scRet = sc;

  // Release event selector context
  if ( m_evtSelector && m_evtContext ) {
    m_evtSelector->releaseContext( m_evtContext ).ignore();
    m_evtContext = nullptr;
  }

  m_incidentSvc.reset();
  m_appMgrUI.reset();

  // Release all interfaces...
  m_histoDataMgrSvc.reset();
  m_histoPersSvc.reset();

  m_evtSelector.reset();
  m_whiteboard.reset();
  m_evtDataMgrSvc.reset();

  return scRet;
}

//--------------------------------------------------------------------------------------------
// implementation of executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::executeEvent( void* createdEvts_IntPtr )
{

  // Leave the interface intact and swallow this C trick.
  int& createdEvts = *( (int*)createdEvts_IntPtr );

  EventContext* evtContext( nullptr );

  // Check if event number is in blacklist
  if ( LIKELY( m_blackListBS != nullptr ) ) { // we are processing a finite number of events, use bitset blacklist
    if ( m_blackListBS->test( createdEvts ) ) {
      verbose() << "Event " << createdEvts << " on black list" << endmsg;
      return StatusCode::RECOVERABLE;
    }
  } else if ( std::binary_search( m_eventNumberBlacklist.begin(), m_eventNumberBlacklist.end(), createdEvts ) ) {

    verbose() << "Event " << createdEvts << " on black list" << endmsg;
    return StatusCode::RECOVERABLE;
  }

  if ( createEventContext( evtContext, createdEvts ).isFailure() ) {
    fatal() << "Impossible to create event context" << endmsg;
    return StatusCode::FAILURE;
  }

  verbose() << "Beginning to process event " << createdEvts << endmsg;

  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  // DP have to find out who shoots this
  /*  if ( m_scheduledStop ) {
      always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
      return StatusCode::SUCCESS;
      }*/

  StatusCode declEvtRootSc = declareEventRootAddress();
  if ( declEvtRootSc.isFailure() ) { // We ran out of events!
    createdEvts = -1;                // Set created event to a negative value: we finished!
    return StatusCode::SUCCESS;
  }

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident( std::make_unique<Incident>( name(), IncidentType::BeginEvent, *evtContext ) );

  // Now add event to the scheduler
  verbose() << "Adding event " << evtContext->evt() << ", slot " << evtContext->slot() << " to the scheduler" << endmsg;

  m_incidentSvc->fireIncident( std::make_unique<Incident>( name(), IncidentType::BeginProcessing, *evtContext ) );

  StatusCode addEventStatus = m_schedulerSvc->pushNewEvent( evtContext );

  // If this fails, we need to wait for something to complete
  if ( !addEventStatus.isSuccess() ) {
    fatal() << "An event processing slot should be now free in the scheduler, but it appears not to be the case."
            << endmsg;
  }

  createdEvts++;
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// implementation of IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::executeRun( int maxevt )
{
  StatusCode sc;
  bool eventfailed = false;

  if ( maxevt > 0 ) { // finite number of events to process
    const unsigned int umaxevt = static_cast<unsigned int>( maxevt );
    m_blackListBS              = new boost::dynamic_bitset<>( maxevt ); // all initialized to zero
    for ( unsigned int i = 0; i < m_eventNumberBlacklist.size() && m_eventNumberBlacklist[i] <= umaxevt;
          ++i ) { // black list is sorted in init
      m_blackListBS->set( m_eventNumberBlacklist[i], true );
    }
  }

  // Call now the nextEvent(...)
  sc                                = nextEvent( maxevt );
  if ( sc.isFailure() ) eventfailed = true;

  delete m_blackListBS;

  return eventfailed ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::stopRun()
//--------------------------------------------------------------------------------------------
StatusCode HiveSlimEventLoopMgr::stopRun()
{
  // Set the application return code
  auto appmgr = serviceLocator()->as<IProperty>();
  if ( Gaudi::setAppReturnCode( appmgr, Gaudi::ReturnCode::ScheduledStop ).isFailure() ) {
    error() << "Could not set return code of the application (" << Gaudi::ReturnCode::ScheduledStop << ")" << endmsg;
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
#include "GaudiKernel/Memory.h"
StatusCode HiveSlimEventLoopMgr::nextEvent( int maxevt )
{

  // Calculate runtime
  using Clock = std::chrono::high_resolution_clock;

  // Reset the application return code.
  Gaudi::setAppReturnCode( m_appMgrProperty, Gaudi::ReturnCode::Success, true ).ignore();

  int finishedEvts = 0;
  int createdEvts  = 0;
  int skippedEvts  = 0;
  info() << "Starting loop on events" << endmsg;
  // Loop until the finished events did not reach the maxevt number
  bool loop_ended = false;
  // Run the first event before spilling more than one
  bool newEvtAllowed = false;

  constexpr double oneOver1024 = 1. / 1024.;

  uint iteration  = 0;
  auto start_time = Clock::now();
  while ( !loop_ended && ( maxevt < 0 || ( finishedEvts + skippedEvts ) < maxevt ) ) {
    debug() << "work loop iteration " << iteration++ << endmsg;
    // if the created events did not reach maxevt, create an event
    if ( ( newEvtAllowed || createdEvts == 0 ) &&  // Launch the first event alone
         createdEvts >= 0 &&                       // The events are not finished with an unlimited number of events
         ( createdEvts < maxevt || maxevt < 0 ) && // The events are not finished with a limited number of events
         m_schedulerSvc->freeSlots() > 0 &&        // There are still free slots in the scheduler
         m_whiteboard->freeSlots() > 0 ) {         // There are still free slots in the whiteboard

      if ( 1 == createdEvts ) // reset counter to count from event 1
        start_time = Clock::now();

      debug() << "createdEvts: " << createdEvts << ", freeslots: " << m_schedulerSvc->freeSlots() << endmsg;
      //  DP remove to remove the syscalls...
      //      if (0!=createdEvts){
      //        info()   << "Event Number = " << createdEvts
      //                 << " WSS (MB) = " << System::mappedMemory(System::MemoryUnit::kByte)*oneOver1024
      //                << " Time (s) = " << secsFromStart(start_time) << endmsg;
      //        }

      // TODO can we adapt the interface of executeEvent for a nicer solution?
      StatusCode sc = StatusCode::RECOVERABLE;
      while ( !sc.isSuccess()                               // we haven't created an event yet
              && ( createdEvts < maxevt || maxevt < 0 ) ) { // redunant check for maxEvts, can we do better?
        sc = executeEvent( &createdEvts );

        if ( sc.isRecoverable() ) { // we skipped an event

          // this is all to skip the event
          size_t slot =
              m_whiteboard->allocateStore( createdEvts ); // we need a new store, not to change the previous event
          m_whiteboard->selectStore( slot );
          declareEventRootAddress();       // actually skip over the event
          m_whiteboard->freeStore( slot ); // delete the store

          ++createdEvts;
          ++skippedEvts;
        } else if ( sc.isRecoverable() ) { // exit immediatly
          return StatusCode::FAILURE;
        } // else we have an success --> exit loop
      }

    } // end if condition createdEvts < maxevt
    else {
      // all the events were created but not all finished or the slots were
      // all busy: the scheduler should finish its job

      debug() << "Draining the scheduler" << endmsg;

      // Pull out of the scheduler the finished events
      if ( drainScheduler( finishedEvts ).isFailure() ) loop_ended = true;
      newEvtAllowed                                                = true;
    }
  } // end main loop on finished events
  auto end_time = Clock::now();

  info() << "---> Loop Finished (skipping 1st evt) - "
         << " WSS " << System::mappedMemory( System::MemoryUnit::kByte ) * oneOver1024 << " total time "
         << std::chrono::duration_cast<std::chrono::nanoseconds>( end_time - start_time ).count() << endmsg;
  info() << skippedEvts << " events were SKIPed" << endmsg;

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

/// Create event address using event selector
StatusCode HiveSlimEventLoopMgr::getEventRoot( IOpaqueAddress*& refpAddr )
{
  refpAddr      = nullptr;
  StatusCode sc = m_evtSelector->next( *m_evtContext );
  if ( sc.isFailure() ) return sc;
  // Create root address and assign address to data service
  sc = m_evtSelector->createAddress( *m_evtContext, refpAddr );
  if ( sc.isSuccess() ) return sc;
  sc = m_evtSelector->next( *m_evtContext );
  if ( sc.isFailure() ) return sc;
  sc = m_evtSelector->createAddress( *m_evtContext, refpAddr );
  if ( !sc.isSuccess() ) warning() << "Error creating IOpaqueAddress." << endmsg;
  return sc;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::declareEventRootAddress()
{
  StatusCode sc;
  if ( m_evtContext ) {
    //---This is the "event iterator" context from EventSelector
    IOpaqueAddress* pAddr = nullptr;
    sc                    = getEventRoot( pAddr );
    if ( !sc.isSuccess() ) {
      info() << "No more events in event selection " << endmsg;
      return StatusCode::FAILURE;
    }
    sc = m_evtDataMgrSvc->setRoot( "/Event", pAddr );
    if ( !sc.isSuccess() ) {
      warning() << "Error declaring event root address." << endmsg;
    }
  } else {
    //---In case of no event selector----------------
    sc = m_evtDataMgrSvc->setRoot( "/Event", new DataObject() );
    if ( !sc.isSuccess() ) {
      warning() << "Error declaring event root DataObject" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::createEventContext( EventContext*& evtContext, int createdEvts )
{
  evtContext = new EventContext;
  evtContext->set( createdEvts, m_whiteboard->allocateStore( createdEvts ) );
  m_algExecStateSvc->reset( *evtContext );

  StatusCode sc = m_whiteboard->selectStore( evtContext->slot() );
  if ( sc.isFailure() ) {
    warning() << "Slot " << evtContext->slot() << " could not be selected for the WhiteBoard" << endmsg;
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::drainScheduler( int& finishedEvts )
{

  StatusCode sc( StatusCode::SUCCESS );

  // maybe we can do better
  std::vector<EventContext*> finishedEvtContexts;

  EventContext* finishedEvtContext( nullptr );

  // Here we wait not to loose cpu resources
  debug() << "Waiting for a context" << endmsg;
  sc = m_schedulerSvc->popFinishedEvent( finishedEvtContext );

  // We got past it: cache the pointer
  if ( sc.isSuccess() ) {
    debug() << "Context obtained" << endmsg;
  } else {
    // A problem occurred.
    debug() << "Context not obtained: a problem in the scheduling?" << endmsg;
    //     return StatusCode::FAILURE;
  }

  finishedEvtContexts.push_back( finishedEvtContext );

  // Let's see if we can pop other event contexts
  while ( m_schedulerSvc->tryPopFinishedEvent( finishedEvtContext ).isSuccess() ) {
    finishedEvtContexts.push_back( finishedEvtContext );
  }

  // Now we flush them
  StatusCode finalSC;
  for ( auto& thisFinishedEvtContext : finishedEvtContexts ) {
    if ( !thisFinishedEvtContext ) {
      error() << "Detected nullptr ctxt before clearing WB!" << endmsg;
      finalSC = StatusCode::FAILURE;
      continue;
    }
    if ( m_algExecStateSvc->eventStatus( *thisFinishedEvtContext ) != EventStatus::Success ) {
      fatal() << "Failed event detected on " << thisFinishedEvtContext << endmsg;
      finalSC = StatusCode::FAILURE;
    }
    // shouldn't these incidents move to the forward scheduler?
    // If we want to consume incidents with an algorithm at the end of the graph
    // we need to add this to forward scheduler lambda action,
    // otherwise we have to do this serially on this thread!
    m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndProcessing, *thisFinishedEvtContext ) );
    m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndEvent, *thisFinishedEvtContext ) );

    debug() << "Clearing slot " << thisFinishedEvtContext->slot() << " (event " << thisFinishedEvtContext->evt()
            << ") of the whiteboard" << endmsg;

    StatusCode sc = clearWBSlot( thisFinishedEvtContext->slot() );
    if ( !sc.isSuccess() )
      error() << "Whiteboard slot " << thisFinishedEvtContext->slot() << " could not be properly cleared";

    delete thisFinishedEvtContext;

    finishedEvts++;
  }
  return finalSC;
}

//---------------------------------------------------------------------------

StatusCode HiveSlimEventLoopMgr::clearWBSlot( int evtSlot )
{
  StatusCode sc = m_whiteboard->clearStore( evtSlot );
  if ( !sc.isSuccess() ) warning() << "Clear of Event data store failed" << endmsg;
  return m_whiteboard->freeStore( evtSlot );
}
//---------------------------------------------------------------------------

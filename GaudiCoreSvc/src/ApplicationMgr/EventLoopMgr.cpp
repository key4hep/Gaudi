/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/Memory.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <GaudiKernel/MsgStream.h>

// STL includes
#include <chrono>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

/** Class definition of EventLoopMgr.
    This is the default processing manager of the application manager.
    This object handles the minimal requirements needed by the
    application manager. It also handles the default user
    configuration setup for standard event processing.

    <UL>
    <LI> handling of the incidents when starting event processing
    <LI> handling of the event loop
    </UL>

    <B>History:</B>
    <PRE>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    |13/12/00 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
    </PRE>
   @author Markus Frank
   @version 1.0
*/
class EventLoopMgr : public MinimalEventLoopMgr {
public:
protected:
  // Properties

  Gaudi::Property<std::string> m_histPersName{ this, "HistogramPersistency", {}, "name of the Hist Pers type" };
  Gaudi::Property<std::string> m_evtsel{ this, "EvtSel", {}, "event selector" };

  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc> m_evtDataMgrSvc = nullptr;
  /// Reference to the Event Data Service's IDataProviderSvc interface
  SmartIF<IDataProviderSvc> m_evtDataSvc = nullptr;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector> m_evtSelector = nullptr;
  /// Event Iterator
  IEvtSelector::Context* m_evtContext = nullptr;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc> m_histoDataMgrSvc = nullptr;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc> m_histoPersSvc = nullptr;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty> m_appMgrProperty = nullptr;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool m_endEventFired = true;

public:
  // inherit contructor from base class
  using MinimalEventLoopMgr::MinimalEventLoopMgr;

  /// Standard Destructor
  ~EventLoopMgr() override;
  /// Create event address using event selector
  StatusCode getEventRoot( IOpaqueAddress*& refpAddr );

  /// implementation of IService::initialize
  StatusCode initialize() override;
  /// implementation of IService::reinitialize
  StatusCode reinitialize() override;
  /// implementation of IService::stop
  StatusCode stop() override;
  /// implementation of IService::finalize
  StatusCode finalize() override;
  /// implementation of IEventProcessor::nextEvent
  StatusCode nextEvent( int maxevt ) override;
  /// implementation of IEventProcessor::executeEvent(EventContext&&)
  StatusCode executeEvent( EventContext&& ctx ) override;
};

DECLARE_COMPONENT( EventLoopMgr )

#define ON_DEBUG if ( outputLevel() <= MSG::DEBUG )
#define DEBMSG ON_DEBUG debug()

EventLoopMgr::~EventLoopMgr() { delete m_evtContext; }

StatusCode EventLoopMgr::initialize() {
  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::initialize();
  if ( !sc.isSuccess() ) {
    DEBMSG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }

  // Setup access to event data services
  m_evtDataMgrSvc = serviceLocator()->service( "EventDataSvc" );
  if ( !m_evtDataMgrSvc ) {
    fatal() << "Error retrieving EventDataSvc interface IDataManagerSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  m_evtDataSvc = serviceLocator()->service( "EventDataSvc" );
  if ( !m_evtDataSvc ) {
    fatal() << "Error retrieving EventDataSvc interface IDataProviderSvc." << endmsg;
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

  if ( m_evtsel != "NONE" || m_evtsel.empty() ) {
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
    m_evtSelector = nullptr;
    m_evtContext  = nullptr;
    info() << "Unable to locate service \"EventSelector\": "
           << "No events will be processed from external input." << endmsg;
  }

  setProperty( m_appMgrProperty->getProperty( "HistogramPersistency" ) ).ignore();
  if ( m_histPersName != "NONE" && !m_histPersName.empty() ) {
    // Setup access to histogramming services
    m_histoDataMgrSvc = serviceLocator()->service( "HistogramDataSvc" );
    if ( !m_histoDataMgrSvc ) {
      fatal() << "Error retrieving HistogramDataSvc." << endmsg;
      return StatusCode::FAILURE;
    }
    // Setup histogram persistency
    m_histoPersSvc = serviceLocator()->service( "HistogramPersistencySvc" );
    if ( !m_histoPersSvc ) { warning() << "Histograms cannot not be saved - though required." << endmsg; }
  }

  return StatusCode::SUCCESS;
}

StatusCode EventLoopMgr::reinitialize() {

  // Initialize the base class
  StatusCode sc = MinimalEventLoopMgr::reinitialize();
  if ( !sc.isSuccess() ) {
    DEBMSG << "Error Initializing base class MinimalEventLoopMgr." << endmsg;
    return sc;
  }

  // Check to see whether a new Event Selector has been specified
  if ( sc = setProperty( m_appMgrProperty->getProperty( "EvtSel" ) ); !sc ) return sc;
  if ( m_evtsel != "NONE" || m_evtsel.length() == 0 ) {
    auto theSvc    = serviceLocator()->service<IService>( "EventSelector" );
    auto theEvtSel = theSvc.as<IEvtSelector>();
    if ( theEvtSel && ( theEvtSel.get() != m_evtSelector.get() ) ) {
      // Setup Event Selector
      if ( m_evtSelector.get() && m_evtContext ) {
        // Need to release context before switching to new event selector
        if ( sc = m_evtSelector->releaseContext( m_evtContext ); !sc ) return sc;
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
      info() << "EventSelector service changed to " << theSvc->name() << endmsg;
    } else if ( m_evtSelector ) {
      if ( m_evtContext ) {
        if ( sc = m_evtSelector->releaseContext( m_evtContext ); !sc ) return sc;
        m_evtContext = nullptr;
      }
      sc = m_evtSelector->createContext( m_evtContext );
      if ( !sc.isSuccess() ) {
        error() << "Can not create Context " << theSvc->name() << endmsg;
        return sc;
      }
    }
  } else if ( m_evtSelector && m_evtContext ) {
    sc            = m_evtSelector->releaseContext( m_evtContext );
    m_evtSelector = nullptr;
    m_evtContext  = nullptr;
  }
  return sc;
}

StatusCode EventLoopMgr::stop() {
  if ( !m_endEventFired ) {
    // Fire pending EndEvent incident
    m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndEvent ) );
    m_endEventFired = true;
  }
  return m_evtDataMgrSvc
      ->clearStore()                                               //
      .andThen( [this]() { return MinimalEventLoopMgr::stop(); } ) //
      .orElse( [this]() { DEBMSG << "Clear of Event data store failed" << endmsg; } );
}

StatusCode EventLoopMgr::finalize() {
  // Finalize base class
  StatusCode sc = MinimalEventLoopMgr::finalize();
  if ( !sc.isSuccess() ) {
    error() << "Error finalizing base class" << endmsg;
    return sc;
  }

  // Save Histograms Now
  if ( m_histoPersSvc ) {
    std::vector<DataObject*> objects;
    sc = m_histoDataMgrSvc->traverseTree( [&objects]( IRegistry* reg, int ) {
      DataObject* obj = reg->object();
      if ( !obj || obj->clID() == CLID_StatisticsFile ) return false;
      objects.push_back( obj );
      return true;
    } );
    if ( sc.isSuccess() ) {
      // skip /stat entry!
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode isc, auto& i ) {
        IOpaqueAddress* pAddr = nullptr;
        StatusCode      iret  = m_histoPersSvc->createRep( i, pAddr );
        if ( iret.isFailure() ) return iret;
        i->registry()->setAddress( pAddr );
        return isc;
      } );
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode isc, auto& i ) {
        IRegistry* reg  = i->registry();
        StatusCode iret = m_histoPersSvc->fillRepRefs( reg->address(), i );
        return iret.isFailure() ? iret : isc;
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

  // Release event selector context
  if ( m_evtSelector && m_evtContext ) {
    m_evtSelector->releaseContext( m_evtContext ).ignore();
    m_evtContext = nullptr;
  }

  // Release all interfaces...
  m_histoDataMgrSvc = nullptr;
  m_histoPersSvc    = nullptr;

  m_evtSelector   = nullptr;
  m_evtDataSvc    = nullptr;
  m_evtDataMgrSvc = nullptr;

  return StatusCode::SUCCESS;
}

StatusCode EventLoopMgr::executeEvent( EventContext&& ctx ) {

  // DP Monitoring

  // Fire BeginEvent "Incident"
  m_incidentSvc->fireIncident( Incident( name(), IncidentType::BeginEvent ) );
  // An incident may schedule a stop, in which case is better to exit before the actual execution.
  if ( m_scheduledStop ) {
    always() << "Terminating event processing loop due to a stop scheduled by an incident listener" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute Algorithms
  m_incidentSvc->fireIncident( Incident( name(), IncidentType::BeginProcessing ) );
  StatusCode sc = MinimalEventLoopMgr::executeEvent( std::move( ctx ) );
  m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndProcessing ) );

  // Check if there was an error processing current event
  if ( !sc.isSuccess() ) { error() << "Terminating event processing loop due to errors" << endmsg; }
  return sc;
}

StatusCode EventLoopMgr::nextEvent( int maxevt ) {

  // DP Monitoring
  // Calculate runtime
  using Clock      = std::chrono::high_resolution_clock;
  using time_point = Clock::time_point;

  const float oneOver1024 = 1.f / 1024.f;

  DataObject* pObject = nullptr;
  StatusCode  sc      = StatusCode::SUCCESS;

  // loop over events if the maxevt (received as input) if different from -1.
  // if evtmax is -1 it means infinite loop
  time_point start_time = Clock::now();
  for ( int nevt = 0; maxevt == -1 || nevt < maxevt; ++nevt ) {

    if ( 1 == nevt ) // reset after first evt
      start_time = Clock::now();

    auto ctx = createEventContext();

    // always() << "Event Number = " << total_nevt
    //         << " WSS (MB) = " << System::mappedMemory(System::MemoryUnit::kByte)*oneOver1024
    //         << " Time (s) = " << secsFromStart(start_time) << endmsg;

    // Check if there is a scheduled stop issued by some algorithm/service
    if ( m_scheduledStop ) {
      m_scheduledStop = false;
      always() << "Terminating event processing loop due to scheduled stop" << endmsg;
      break;
    }
    // Clear the event store, if used in the event loop
    if ( 0 != ctx.evt() ) {

      if ( !m_endEventFired ) {
        // Fire EndEvent "Incident" (it is considered part of the clearing of the TS)
        m_incidentSvc->fireIncident( Incident( name(), IncidentType::EndEvent ) );
        m_endEventFired = true;
      }
      sc = m_evtDataMgrSvc->clearStore();
      if ( !sc.isSuccess() ) { DEBMSG << "Clear of Event data store failed" << endmsg; }
    }

    // Setup event in the event store
    if ( m_evtContext ) {
      IOpaqueAddress* addr = nullptr;
      // Only if there is a EventSelector
      sc = getEventRoot( addr );
      if ( !sc.isSuccess() ) {
        info() << "No more events in event selection " << endmsg;
        break;
      }
      // Set root clears the event data store first
      sc = m_evtDataMgrSvc->setRoot( "/Event", addr );
      if ( !sc.isSuccess() ) {
        warning() << "Error declaring event root address." << endmsg;
        continue;
      }
      sc = m_evtDataSvc->retrieveObject( "/Event", pObject );
      if ( !sc.isSuccess() ) {
        warning() << "Unable to retrieve Event root object" << endmsg;
        break;
      }
    } else {
      sc = m_evtDataMgrSvc->setRoot( "/Event", new DataObject() );
      if ( !sc.isSuccess() ) { warning() << "Error declaring event root DataObject" << endmsg; }
    }
    // Execute event for all required algorithms
    sc              = executeEvent( std::move( ctx ) );
    m_endEventFired = false;
    if ( !sc.isSuccess() ) {
      error() << "Terminating event processing loop due to errors" << endmsg;
      Gaudi::setAppReturnCode( m_appMgrProperty, Gaudi::ReturnCode::AlgorithmFailure ).ignore();
      return sc;
    }
  }

  if ( outputLevel() <= MSG::DEBUG )
    debug() << "---> Loop Finished - "
            << " WSS " << System::mappedMemory( System::MemoryUnit::kByte ) * oneOver1024
            << " | total time (skipping 1st evt) "
            << std::chrono::duration_cast<std::chrono::nanoseconds>( Clock::now() - start_time ).count() << " ns"
            << endmsg;

  return StatusCode::SUCCESS;
}

/// Create event address using event selector
StatusCode EventLoopMgr::getEventRoot( IOpaqueAddress*& refpAddr ) {
  refpAddr      = nullptr;
  StatusCode sc = m_evtSelector->next( *m_evtContext );
  if ( !sc.isSuccess() ) return sc;
  // Create root address and assign address to data service
  sc = m_evtSelector->createAddress( *m_evtContext, refpAddr );
  if ( !sc.isSuccess() ) {
    sc = m_evtSelector->next( *m_evtContext );
    if ( sc.isSuccess() ) {
      sc = m_evtSelector->createAddress( *m_evtContext, refpAddr );
      if ( !sc.isSuccess() ) warning() << "Error creating IOpaqueAddress." << endmsg;
    }
  }
  return sc;
}

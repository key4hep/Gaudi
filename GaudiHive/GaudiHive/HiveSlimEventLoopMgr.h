#ifndef GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H
#define GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/IIncidentSvc.h"
#include <GaudiKernel/IIncidentListener.h>

// Standard includes
#include <functional>

// External Libraries
#include "tbb/concurrent_queue.h"
#include <boost/dynamic_bitset.hpp>

// Forward declarations
class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;

class HiveSlimEventLoopMgr: public extends<Service,
                                           IEventProcessor> {

protected:
  StringProperty  m_histPersName {this, "HistogramPersistency",  "", ""};
  StringProperty  m_evtsel {this, "EvtSel",  "", ""};
  BooleanProperty m_warnings{this, "Warnings", true,  "Set this property to false to suppress warning messages"};
  StringProperty  m_schedulerName{this, "SchedulerName", "ForwardSchedulerSvc",  "Name of the scheduler to be used"};
  UnsignedIntegerArrayProperty  m_eventNumberBlacklist {this, "EventNumberBlackList",  {}, ""};

  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector>     m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context*      m_evtContext = nullptr;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc>  m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc>   m_histoPersSvc;
  /// Reference to the Whiteboard
  SmartIF<IHiveWhiteBoard>  m_whiteboard;
  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired = false;
  /// A shortcut for the scheduler
  SmartIF<IScheduler> m_schedulerSvc;
  /// Clear a slot in the WB
  StatusCode clearWBSlot(int evtSlot);
  /// Declare the root address of the event
  StatusCode declareEventRootAddress();
  /// Create event context
  StatusCode createEventContext(EventContext*& eventContext, int createdEvents);
  /// Drain the scheduler from all actions that may be queued
  StatusCode drainScheduler(int& finishedEvents);
  /// Instance of the incident listener waiting for AbortEvent.
  SmartIF< IIncidentListener >  m_abortEventListener;
  /// Scheduled stop of event processing
  bool                m_scheduledStop = false;
  /// Reference to the IAppMgrUI interface of the application manager
  SmartIF<IAppMgrUI> m_appMgrUI;
  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;

  //if finite number of evts is processed use bitset
  boost::dynamic_bitset<> * m_blackListBS = nullptr;

public:
  /// Standard Constructor
  HiveSlimEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~HiveSlimEventLoopMgr();
  /// Create event address using event selector
  StatusCode getEventRoot(IOpaqueAddress*& refpAddr);
  /// implementation of IService::initialize
  virtual StatusCode initialize();
  /// implementation of IService::reinitialize
  virtual StatusCode reinitialize();
  /// implementation of IService::stop
  virtual StatusCode stop();
  /// implementation of IService::finalize
  virtual StatusCode finalize();
  /// implementation of IService::nextEvent
  virtual StatusCode nextEvent(int maxevt);
  /// implementation of IEventProcessor::executeEvent(void* par)
  virtual StatusCode executeEvent(void* par);
  /// implementation of IEventProcessor::executeRun()
  virtual StatusCode executeRun(int maxevt);
  /// implementation of IEventProcessor::stopRun()
  virtual StatusCode stopRun();
};
#endif // GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H

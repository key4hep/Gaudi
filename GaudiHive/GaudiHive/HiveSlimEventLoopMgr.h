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
  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector>     m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context*      m_evtContext;
  /// Event selector
  std::string       m_evtsel;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc>  m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc>   m_histoPersSvc;
  /// Reference to the Whiteboard 
  SmartIF<IHiveWhiteBoard>  m_whiteboard;
  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  /// Name of the Hist Pers type
  std::string       m_histPersName;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired;
  /// Flag to disable warning messages when using external input
  bool              m_warnings;
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
  /// Name of the scheduler to be used
  std::string m_schedulerName;
  /// Scheduled stop of event processing
  bool                m_scheduledStop;
  /// Reference to the IAppMgrUI interface of the application manager
  SmartIF<IAppMgrUI> m_appMgrUI;  
  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;
  
  /// List of events to be skipped. The number is the number in the job.
  std::vector<unsigned int> m_eventNumberBlacklist;

  //if finite number of evts is processed use bitset
  boost::dynamic_bitset<> * m_blackListBS;

public:
  /// Standard Constructor
  HiveSlimEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  ~HiveSlimEventLoopMgr() override;
  /// Create event address using event selector
  StatusCode getEventRoot(IOpaqueAddress*& refpAddr);    
  /// implementation of IService::initialize
  StatusCode initialize() override;
  /// implementation of IService::reinitialize
  StatusCode reinitialize() override;
  /// implementation of IService::stop
  StatusCode stop() override;
  /// implementation of IService::finalize
  StatusCode finalize() override;
  /// implementation of IService::nextEvent
  StatusCode nextEvent(int maxevt) override;
  /// implementation of IEventProcessor::executeEvent(void* par)
  StatusCode executeEvent(void* par) override;
  /// implementation of IEventProcessor::executeRun()
  StatusCode executeRun(int maxevt) override;
  /// implementation of IEventProcessor::stopRun()
  StatusCode stopRun() override;
};
#endif // GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H

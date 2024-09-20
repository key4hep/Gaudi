/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H
#define GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H 1

// Framework include files
#include <GaudiKernel/IAlgExecStateSvc.h>
#include <GaudiKernel/IAlgResourcePool.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IHiveWhiteBoard.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IScheduler.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <GaudiKernel/SmartIF.h>

// External Libraries
#include <boost/dynamic_bitset.hpp>

// STL
#include <memory>

class HiveSlimEventLoopMgr : public extends<Service, IEventProcessor> {

protected:
  Gaudi::Property<std::string> m_histPersName{ this, "HistogramPersistency", "", "" };
  Gaudi::Property<std::string> m_evtsel{ this, "EvtSel", "", "" };
  Gaudi::Property<bool> m_warnings{ this, "Warnings", true, "Set this property to false to suppress warning messages" };
  Gaudi::Property<std::string>               m_schedulerName{ this, "SchedulerName", "AvalancheSchedulerSvc",
                                                "Name of the scheduler to be used" };
  Gaudi::Property<std::vector<unsigned int>> m_eventNumberBlacklist{ this, "EventNumberBlackList", {}, "" };
  Gaudi::Property<bool> m_abortOnFailure{ this, "AbortOnFailure", true, "Abort job on event failure" };

  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc> m_evtDataMgrSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector> m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context* m_evtContext = nullptr;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc> m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc> m_histoPersSvc;
  /// Reference to the Whiteboard
  SmartIF<IHiveWhiteBoard> m_whiteboard;
  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool> m_algResourcePool;
  /// Reference to the AlgExecStateSvc
  SmartIF<IAlgExecStateSvc> m_algExecStateSvc;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty> m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool m_endEventFired = false;
  /// A shortcut for the scheduler
  SmartIF<IScheduler> m_schedulerSvc;
  /// Clear a slot in the WB
  StatusCode clearWBSlot( int evtSlot );
  /// Declare the root address of the event
  StatusCode declareEventRootAddress();
  /// Drain the scheduler from all actions that may be queued
  StatusCode drainScheduler( int& finishedEvents );
  /// Instance of the incident listener waiting for AbortEvent.
  SmartIF<IIncidentListener> m_abortEventListener;
  /// Scheduled stop of event processing
  bool m_scheduledStop = false;
  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;

  // if finite number of evts is processed use bitset
  std::unique_ptr<boost::dynamic_bitset<>> m_blackListBS;

  EventContext::ContextEvt_t m_nevt{ 0 };

public:
  // inherit base class constructor
  using extends::extends;

  ~HiveSlimEventLoopMgr() override;

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
  /// implementation of IService::nextEvent
  StatusCode nextEvent( int maxevt ) override;
  /// implementation of IEventProcessor::createEventContext()
  EventContext createEventContext() override;
  /// implementation of IEventProcessor::executeEvent(EventContext&&)
  StatusCode executeEvent( EventContext&& ctx ) override;
  /// implementation of IEventProcessor::executeRun()
  StatusCode executeRun( int maxevt ) override;
  /// implementation of IEventProcessor::stopRun()
  StatusCode stopRun() override;
};
#endif // GAUDIHIVE_HIVESLIMEVENTLOOPMGR_H

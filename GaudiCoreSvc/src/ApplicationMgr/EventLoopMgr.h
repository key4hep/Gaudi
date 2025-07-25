/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDISVC_EVENTLOOPMGR_H
#define GAUDISVC_EVENTLOOPMGR_H 1

#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <memory>

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
#endif // GAUDISVC_EVENTLOOPMGR_H

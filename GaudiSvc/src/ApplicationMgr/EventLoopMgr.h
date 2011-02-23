// $Id: EventLoopMgr.h,v 1.9 2008/10/09 13:40:18 marcocle Exp $
#ifndef GAUDISVC_EVENTLOOPMGR_H
#define GAUDISVC_EVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"

// std includes
#include <memory>

// Forward declarations
class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;

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
class EventLoopMgr : public MinimalEventLoopMgr   {
public:

protected:
  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Data Service's IDataProviderSvc interface
  SmartIF<IDataProviderSvc> m_evtDataSvc;
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
  /// Name of the Hist Pers type
  std::string       m_histPersName;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired;
  /// Flag to disable warning messages when using external input
  bool              m_warnings;

public:
  /// Standard Constructor
  EventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~EventLoopMgr();
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
};
#endif // GAUDISVC_EVENTLOOPMGR_H

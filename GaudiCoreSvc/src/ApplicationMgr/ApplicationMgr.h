/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDI_APPLICATIONMGR_H
#define GAUDI_APPLICATIONMGR_H

#include <Gaudi/Interfaces/IQueueingEventProcessor.h>
#include <Gaudi/MonitoringHub.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/CommonMessaging.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IComponentManager.h>
#include <GaudiKernel/IEventProcessor.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/Service.h>
#include <vector>

// Forward declarations
class ISvcLocator;
class ServiceManager;
class AlgorithmManager;
class DLLClassManager;

class IMessageSvc;
class IIncidentSvc;
class IRunable;
class IDataManagerSvc;
class IConversionSvc;

/** @class ApplicationMgr

    The Application Manager class. The main purpose of the Application
    Manager is to steer any data processing application. This includes
    all data processing applications for LHCb data in all stages:
    simulation, reconstruction, analysis, high level triggers, etc.
    Specific implementations of the ApplicationMgr will be developed
    to cope with the different environments (on-line, off-line,
    interactive, batch, etc.).

    @author Pere Mato
*/
class ApplicationMgr
    : public PropertyHolder<CommonMessaging<implements<IAppMgrUI, Gaudi::Interfaces::IQueueingEventProcessor, IService,
                                                       IStateful, INamedInterface, IProperty>>> {
public:
  // default creator
  ApplicationMgr( IInterface* = nullptr );

  // implementation of IInterface::queryInterface
  StatusCode queryInterface( const InterfaceID& iid, void** pinterface ) override;

  // implementation of IAppMgrUI::run
  StatusCode run() override;
  // implementation of IAppMgrUI::configure
  StatusCode configure() override;
  // implementation of IAppMgrUI::terminate
  StatusCode terminate() override;
  // implementation of IAppMgrUI::initialize
  StatusCode initialize() override;
  // implementation of IAppMgrUI::start
  StatusCode start() override;
  // implementation of IAppMgrUI::stop
  StatusCode stop() override;
  // implementation of IAppMgrUI::finalize
  StatusCode finalize() override;
  // implementation of IAppMgrUI::nextEvent
  StatusCode nextEvent( int maxevt ) override;
  // implementation of IAppMgrUI::name
  const std::string& name() const override;
  /// implementation of IEventProcessor::createEventContext()
  EventContext createEventContext() override;
  /// implementation of IEventProcessor::executeEvent(void*)
  StatusCode executeEvent( EventContext&& ctx ) override;
  /// implementation of IEventProcessor::executeRun(int)
  StatusCode executeRun( int evtmax ) override;
  /// implementation of IEventProcessor::stopRun()
  StatusCode stopRun() override;

  // implementation of IStateful::state
  Gaudi::StateMachine::State FSMState() const override;
  // implementation of IStateful::targetState
  Gaudi::StateMachine::State targetFSMState() const override;
  // implementation of IStateful::reinitialize
  StatusCode reinitialize() override;
  // implementation of IStateful::reinitialize
  StatusCode restart() override;
  // implementation of IService::sysItinitilaize
  StatusCode sysInitialize() override { return StatusCode::SUCCESS; }
  // implementation of IService::sysStart
  StatusCode sysStart() override { return StatusCode::SUCCESS; }
  // implementation of IService::sysStop
  StatusCode sysStop() override { return StatusCode::SUCCESS; }
  // implementation of IService::sysFinalize
  StatusCode sysFinalize() override { return StatusCode::SUCCESS; }
  // implementation of IService::sysReinitialize
  StatusCode sysReinitialize() override { return StatusCode::SUCCESS; }
  // implementation of IService::sysRestart
  StatusCode sysRestart() override { return StatusCode::SUCCESS; }

  ///@{
  void                                                                  push( EventContext&& ctx ) override;
  bool                                                                  empty() const override;
  std::optional<Gaudi::Interfaces::IQueueingEventProcessor::ResultType> pop() override;
  ///@}

  /// @name Gaudi::Details::PropertyBase handlers
  //@{
  void       evtLoopPropertyHandler( Gaudi::Details::PropertyBase& theProp );
  StatusCode decodeExtSvcNameList();
  StatusCode decodeCreateSvcNameList();
  void       createSvcNameListHandler( Gaudi::Details::PropertyBase& );
  void       extSvcNameListHandler( Gaudi::Details::PropertyBase& theProp );
  StatusCode decodeDllNameList();
  void       dllNameListHandler( Gaudi::Details::PropertyBase& theProp );
  void       pluginDebugPropertyHandler( Gaudi::Details::PropertyBase& theProp );
  //@}

  template <class I>
  SmartIF<IComponentManager>& getManager() {
    return m_managers[I::interfaceID().id()];
  }

  inline SmartIF<ISvcManager>& svcManager() {
    // Cache the casted pointer to ISvcManager
    if ( !m_svcManager ) { m_svcManager = getManager<IService>(); }
    return m_svcManager;
  }

  inline SmartIF<IAlgManager>& algManager() {
    // Cache the casted pointer to IAlgManager
    if ( !m_algManager ) { m_algManager = getManager<IAlgorithm>(); }
    return m_algManager;
  }

  /// Needed to locate the message service
  SmartIF<ISvcLocator>& serviceLocator() const override { return m_svcLocator; }

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  void outputLevelUpdate() override;

  Gaudi::Monitoring::Hub& monitoringHub() override { return m_monitoringHub; }

  /// Print the sequence of algorithms that have been loaded.
  void printAlgsSequences();

protected:
  // implementation of IService::setServiceManager
  void setServiceManager( ISvcManager* ) override {}

  /// Internal startup routine
  StatusCode i_startup();

  /// Reach a state from current state (whichever it is) going through the
  /// correct transitions.
  /// By default, if a transition fails, the chain is interrupted, but the
  /// behavior can be changed with the parameter "gnoreFailures"
  StatusCode GoToState( Gaudi::StateMachine::State state, bool ignoreFailures = false );

  /// Typedef for the map of component managers, the key is the "id" field
  /// of the basic InterfaceID of the managed components.
  typedef std::map<unsigned long, SmartIF<IComponentManager>> ManagersMap;
  /// Map of known component managers. It contains (at least) the managers for
  /// IService and IAlgorithm. IAlgTool and IAuditor are not mandatory (but a missing
  /// manager for IAlgTool will probably not allow any job to run).
  ManagersMap m_managers;
  /// Property to declare the list of known managers.
  Gaudi::Property<std::vector<std::string>> m_declaredManagers;

  /// Cached pointer to the manager of services.
  SmartIF<ISvcManager> m_svcManager;

  /// Cached pointer to the manager of algorithms.
  SmartIF<IAlgManager> m_algManager;

  // data members
  mutable SmartIF<ISvcLocator> m_svcLocator; ///< Reference to its own service locator (must be instantiated prior to
                                             /// any service!)
  SmartIF<DLLClassManager> m_classManager;   ///< Reference to the class manager

  Gaudi::Property<int>                      m_SIGo{ this, "Go", 0,
                               [this]( auto& ) {
                                 // Re-initialize everything
                                 this->reinitialize().ignore();
                                 // Execute a number of events
                                 this->executeRun( m_evtMax ).ignore();
                               },
                               "For SI's \"Go\" command via callback" };
  Gaudi::Property<int>                      m_SIExit{ this, "Exit", 0,
                                 [this]( auto& ) {
                                   finalize().ignore();
                                   terminate().ignore();
                                   ::exit( 0 );
                                 },
                                 "For SI's \"Exit\" command via callback" };
  Gaudi::Property<std::vector<std::string>> m_topAlgNameList{ this,
                                                              "TopAlg",
                                                              {},
                                                              &ApplicationMgr::evtLoopPropertyHandler,
                                                              "List of top level algorithms names",
                                                              "vector<Algorithm>" };
  Gaudi::Property<std::vector<std::string>> m_outStreamNameList{
      this, "OutStream", {}, &ApplicationMgr::evtLoopPropertyHandler, "List of output stream names" };
  Gaudi::Property<std::string> m_outStreamType{ this, "OutStreamType", "OutputStream",
                                                &ApplicationMgr::evtLoopPropertyHandler,
                                                "[[deprecated]] Output stream type" };
  Gaudi::Property<std::string> m_messageSvcType{ this, "MessageSvcType", "MessageSvc", "MessageSvc type",
                                                 "Service:IMessageSvc" };
  Gaudi::Property<std::string> m_jobOptionsSvcType{ this, "JobOptionsSvcType", "JobOptionsSvc", "JobOptionsSvc type",
                                                    "Service:Gaudi::Interfaces::IOptionsSvc" };

  std::string                m_name        = "ApplicationMgr";             ///< Name
  Gaudi::StateMachine::State m_state       = Gaudi::StateMachine::OFFLINE; ///< Internal State
  Gaudi::StateMachine::State m_targetState = Gaudi::StateMachine::OFFLINE; ///< Internal State

  Gaudi::Property<std::vector<std::string>> m_svcMapping{ this, "SvcMapping", {}, "Default mapping of services" };
  Gaudi::Property<std::vector<std::string>> m_svcOptMapping{
      this, "SvcOptMapping", {}, "Default mapping of optional services" };

  SmartIF<IMessageSvc>     m_messageSvc;    ///< Reference to the message service
  SmartIF<IRunable>        m_runable;       ///< Reference to the runable object
  SmartIF<IEventProcessor> m_processingMgr; ///< Reference to processing manager object

  SmartIF<IQueueingEventProcessor> m_queueingProcessor; ///< Reference to a queueing processing manager object

  Gaudi::Monitoring::Hub m_monitoringHub;

  //
  // The public ApplicationMgr properties
  //

  Gaudi::Property<int> m_evtMax{ this, "EvtMax", -1, "Number of events to be processed (-1 means all events)" };
  Gaudi::Property<std::vector<std::string>> m_extSvcNameList{
      this, "ExtSvc", {}, "List of external services names", "vector<Service>" };
  Gaudi::Property<bool> m_extSvcCreates{ this, "ExtSvcCreates", true,
                                         "LHCb (default) or ATLAS definition of \"ExtSvc\"" };

  Gaudi::Property<std::vector<std::string>> m_dllNameList{ this, "Dlls", {}, "List of DDL's names" };
  Gaudi::Property<std::string> m_jobOptionsType{ this, "JobOptionsType", "FILE", "Source type (e.g. dbase, file...)" };
  Gaudi::Property<std::string> m_jobOptionsPath{ this, "JobOptionsPath", {}, "The \"file\" to look for properties" };
  Gaudi::Property<std::string> m_jobOptionsPreAction{
      this, "JobOptionsPostAction", {}, "additional command to run on config" };
  Gaudi::Property<std::string> m_jobOptionsPostAction{
      this, "JobOptionsPreAction", {}, "additional command to run on config" };
  Gaudi::Property<std::string> m_runableType{ this, "Runable", "AppMgrRunable", "Runable type" };
  Gaudi::Property<std::string> m_eventLoopMgr{ this, "EventLoop", "EventLoopMgr", "Processing manager type" };
  Gaudi::Property<std::string> m_evtsel{ this, "EvtSel", {}, "Event selection" };
  Gaudi::Property<std::string> m_histPersName{ this, "HistogramPersistency", "NONE", "Name of the Hist Pers Svc" };
  Gaudi::Property<int>         m_outputLevel{ this, "OutputLevel", MSG::INFO, "Message output level" };
  Gaudi::Property<std::string> m_appName{ this, "AppName", "ApplicationMgr", "The name of the application" };
  Gaudi::Property<std::string> m_appVersion{ this, "AppVersion", {}, "The version of the application" };
  Gaudi::Property<bool>        m_actHistory{ this, "ActivateHistory", false, "Activate HistorySvc" };
  Gaudi::Property<bool>        m_codeCheck{ this, "StatusCodeCheck", false, "[[deprecated]] will be removed" };
  Gaudi::Property<int>         m_pluginDebugLevel{ this, "PluginDebugLevel", 0,
                                           [this]( auto& ) {
                                             // Setup debug level for the plugin system
                                             if ( m_pluginDebugLevel.value() ) {
                                               MsgStream log( m_messageSvc, this->name() );
                                               log << MSG::INFO
                                                   << "Updating Gaudi::PluginService::SetDebug(level) to level="
                                                   << m_pluginDebugLevel.value() << endmsg;
                                             }
                                             Gaudi::PluginService::SetDebug( m_pluginDebugLevel );
                                           },
                                           "Debug level for the plugin system" };

  Gaudi::Property<std::vector<std::string>> m_createSvcNameList{
      this, "CreateSvc", {}, "List of extra services to be created" };

  /// Defaults for auditors.
  Gaudi::Property<bool> m_auditTools{ this, "AuditTools", false };
  Gaudi::Property<bool> m_auditSvcs{ this, "AuditServices", false };
  Gaudi::Property<bool> m_auditAlgs{ this, "AuditAlgorithms", false };

  Gaudi::Property<std::map<std::string, std::string>> m_environment{
      this, "Environment", {}, "Environment variables to set" };

  Gaudi::Property<bool> m_loopCheck{ this, "InitializationLoopCheck", true,
                                     [this]( auto& ) { this->svcManager()->setLoopCheckEnabled( m_loopCheck ); },
                                     "For ServiceMgr initialization loop checking" };

  /// Property to enable/disable the "stop on signal" service.
  /// @see Gaudi::Utils::StopSignalHandler
  Gaudi::Property<bool> m_stopOnSignal{
      this, "StopOnSignal", false, "Flag to enable/disable the signal handler that schedule a stop of the event loop" };

  /// Property to enable/disable the monitoring and reporting of stalled events.
  /// @see StalledEventMonitor
  Gaudi::Property<bool> m_stalledEventMonitoring{
      this, "StalledEventMonitoring", false, "Flag to enable/disable the monitoring and reporting of stalled events" };

  Gaudi::Property<bool> m_propertiesPrint{ this, "PropertiesPrint", false,
                                           "Flag to activate the printout of properties" };

  /// Property to record the error conditions occurring during the running.
  Gaudi::Property<int> m_returnCode{ this, "ReturnCode", Gaudi::ReturnCode::Success,
                                     "Return code of the application. Set internally in case of error conditions." };

  Gaudi::Property<bool> m_printAlgsSequence{ this, "PrintAlgsSequence", false,
                                             "Print the sequence of algorithms that have been loaded." };

  // For concurrency
  bool m_useHiveAlgorithmManager;

private:
  std::vector<std::string> m_okDlls; ///< names of successfully loaded dlls

  /// Helper to delegate calls to event processor implementation
  template <typename SELF, typename PIMPL, typename METHOD, typename... ARGS>
  static auto i_delegateToEvtProc( SELF* self, PIMPL& member, std::string_view method_name, METHOD&& method,
                                   ARGS&&... args ) {
    if ( LIKELY( self->m_state == Gaudi::StateMachine::RUNNING ) ) {
      if ( LIKELY( bool( member ) ) ) {
        return std::invoke( method, *member.get(), std::forward<ARGS>( args )... );
      } else {
        std::stringstream s;
        s << method_name << ": event processor is not a \""
          << System::typeinfoName( typeid( decltype( *member.get() ) ) ) << '"';
        throw GaudiException{ s.str(), self->name(), StatusCode::FAILURE };
      }
    }
    std::stringstream s;
    s << method_name << ": Invalid state \"" << self->FSMState() << '"';
    throw GaudiException{ s.str(), self->name(), StatusCode::FAILURE };
  }
};
#endif // GAUDI_APPLICATIONMGR_H

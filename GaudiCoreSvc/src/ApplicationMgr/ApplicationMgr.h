// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiSvc/src/ApplicationMgr/ApplicationMgr.h,v 1.34 2008/11/10 15:29:09 marcocle Exp $	//
#ifndef GAUDI_APPLICATIONMGR_H
#define GAUDI_APPLICATIONMGR_H

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/IComponentManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/PropertyMgr.h"

// STL include files
#include <list>
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
class IJobOptionsSvc;

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
class ApplicationMgr : public CommonMessaging<implements4<IAppMgrUI,
                                                          IEventProcessor,
                                                          IService,
                                                          IStateful> > {
  typedef CommonMessaging<implements4<IAppMgrUI,
                                      IEventProcessor,
                                      IService,
                                      IStateful> > base_class;
public:
  typedef std::list<std::pair<IService*,int> >  ListSvc;
  typedef std::list<std::string>   ListName;
  typedef std::vector<std::string> VectorName;

public:

  // default creator
  ApplicationMgr(IInterface* = 0);
  // virtual destructor
  virtual ~ApplicationMgr();

  // implementation of IInterface::queryInterface
  virtual StatusCode queryInterface(const InterfaceID& iid, void** pinterface);

  // implementation of IAppMgrUI::run
  virtual StatusCode run();
  // implementation of IAppMgrUI::configure
  virtual StatusCode configure();
  // implementation of IAppMgrUI::terminate
  virtual StatusCode terminate();
  // implementation of IAppMgrUI::initialize
  virtual StatusCode initialize();
  // implementation of IAppMgrUI::start
  virtual StatusCode start();
  // implementation of IAppMgrUI::stop
  virtual StatusCode stop();
  // implementation of IAppMgrUI::finalize
  virtual StatusCode finalize();
  // implementation of IAppMgrUI::nextEvent
  virtual StatusCode nextEvent(int maxevt);
  // implementation of IAppMgrUI::name
  virtual const std::string&  name() const;
  /// implementation of IEventProcessor::executeEvent(void*)
  virtual StatusCode executeEvent(void* par);
  /// implementation of IEventProcessor::executeRun(int)
  virtual StatusCode executeRun(int evtmax);
  /// implementation of IEventProcessor::stopRun()
  virtual StatusCode stopRun();

  // implementation of IStateful::state
  virtual Gaudi::StateMachine::State FSMState() const;
  // implementation of IStateful::targetState
  virtual Gaudi::StateMachine::State targetFSMState() const;
  // implementation of IStateful::reinitialize
  virtual StatusCode reinitialize();
  // implementation of IStateful::reinitialize
  virtual StatusCode restart();
  // implementation of IService::sysItinitilaize
  virtual StatusCode sysInitialize() { return StatusCode::SUCCESS; }
  // implementation of IService::sysStart
  virtual StatusCode sysStart() { return StatusCode::SUCCESS; }
  // implementation of IService::sysStop
  virtual StatusCode sysStop() { return StatusCode::SUCCESS; }
  // implementation of IService::sysFinalize
  virtual StatusCode sysFinalize() { return StatusCode::SUCCESS; }
  // implementation of IService::sysReinitialize
  virtual StatusCode sysReinitialize() { return StatusCode::SUCCESS; }
  // implementation of IService::sysRestart
  virtual StatusCode sysRestart() { return StatusCode::SUCCESS; }

  // SI Go Handler
  void       SIGoHandler             ( Property& theProp );
  // SI Exit handler
  void       SIExitHandler           ( Property& theProp );

  /// @name Property handlers
  //@{
  void       evtLoopPropertyHandler  ( Property& theProp );
  StatusCode decodeExtSvcNameList    ( );
  StatusCode decodeCreateSvcNameList ( );
  void createSvcNameListHandler(Property &);
  void       extSvcNameListHandler   ( Property& theProp );
  StatusCode decodeMultiThreadSvcNameList( );
  void  multiThreadSvcNameListHandler( Property& theProp );
  StatusCode decodeDllNameList       ( );
  void       dllNameListHandler      ( Property& theProp );
  void       pluginDebugPropertyHandler ( Property& theProp );
  void       initLoopCheckHndlr      ( Property& );
  //@}

  template <class I>
  SmartIF<IComponentManager> &getManager() {
    return m_managers[I::interfaceID().id()];
  }

  inline SmartIF<ISvcManager> &svcManager() {
    // Cache the casted pointer to ISvcManager
    if (!m_svcManager) {
      m_svcManager = getManager<IService>();
    }
    return m_svcManager;
  }

  inline SmartIF<IAlgManager> &algManager() {
    // Cache the casted pointer to IAlgManager
    if (!m_algManager) {
      m_algManager = getManager<IAlgorithm>();
    }
    return m_algManager;
  }

  /// Needed to locate the message service
  virtual SmartIF<ISvcLocator>& serviceLocator() const {
    return m_svcLocator;
  }

protected:
  /// declare one or more copies of svc type/name as determined by NoOfThreads
  StatusCode declareMultiSvcType(const std::string& name,
				 const std::string& type);
  /// add one or more copies of svc type/name as determined by NoOfThreads
  StatusCode addMultiSvc(const Gaudi::Utils::TypeNameString &typeName,
			 int prio);

  // implementation of IService::setServiceManager
  void setServiceManager(ISvcManager*) {}

  /// Internal startup routine
  StatusCode i_startup();

  /// Reach a state from current state (whichever it is) going through the
  /// correct transitions.
  /// By default, if a transition fails, the chain is interrupted, but the
  /// behavior can be changed with the parameter "gnoreFailures"
  StatusCode GoToState(Gaudi::StateMachine::State state, bool ignoreFailures = false);

  /// Typedef for the map of component managers, the key is the "id" field
  /// of the basic InterfaceID of the managed components.
  typedef std::map<unsigned long, SmartIF<IComponentManager> > ManagersMap;
  /// Map of known component managers. It contains (at least) the managers for
  /// IService and IAlgorithm. IAlgTool and IAuditor are not mandatory (but a missing
  /// manager for IAlgTool will probably not allow any job to run).
  ManagersMap         m_managers;
  /// Property to declare the list of known managers.
  StringArrayProperty m_declaredManagers;

  /// Cached pointer to the manager of services.
  SmartIF<ISvcManager> m_svcManager;

  /// Cached pointer to the manager of algorithms.
  SmartIF<IAlgManager> m_algManager;


  // data members
  DLLClassManager*    m_classManager;       ///< Reference to the class manager
  mutable SmartIF<ISvcLocator> m_svcLocator; ///< Reference to its own service locator

  IntegerProperty     m_SIGo;               ///< For SI's "Go" command via callback
  IntegerProperty     m_SIExit;             ///< For SI's "Exit" command via callback
  StringArrayProperty m_topAlgNameList;     ///< List of top level algorithms names
  StringArrayProperty m_outStreamNameList;  ///< List of output stream names
  StringProperty      m_outStreamType;      ///< Output stream type (obsolete?)
  StringProperty      m_messageSvcType;     ///< MessageSvc type
  StringProperty      m_jobOptionsSvcType;  ///< JobOptionsSvc type


  ListSvc             m_topSvcList;         ///< List of top level services
  ListName            m_topSvcNameList;     ///< List of top level services names

  std::string         m_name;               ///< Name
  Gaudi::StateMachine::State m_state;              ///< Internal State
  Gaudi::StateMachine::State m_targetState;              ///< Internal State

  VectorName          m_defServices;        ///< Vector default services names
  VectorName          m_svcMapping;         ///< Default mapping of services
  VectorName          m_svcOptMapping;      ///< Default mapping of services

  PropertyMgr*        m_propertyMgr;        ///< Reference to Property Manager
  SmartIF<IMessageSvc>      m_messageSvc;         ///< Reference to the message service
  SmartIF<IRunable>         m_runable;            ///< Reference to the runable object
  SmartIF<IEventProcessor>  m_processingMgr;      ///< Reference to processing manager object
  SmartIF<IJobOptionsSvc>   m_jobOptionsSvc;      ///< Reference to JobOption service

  //
  // The public ApplicationMgr properties
  //

  int                  m_evtMax;            ///< Number of events to be processed
  StringArrayProperty  m_extSvcNameList;    ///< List of external services names
  BooleanProperty      m_extSvcCreates;     ///< LHCb or ATLAS defn of "ExtSvc"

  /// List of external services names for which we want a copy per evt thread
  StringArrayProperty  m_multiThreadSvcNameList;
  int                  m_noOfEvtThreads;    ///< no of multiThreadSvc copies

  StringArrayProperty  m_dllNameList;       ///< List of DDL's names
  std::string          m_jobOptionsType;    ///< Source type (e.g. dbase, file...)
  std::string          m_jobOptionsPath;    ///< The "file" to look for properties
  std::string          m_runableType;       ///< Runable type
  std::string          m_eventLoopMgr;      ///< Processing manager type
  std::string          m_evtsel;            ///< Event selection
  std::string          m_histPersName;      ///< CGL: Name of the Hist Pers Svc
  int                  m_outputLevel;       ///< Message output level
  std::string          m_appName;           ///< The name of the application
  std::string          m_appVersion;        ///< The version of the application
  bool                 m_actHistory;        ///< Activate HistorySvc
  bool                 m_codeCheck;         ///< Activate StatusCode checking
  IntegerProperty      m_pluginDebugLevel;  ///< Debug level for the plugin system

  StringArrayProperty  m_createSvcNameList;

  /// Defaults for auditors.
  BooleanProperty      m_auditTools;
  BooleanProperty      m_auditSvcs;
  BooleanProperty      m_auditAlgs;

  std::map<std::string,std::string> m_environment; ///< Environment variables to set

  /// For ServiceMgr initialization loop checking
  BooleanProperty      m_loopCheck;

  /// Property to enable/disable the "stop on signal" service (enabled by default).
  /// @see Gaudi::Utils::StopSignalHandler
  BooleanProperty      m_stopOnSignal;

  /// Property to enable/disable the monitoring and reporting of stalled events (enabled by default).
  /// @see StalledEventMonitor
  BooleanProperty      m_stalledEventMonitoring;

  // Flag to activate the printout of properties
  bool                 m_propertiesPrint ; ///< flag to activate the printout of properties

  /// Property to record the error conditions occurring during the running.
  IntegerProperty      m_returnCode;

private:
   std::vector<std::string> m_okDlls;       ///< names of successfully loaded dlls
};
#endif  // GAUDI_APPLICATIONMGR_H

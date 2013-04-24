// Include files
#include "ApplicationMgr.h"
#include "ServiceManager.h"
#include "AlgorithmManager.h"
#include "DLLClassManager.h"

#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/ObjectFactory.h"

#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ThreadGaudi.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Time.h"
#include "GaudiKernel/System.h"

#include "GaudiKernel/AppReturnCode.h"

#include "GaudiCoreSvcVersion.h"


using System::getEnv;
using System::isEnvSet;

#include <algorithm>
#include <cassert>
#include <ctime>
#include <limits>

static const char* s_eventloop = "EventLoop";
static const char* s_runable   = "Runable";

#define ON_DEBUG if (UNLIKELY(m_outputLevel <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(m_outputLevel <= MSG::VERBOSE))

DECLARE_OBJECT_FACTORY(ApplicationMgr)

// Implementation class for the Application Manager. In this way the
// ApplicationMgr class is a fully insulated concrete class. Clients
// (main programs) will not need to re-compile if there are changes
// in the implementation

//=======================================================================
// Constructor
//=======================================================================
ApplicationMgr::ApplicationMgr(IInterface*): base_class() {
  // IInterface initialization
  addRef(); // Initial count set to 1

  // Initialize two basic services: messagesvc & joboptions
  m_messageSvc        = 0;
  m_jobOptionsSvc     = 0;

  // Instantiate component managers
  m_managers[IService::interfaceID().id()] = new ServiceManager(this);
  m_managers[IAlgorithm::interfaceID().id()] = new AlgorithmManager(this);

  m_svcLocator = svcManager();

  // Instantiate internal services
  // SvcLocator/Factory HAS to be already instantiated
  m_classManager = new DLLClassManager(this);
  m_propertyMgr  = new PropertyMgr(this);

  m_name  = "ApplicationMgr";
  m_state = Gaudi::StateMachine::OFFLINE;
  m_targetState = Gaudi::StateMachine::OFFLINE;

  m_propertyMgr->declareProperty("Go",            m_SIGo = 0 );
  m_propertyMgr->declareProperty("Exit",          m_SIExit = 0 );
  m_propertyMgr->declareProperty("Dlls",          m_dllNameList );
  m_propertyMgr->declareProperty("ExtSvc",        m_extSvcNameList );
  m_propertyMgr->declareProperty("CreateSvc",     m_createSvcNameList );
  m_propertyMgr->declareProperty("ExtSvcCreates", m_extSvcCreates=true );

  m_propertyMgr->declareProperty("SvcMapping",    m_svcMapping );
  m_propertyMgr->declareProperty("SvcOptMapping", m_svcOptMapping );

  m_propertyMgr->declareProperty("TopAlg",        m_topAlgNameList );
  m_propertyMgr->declareProperty("OutStream",     m_outStreamNameList );
  m_propertyMgr->declareProperty("OutStreamType", m_outStreamType = "OutputStream" );
  m_propertyMgr->declareProperty("MessageSvcType",m_messageSvcType= "MessageSvc" );
  m_propertyMgr->declareProperty("JobOptionsSvcType",
                                 m_jobOptionsSvcType = "JobOptionsSvc" );
  m_propertyMgr->declareProperty( s_runable,      m_runableType   = "AppMgrRunable");
  m_propertyMgr->declareProperty( s_eventloop,    m_eventLoopMgr  = "EventLoopMgr");

  m_propertyMgr->declareProperty("HistogramPersistency", m_histPersName="NONE");

  // Declare Job Options Service properties and set default
  m_propertyMgr->declareProperty("JobOptionsType", m_jobOptionsType = "FILE");
  m_propertyMgr->declareProperty("JobOptionsPath", m_jobOptionsPath = "");
  m_propertyMgr->declareProperty("EvtMax",         m_evtMax = -1);
  m_propertyMgr->declareProperty("EvtSel",         m_evtsel );
  m_propertyMgr->declareProperty("OutputLevel",    m_outputLevel = MSG::INFO);

  m_propertyMgr->declareProperty("MultiThreadExtSvc", m_multiThreadSvcNameList);
  m_propertyMgr->declareProperty("NoOfThreads",    m_noOfEvtThreads = 0);
  m_propertyMgr->declareProperty("AppName",        m_appName = "ApplicationMgr");
  m_propertyMgr->declareProperty("AppVersion",     m_appVersion = "");

  m_propertyMgr->declareProperty("AuditTools",      m_auditTools = false);
  m_propertyMgr->declareProperty("AuditServices",   m_auditSvcs = false);
  m_propertyMgr->declareProperty("AuditAlgorithms", m_auditAlgs = false);

  m_propertyMgr->declareProperty("ActivateHistory", m_actHistory = false);
  m_propertyMgr->declareProperty("StatusCodeCheck", m_codeCheck = false);

  m_propertyMgr->declareProperty("Environment",    m_environment);

  // ServiceMgr Initialization loop checking
  m_propertyMgr->declareProperty("InitializationLoopCheck", m_loopCheck = true)
    ->declareUpdateHandler(&ApplicationMgr::initLoopCheckHndlr, this);
  svcManager()->setLoopCheckEnabled(m_loopCheck);

  // Flag to activate the printout of properties
  m_propertyMgr->declareProperty
    ( "PropertiesPrint",
      m_propertiesPrint = false,
      "Flag to activate the printout of properties" );

  m_propertyMgr->declareProperty("ReflexPluginDebugLevel", m_reflexDebugLevel = 0 );

  m_propertyMgr->declareProperty("StopOnSignal", m_stopOnSignal = false,
      "Flag to enable/disable the signal handler that schedule a stop of the event loop");

  m_propertyMgr->declareProperty("StalledEventMonitoring", m_stalledEventMonitoring = false,
      "Flag to enable/disable the monitoring and reporting of stalled events");

  m_propertyMgr->declareProperty("ReturnCode", m_returnCode = Gaudi::ReturnCode::Success,
      "Return code of the application. Set internally in case of error conditions.");

  // Add action handlers to the appropriate properties
  m_SIGo.declareUpdateHandler  ( &ApplicationMgr::SIGoHandler         , this );
  m_SIExit.declareUpdateHandler( &ApplicationMgr::SIExitHandler       , this );
  m_topAlgNameList.declareUpdateHandler(&ApplicationMgr::evtLoopPropertyHandler, this);
  m_outStreamNameList.declareUpdateHandler(&ApplicationMgr::evtLoopPropertyHandler, this);
  m_outStreamType.declareUpdateHandler(&ApplicationMgr::evtLoopPropertyHandler, this);
  m_reflexDebugLevel.declareUpdateHandler(&ApplicationMgr::reflexDebugPropertyHandler, this);
  m_svcMapping.push_back("EvtDataSvc/EventDataSvc");
  m_svcMapping.push_back("DetDataSvc/DetectorDataSvc");
  m_svcMapping.push_back("HistogramSvc/HistogramDataSvc");
  m_svcMapping.push_back("HbookCnv::PersSvc/HbookHistSvc");
  m_svcMapping.push_back("RootHistCnv::PersSvc/RootHistSvc");
  m_svcMapping.push_back("EvtPersistencySvc/EventPersistencySvc");
  m_svcMapping.push_back("DetPersistencySvc/DetectorPersistencySvc");
  m_svcMapping.push_back("HistogramPersistencySvc/HistogramPersistencySvc");
}

//============================================================================
// destructor
//============================================================================
ApplicationMgr::~ApplicationMgr() {
  if( m_classManager ) m_classManager->release();
  if( m_propertyMgr ) m_propertyMgr->release();
  if( m_messageSvc ) m_messageSvc->release();
  if( m_jobOptionsSvc ) m_jobOptionsSvc->release();
}

//============================================================================
// IInterface implementation: queryInterface::addRef()
//============================================================================
StatusCode ApplicationMgr::queryInterface
( const InterfaceID& iid  ,
  void**             ppvi )
{
  if ( 0 == ppvi ) { return StatusCode::FAILURE ; }

  // try to find own/direct interfaces:
  StatusCode sc = base_class::queryInterface(iid,ppvi);
  if (sc.isSuccess()) return sc;

  // find indirect interfaces :
  if      ( ISvcLocator     ::interfaceID() . versionMatch ( iid ) )
  { return serviceLocator()-> queryInterface ( iid , ppvi ) ; }
  else if ( ISvcManager     ::interfaceID() . versionMatch ( iid ) )
  { return svcManager()    -> queryInterface ( iid , ppvi ) ; }
  else if ( IAlgManager     ::interfaceID() . versionMatch ( iid ) )
  { return algManager()    -> queryInterface ( iid , ppvi ) ; }
  else if ( IClassManager   ::interfaceID() . versionMatch ( iid ) )
  { return m_classManager  -> queryInterface ( iid , ppvi ) ; }
  else if ( IProperty       ::interfaceID() . versionMatch ( iid ) )
  { return m_propertyMgr   -> queryInterface ( iid , ppvi ) ; }
  else if ( IMessageSvc     ::interfaceID() . versionMatch ( iid ) )
  {
    *ppvi = reinterpret_cast<void*>(m_messageSvc.get());
    if (m_messageSvc) {
      m_messageSvc->addRef();
    }
    // Note that 0 can be a valid IMessageSvc pointer value (when used for
    // MsgStream).
    return StatusCode::SUCCESS;
  }
  *ppvi = 0;
  return StatusCode::FAILURE;
}

//============================================================================
// ApplicationMgr::i_startup()
//============================================================================
StatusCode ApplicationMgr::i_startup() {
  StatusCode  sc;

  // declare factories in current module
  m_classManager->loadModule("").ignore();

  // Create the Message service
  SmartIF<IService> msgsvc = svcManager()->createService(Gaudi::Utils::TypeNameString("MessageSvc", m_messageSvcType));
  if( !msgsvc.isValid() )  {
    fatal() << "Error creating MessageSvc of type " << m_messageSvcType << endmsg;
    return sc;
  }
  // Create the Job Options service
  SmartIF<IService> jobsvc = svcManager()->createService(Gaudi::Utils::TypeNameString("JobOptionsSvc", m_jobOptionsSvcType));
  if( !jobsvc.isValid() )   {
    fatal() << "Error creating JobOptionsSvc" << endmsg;
    return sc;
  }

  SmartIF<IProperty> jobOptsIProp(jobsvc);
  if ( !jobOptsIProp.isValid() )   {
    fatal() << "Error locating JobOptionsSvc" << endmsg;
    return sc;
  }
  sc = jobOptsIProp->setProperty( StringProperty("TYPE", m_jobOptionsType) );
  if( !sc.isSuccess() )   {
    fatal() << "Error setting TYPE option in JobOptionsSvc" << endmsg;
    return sc;
  }

  if ( m_jobOptionsPath != "") {         // The command line takes precedence
    sc = jobOptsIProp->setProperty( StringProperty("PATH", m_jobOptionsPath) );
    if( !sc.isSuccess() )   {
      fatal() << "Error setting PATH option in JobOptionsSvc" << endmsg;
      return sc;
    }
  }
  else if ( isEnvSet("JOBOPTPATH") ) {// Otherwise the Environment JOBOPTPATH
    sc = jobOptsIProp->setProperty (StringProperty("PATH",
                                                   getEnv("JOBOPTPATH")));
    if( !sc.isSuccess() )   {
      fatal()
           << "Error setting PATH option in JobOptionsSvc from env"
           << endmsg;
      return sc;
    }
  }
  else {                                   // Otherwise the default
    sc = jobOptsIProp->setProperty (StringProperty("PATH",
                                                   "../options/job.opts"));
    if( !sc.isSuccess() )   {
      fatal()
           << "Error setting PATH option in JobOptionsSvc to default"
           << endmsg;
      return sc;
    }
  }
  jobOptsIProp->release();

  // Sets my default the Output Level of the Message service to be
  // the same as this
  SmartIF<IProperty> msgSvcIProp(msgsvc);
  msgSvcIProp->setProperty( IntegerProperty("OutputLevel", m_outputLevel)).ignore();
  msgSvcIProp->release();

  sc = jobsvc->sysInitialize();
  if( !sc.isSuccess() )   {
    fatal() << "Error initializing JobOptionsSvc" << endmsg;
    return sc;
  }
  sc = msgsvc->sysInitialize();
  if( !sc.isSuccess() )  {
    fatal() << "Error initializing MessageSvc" << endmsg;
    return sc;
  }

  // Get the useful interface from Message and JobOptions services
  m_messageSvc = m_svcLocator->service("MessageSvc");
  if( !m_messageSvc.isValid() )  {
    fatal() << "Error retrieving MessageSvc." << endmsg;
    return sc;
  }
  m_jobOptionsSvc = m_svcLocator->service("JobOptionsSvc");
  if( !m_jobOptionsSvc.isValid() )  {
    fatal() << "Error retrieving JobOptionsSvc." << endmsg;
    return sc;
  }

  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::configure()
//============================================================================
StatusCode ApplicationMgr::configure() {
  // Check if the state is compatible with the transition
  MsgStream tlog( m_messageSvc, name() );
  if( Gaudi::StateMachine::CONFIGURED == m_state ) {
    tlog << MSG::INFO << "Already Configured" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( Gaudi::StateMachine::OFFLINE != m_state ) {
    tlog << MSG::FATAL
         << "configure: Invalid state \""  << m_state << "\"" << endmsg;
    return StatusCode::FAILURE;
  }
  m_targetState = Gaudi::StateMachine::CONFIGURED;

  // Reset application return code.
  m_returnCode = Gaudi::ReturnCode::Success;

  StatusCode  sc;
  sc = i_startup();
  if ( !sc.isSuccess() )    {
    return sc;
  }

  MsgStream log( m_messageSvc, name() );

  // Get my own options using the Job options service
  if (log.level() <= MSG::DEBUG)
    log << MSG::DEBUG << "Getting my own properties" << endmsg;
  sc = m_jobOptionsSvc->setMyProperties( name(), m_propertyMgr );
  if( !sc.isSuccess() ) {
    log << MSG::WARNING << "Problems getting my properties from JobOptionsSvc"
        << endmsg;
    return sc;
  }

  // Check current outputLevel to eventually inform the MessageSvc
  if( m_outputLevel != MSG::NIL && !m_appName.empty() ) {
    assert(m_messageSvc != 0);
    m_messageSvc->setOutputLevel( name(), m_outputLevel );
    // Print a welcome message
    log << MSG::ALWAYS
        << std::endl
        << "=================================================================="
        << "=================================================================="
        << std::endl
        << "                                "
        << "                   Welcome to " << m_appName;

    if( "" != m_appVersion ) {
      log << MSG::ALWAYS << " version " << m_appVersion;
    }
    else {
      log << MSG::ALWAYS
          << " (GaudiCoreSvc "
          << "v" << GAUDICORESVC_MAJOR_VERSION
          << "r" << GAUDICORESVC_MINOR_VERSION
#if GAUDICORESVC_PATCH_VERSION
          << "p" << GAUDICORESVC_PATCH_VERSION
#endif
          << ")";
    }

    // Add the host name and current time to the message
    log << MSG::ALWAYS
        << std::endl
        << "                                "
        << "          running on " << System::hostName()
        << " on " << Gaudi::Time::current().format(true) << std::endl
        << "=================================================================="
        << "=================================================================="
        << endmsg;
  }

  // print all own properties if the options "PropertiesPrint" is set to true
  if ( m_propertiesPrint )
  {
    typedef std::vector<Property*> Properties;
    const Properties& properties = m_propertyMgr->getProperties() ;
    log << MSG::ALWAYS
        << "List of ALL properties of "
        << System::typeinfoName ( typeid( *this ) ) << "/" << this->name()
        << "  #properties = " << properties.size() << endmsg ;
    for ( Properties::const_iterator property = properties.begin() ;
          properties.end() != property ; ++property )
    { log << "Property ['Name': Value] = " << ( **property) << endmsg ; }
  }

  // Check if StatusCode need to be checked
  if (m_codeCheck) {
    StatusCode::enableChecking();
    sc = addMultiSvc("StatusCodeSvc", -9999);
    if ( sc.isFailure() ) {
      log << MSG::FATAL << "Error adding StatusCodeSvc for multiple threads" << endmsg;
      return StatusCode::FAILURE;
    }
  } else {
    StatusCode::disableChecking();
  }

  // set the requested environment variables
  std::map<std::string,std::string>::iterator var;
  for ( var = m_environment.begin(); var != m_environment.end(); ++var ) {
    const std::string &name  = var->first;
    const std::string &value = var->second;
    std::string old = System::getEnv(name.c_str());
    const MSG::Level lvl = (!old.empty() && (old != "UNKNOWN" ))
        ? MSG::WARNING
        : MSG::DEBUG;
    if (UNLIKELY(m_outputLevel <= lvl))
      log << lvl << "Setting " << name << " = " << value << endmsg;
    System::setEnv(name,value);
  }

  //Declare Service Types
  VectorName::const_iterator j;
  for(j=m_svcMapping.begin(); j != m_svcMapping.end(); ++j)  {
    Gaudi::Utils::TypeNameString itm(*j);
    if ( declareMultiSvcType(itm.name(), itm.type()).isFailure() )  {
      log << MSG::ERROR << "configure: declaring svc type:'" << *j << "' failed." << endmsg;
      return StatusCode::FAILURE;
    }
  }
  for(j=m_svcOptMapping.begin(); j != m_svcOptMapping.end(); ++j)  {
    Gaudi::Utils::TypeNameString itm(*j);
    if ( declareMultiSvcType(itm.name(), itm.type()).isFailure() )  {
      log << MSG::ERROR << "configure: declaring svc type:'" << *j << "' failed." << endmsg;
      return StatusCode::FAILURE;
    }
  }

  //--------------------------------------------------------------------------
  // Declare other Services and Algorithms by loading DLL's
  sc = decodeDllNameList( );
  if ( sc.isFailure( ) ) {
    log << MSG::ERROR << "Failure loading declared DLL's" << endmsg;
    return sc;
  }

  //--------------------------------------------------------------------------
  // Deal with the services explicitly declared by the user.
  sc = decodeExtSvcNameList();
  if ( sc.isFailure( ) ) {
    log << MSG::ERROR << "Failure during external service association" << endmsg;
    return sc;
  }

  sc = decodeMultiThreadSvcNameList( );
  if ( sc.isFailure( ) ) {
    log << MSG::ERROR << "Failure during multi thread service creation"
        << endmsg;
    return sc;
  }

  sc = decodeCreateSvcNameList();
  if ( sc.isFailure( ) ) {
    log << MSG::ERROR << "Failure during external service creation" << endmsg;
    return sc;
  }


  //--------------------------------------------------------------------------
  // Retrieve intrinsic services. If needed configure them.
  //--------------------------------------------------------------------------
  Gaudi::Utils::TypeNameString evtloop_item(m_eventLoopMgr);
  sc = addMultiSvc(evtloop_item, ServiceManager::DEFAULT_SVC_PRIORITY*10);
  if( !sc.isSuccess() )  {
    log << MSG::FATAL << "Error adding :" << m_eventLoopMgr << endmsg;
    return sc;
  }

  if (m_noOfEvtThreads == 0) {
    m_runable = m_svcLocator->service(m_runableType);
    if( !m_runable.isValid() )  {
      log << MSG::FATAL
          << "Error retrieving Runable:" << m_runableType
          << "\n Check option ApplicationMgr." << s_runable << endmsg;
      return sc;
    }
    m_processingMgr = m_svcLocator->service(evtloop_item);
    if( !m_processingMgr.isValid() )  {
      log << MSG::FATAL
          << "Error retrieving Processing manager:" << m_eventLoopMgr
          << "\n Check option ApplicationMgr." << s_eventloop
          << "\n No events will be processed." << endmsg;
      return sc;
    }
  }

  // Establish Update Handlers for ExtSvc and DLLs Properties
  m_extSvcNameList.declareUpdateHandler (&ApplicationMgr::extSvcNameListHandler,
                                         this);
  m_createSvcNameList.declareUpdateHandler (&ApplicationMgr::createSvcNameListHandler,
                                         this);
  m_multiThreadSvcNameList.declareUpdateHandler
    (&ApplicationMgr::multiThreadSvcNameListHandler, this);
  m_dllNameList.declareUpdateHandler (&ApplicationMgr::dllNameListHandler,
                                      this );

  if (m_actHistory) {
    // Create HistorySvc with a priority to ensure it's initialized last, finalized first
    sc = svcManager()->addService("HistorySvc",std::numeric_limits<int>::max());
    if ( sc.isFailure() ) {
      log << MSG::FATAL << "Error adding HistorySvc" << endmsg;
      return StatusCode::FAILURE;
    }

    if (m_noOfEvtThreads > 0) {
      sc = addMultiSvc("HistorySvc",std::numeric_limits<int>::max());
      if ( sc.isFailure() ) {
        log << MSG::FATAL << "Error adding HistorySvc for multiple threads"
            << endmsg;
        return StatusCode::FAILURE;
      }
    }
  }

  log << MSG::INFO << "Application Manager Configured successfully" << endmsg;
  m_state = m_targetState;
  return StatusCode::SUCCESS;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::initialize()
//============================================================================
StatusCode ApplicationMgr::initialize() {

  MsgStream log( m_messageSvc, name() );
  StatusCode sc;

  // I cannot add these services in configure() because they are coming from GaudiUtils
  // and it messes up genconf when rebuilding it.
  if (m_stopOnSignal) {
    // Instantiate the service that schedules a stop when a signal is received
    std::string svcname("Gaudi::Utils::StopSignalHandler");
    sc = svcManager()->addService(svcname);
    if ( sc.isFailure() ) {
      log << MSG::INFO << "Cannot instantiate " << svcname << "signals will be ignored" << endmsg;
    }
  }

  if (m_stalledEventMonitoring) {
    // Instantiate the service that schedules a stop when a signal is received
    std::string svcname("StalledEventMonitor");
    sc = svcManager()->addService(svcname);
    if ( sc.isFailure() ) {
      log << MSG::INFO << "Cannot instantiate " << svcname << "signals will be ignored" << endmsg;
    }
  }

  if( m_state == Gaudi::StateMachine::INITIALIZED ) {
    log << MSG::INFO << "Already Initialized!" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( m_state != Gaudi::StateMachine::CONFIGURED ) {
    log << MSG::FATAL
         << "initialize: Invalid state \""  << m_state << "\"" << endmsg;
    return StatusCode::FAILURE;
  }
  m_targetState = Gaudi::StateMachine::INITIALIZED;

  //--------------------------------------------------------------------------
  // Initialize the list of top Services
  //--------------------------------------------------------------------------
  sc = svcManager()->initialize();
  if( !sc.isSuccess() ) return sc;

  //--------------------------------------------------------------------------
  // Final steps: Inform user and change internal state
  //--------------------------------------------------------------------------
  log << MSG::INFO << "Application Manager Initialized successfully"  << endmsg;
  m_state = m_targetState;

  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::start()
//============================================================================
StatusCode ApplicationMgr::start() {

  MsgStream log( m_messageSvc, name() );
  StatusCode sc;

  if( m_state == Gaudi::StateMachine::RUNNING ) {
    log << MSG::INFO << "Already Initialized!" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( m_state != Gaudi::StateMachine::INITIALIZED ) {
    log << MSG::FATAL
         << "start: Invalid state \""  << m_state << "\"" << endmsg;
    return StatusCode::FAILURE;
  }
  m_targetState = Gaudi::StateMachine::RUNNING;

  //--------------------------------------------------------------------------
  // Initialize the list of top Services
  //--------------------------------------------------------------------------
  sc = svcManager()->start();
  if( !sc.isSuccess() ) return sc;

  //--------------------------------------------------------------------------
  // Final steps: Inform user and change internal state
  //--------------------------------------------------------------------------
  log << MSG::INFO << "Application Manager Started successfully"  << endmsg;
  m_state = m_targetState;

  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::nextEvent(int)
//============================================================================
StatusCode ApplicationMgr::nextEvent(int maxevt)    {
  if( m_state != Gaudi::StateMachine::RUNNING ) {
    MsgStream log( m_messageSvc, name() );
    log << MSG::FATAL << "nextEvent: Invalid state \"" << m_state << "\""
        << endmsg;
    return StatusCode::FAILURE;
  }
  if (!m_processingMgr.isValid())   {
    MsgStream log( m_messageSvc, name() );
    log << MSG::FATAL << "No event processing manager specified. Check option:"
        << s_eventloop << endmsg;
    return StatusCode::FAILURE;
  }
  return m_processingMgr->nextEvent(maxevt);
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::stop()
//============================================================================
StatusCode ApplicationMgr::stop() {

  MsgStream log( m_messageSvc, name() );
  StatusCode sc;

  if( m_state == Gaudi::StateMachine::INITIALIZED ) {
    log << MSG::INFO << "Already Initialized!" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( m_state != Gaudi::StateMachine::RUNNING ) {
    log << MSG::FATAL
         << "stop: Invalid state \""  << m_state << "\"" << endmsg;
    return StatusCode::FAILURE;
  }
  m_targetState = Gaudi::StateMachine::INITIALIZED;

  // Stop independently managed Algorithms
  sc = algManager()->stop();
  if( !sc.isSuccess() ) return sc;

  //--------------------------------------------------------------------------
  // Stop the list of top Services
  //--------------------------------------------------------------------------
  sc = svcManager()->stop();
  if( !sc.isSuccess() ) return sc;

  //--------------------------------------------------------------------------
  // Final steps: Inform user and change internal state
  //--------------------------------------------------------------------------
  log << MSG::INFO << "Application Manager Stopped successfully"  << endmsg;
  m_state = m_targetState;

  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::finalize()
//============================================================================
StatusCode ApplicationMgr::finalize() {
  MsgStream log( m_messageSvc, name() );
  if( m_state == Gaudi::StateMachine::CONFIGURED ) {
    log << MSG::INFO << "Already Finalized" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( m_state != Gaudi::StateMachine::INITIALIZED ) {
    log << MSG::FATAL << "finalize: Invalid state \"" << m_state << "\""
        << endmsg;
    return StatusCode::FAILURE;
  }
  m_targetState = Gaudi::StateMachine::CONFIGURED;

  // disable message suppression in finalize
  m_svcLocator->service<IProperty>("MessageSvc")->setProperty(BooleanProperty("enableSuppression", false)).ignore();

  // Finalize independently managed Algorithms
  StatusCode sc = algManager()->finalize();
  if (sc.isFailure()) {
    log << MSG::WARNING << "Failed to finalize an algorithm." << endmsg;
    m_returnCode = Gaudi::ReturnCode::FinalizationFailure;
  }

  // Finalize all Services
  sc = svcManager()->finalize();
  if (sc.isFailure()) {
    log << MSG::WARNING << "Failed to finalize a service." << endmsg;
    m_returnCode = Gaudi::ReturnCode::FinalizationFailure;
  }

  //svcManager()->removeService( (IService*) m_processingMgr.get() );
  //svcManager()->removeService( (IService*) m_runable.get() );

  if (m_codeCheck) {
    StatusCode::disableChecking();
  }

  if (sc.isSuccess()) {
    log << MSG::INFO << "Application Manager Finalized successfully" << endmsg;
  } else {
    log << MSG::ERROR << "Application Manager failed to finalize" << endmsg;
  }

  m_state = m_targetState;
  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::terminate()
//============================================================================
StatusCode ApplicationMgr::terminate() {
  MsgStream log( m_messageSvc, name() );

  if( m_state == Gaudi::StateMachine::OFFLINE ) {
    log << MSG::INFO << "Already Offline" << endmsg;
    return StatusCode::SUCCESS;
  }
  else if( m_state != Gaudi::StateMachine::CONFIGURED ) {
    log << MSG::FATAL << "terminate: Invalid state \"" << m_state << "\""
        << endmsg;
    return StatusCode::FAILURE;
  }
  // release all Services
  m_targetState = Gaudi::StateMachine::OFFLINE;

  if (m_returnCode.value() == Gaudi::ReturnCode::Success) {
    log << MSG::INFO << "Application Manager Terminated successfully" << endmsg;
  } else if (m_returnCode.value() == Gaudi::ReturnCode::ScheduledStop ) {
    log << MSG::INFO << "Application Manager Terminated successfully with a user requested ScheduledStop" << endmsg;
  } else {
    log << MSG::ERROR << "Application Manager Terminated with error code " << m_returnCode.value() << endmsg;
  }

  { // Force a disable the auditing of finalize for MessageSvc
    SmartIF<IProperty> prop(m_messageSvc);
    if (prop.isValid()) {
      prop->setProperty(BooleanProperty("AuditFinalize", false)).ignore();
    }
  }
  { // Force a disable the auditing of finalize for JobOptionsSvc
    SmartIF<IProperty> prop(m_jobOptionsSvc);
    if (prop.isValid()) {
      prop->setProperty(BooleanProperty("AuditFinalize", false)).ignore();
    }
  }

  // finalize MessageSvc
  SmartIF<IService> svc(m_messageSvc);
  if ( !svc.isValid() ) {
    log << MSG::ERROR << "Could not get the IService interface of the MessageSvc" << endmsg;
  } else {
    svc->sysFinalize().ignore();
  }

  // finalize JobOptionsSvc
  svc = m_jobOptionsSvc;
  if ( !svc.isValid() ) {
    log << MSG::ERROR << "Could not get the IService interface of the JobOptionsSvc" << endmsg;
  } else {
    svc->sysFinalize().ignore();
  }

  m_state = m_targetState;
  return StatusCode::SUCCESS;
}

//============================================================================
// Reach the required state going through all the needed transitions
//============================================================================
StatusCode ApplicationMgr::GoToState(Gaudi::StateMachine::State state, bool ignoreFailures) {
  StatusCode sc = StatusCode(StatusCode::SUCCESS,true);

  switch (state) {

  case Gaudi::StateMachine::OFFLINE:
    switch (m_state) {
    case Gaudi::StateMachine::OFFLINE    : return StatusCode::SUCCESS; break;
    case Gaudi::StateMachine::CONFIGURED : return terminate(); break;
    default: // Gaudi::StateMachine::INITIALIZED or Gaudi::StateMachine::RUNNING
      sc = GoToState(Gaudi::StateMachine::CONFIGURED);
      if (sc.isSuccess()) {
        return terminate();
      } break;
    } break;

  case Gaudi::StateMachine::CONFIGURED:
    switch (m_state) {
    case Gaudi::StateMachine::CONFIGURED  : return StatusCode::SUCCESS; break;
    case Gaudi::StateMachine::OFFLINE     : return configure(); break;
    case Gaudi::StateMachine::INITIALIZED : return finalize(); break;
    default: // Gaudi::StateMachine::RUNNING
      sc = GoToState(Gaudi::StateMachine::INITIALIZED);
      if (sc.isSuccess()) {
        return finalize();
      } break;
    } break;

  case Gaudi::StateMachine::INITIALIZED:
    switch (m_state) {
    case Gaudi::StateMachine::INITIALIZED : return StatusCode::SUCCESS; break;
    case Gaudi::StateMachine::CONFIGURED  : return initialize(); break;
    case Gaudi::StateMachine::RUNNING     : return stop(); break;
    default: // Gaudi::StateMachine::OFFLINE
      sc = GoToState(Gaudi::StateMachine::CONFIGURED);
      if (sc.isSuccess()) {
        return initialize();
      } break;
    } break;

  case Gaudi::StateMachine::RUNNING:
    switch (m_state) {
    case Gaudi::StateMachine::RUNNING     : return StatusCode::SUCCESS; break;
    case Gaudi::StateMachine::INITIALIZED : return start(); break;
    default: // Gaudi::StateMachine::OFFLINE or Gaudi::StateMachine::CONFIGURED
      sc = GoToState(Gaudi::StateMachine::INITIALIZED);
      if (sc.isSuccess()) {
        return start();
      } break;
    } break;

  }

  // If I get here, there has been a problem in the recursion

  if (ignoreFailures){
    // force the new state
    m_state = state;
    return StatusCode::SUCCESS;
  }

  return sc;
}

//============================================================================
// IAppMgrUI implementation: ApplicationMgr::run()
//============================================================================
StatusCode ApplicationMgr::run() {
  StatusCode sc = StatusCode::SUCCESS;

  sc = GoToState(Gaudi::StateMachine::RUNNING);
  if ( sc.isSuccess() ) {
    MsgStream log(m_messageSvc, name());
    if ( m_runable != 0 ) { // loop over the events
      sc = m_runable->run();
      if ( !sc.isSuccess() ) {
        log << MSG::FATAL << "Application execution failed. Ending the job."
            << endmsg;
      }
    } else {
      log << MSG::FATAL << "Application has no runable object. Check option:"
          << s_runable << endmsg;
    }
  }
  if (sc.isSuccess()) { // try to close cleanly
    sc = GoToState(Gaudi::StateMachine::OFFLINE);
  }
  // either the runable failed of the stut-down
  if (sc.isFailure()) { // try to close anyway (but keep the StatusCode unchanged)
    GoToState(Gaudi::StateMachine::OFFLINE,true).ignore();
  }
  return sc;
}

//============================================================================
// IEventProcessor implementation: executeEvent(void* par)
//============================================================================
StatusCode ApplicationMgr::executeEvent(void* par)    {
  MsgStream log( m_messageSvc, name() );
  if( m_state == Gaudi::StateMachine::RUNNING ) {
    if ( m_processingMgr.isValid() )    {
      return m_processingMgr->executeEvent(par);
    }
  }
  log << MSG::FATAL << "executeEvent: Invalid state \"" << FSMState() << "\""
      <<endmsg;
  return StatusCode::FAILURE;
}

//============================================================================
// IEventProcessor implementation: executeRun(int)
//============================================================================
StatusCode ApplicationMgr::executeRun(int evtmax)    {
  MsgStream log( m_messageSvc, name() );
  if( m_state == Gaudi::StateMachine::RUNNING ) {
    if ( m_processingMgr.isValid() )    {
      return m_processingMgr->executeRun(evtmax);
    }
    log << MSG::WARNING << "No EventLoop Manager specified " << endmsg;
    return StatusCode::SUCCESS;
  }
  log << MSG::FATAL << "executeRun: Invalid state \"" << FSMState() << "\""
      << endmsg;
  return StatusCode::FAILURE;
}

//============================================================================
// IEventProcessor implementation: stopRun(int)
//============================================================================
StatusCode ApplicationMgr::stopRun()    {
  MsgStream log( m_messageSvc, name() );
  if( m_state == Gaudi::StateMachine::RUNNING ) {
    if ( m_processingMgr.isValid() )    {
      return m_processingMgr->stopRun();
    }
    log << MSG::WARNING << "No EventLoop Manager specified " << endmsg;
    return StatusCode::SUCCESS;
  }
  log << MSG::FATAL << "stopRun: Invalid state \"" << FSMState() << "\""
      << endmsg;
  return StatusCode::FAILURE;
}
// Implementation of IAppMgrUI::name
const std::string& ApplicationMgr::name() const {
  return m_name;
}

// implementation of IService::state
Gaudi::StateMachine::State ApplicationMgr::FSMState( ) const {
  return m_state;
}
// implementation of IService::state
Gaudi::StateMachine::State ApplicationMgr::targetFSMState( ) const {
  return m_targetState;
}


//============================================================================
// implementation of IService::reinitilaize
//============================================================================
StatusCode ApplicationMgr::reinitialize() {
  StatusCode retval = StatusCode::SUCCESS;
  StatusCode sc;
  if ( m_state < Gaudi::StateMachine::INITIALIZED ) {
    throw GaudiException("Cannot reinitialize application if not INITIALIZED or RUNNING",
                         "ApplicationMgr::reinitialize", StatusCode::FAILURE);
  }
  if ( m_state == Gaudi::StateMachine::RUNNING ) {
    retval = GoToState(Gaudi::StateMachine::INITIALIZED);
  }
  sc = svcManager()->reinitialize();
  if (sc.isFailure()) retval = sc;
  sc = algManager()->reinitialize();
  if (sc.isFailure()) retval = sc;
  return retval;
}

//============================================================================
// implementation of IService::reinitiaize
//============================================================================
StatusCode ApplicationMgr::restart() {
  StatusCode retval = StatusCode::SUCCESS;
  StatusCode sc;
  if ( m_state != Gaudi::StateMachine::RUNNING ) {
    throw GaudiException("Cannot restart application if not RUNNING",
                         "ApplicationMgr::restart", StatusCode::FAILURE);
  }
  sc = svcManager()->restart();
  if (sc.isFailure()) retval = sc;
  sc = algManager()->restart();
  if (sc.isFailure()) retval = sc;
  return retval;
}

//============================================================================
// SI Go Handler
//============================================================================
void ApplicationMgr::SIGoHandler( Property& ) {

  MsgStream log (m_messageSvc, name());
  StatusCode sc;

  // Re-initialize everything
  sc = reinitialize();
  // Execute a number of events
  executeRun(m_evtMax);

  return;
}

//============================================================================
// SI Exit Handler
//============================================================================
void ApplicationMgr::SIExitHandler( Property& ) {
  StatusCode status;
  status = finalize();
  status = terminate();
  ::exit( 0 );
}

//============================================================================
// Handle properties of the event loop manager (Top alg/Output stream list)
//============================================================================
void ApplicationMgr::evtLoopPropertyHandler( Property& p ) {
  if ( m_processingMgr.isValid() )    {
    SmartIF<IProperty> props(m_processingMgr);
    if ( props.isValid() )    {
      props->setProperty( p ).ignore();
    }
  }
}

//============================================================================
// External Service List handler
//============================================================================
void ApplicationMgr::createSvcNameListHandler( Property& /* theProp */ ) {
  if ( !(decodeCreateSvcNameList()).isSuccess() ) {
    throw GaudiException("Failed to create ext services",
                         "MinimalEventLoopMgr::createSvcNameListHandler",
                         StatusCode::FAILURE);
  }
}
//============================================================================
//  decodeCreateSvcNameList
//============================================================================
StatusCode ApplicationMgr::decodeCreateSvcNameList() {
  StatusCode result = StatusCode::SUCCESS;
  const std::vector<std::string>& theNames = m_createSvcNameList.value( );
  VectorName::const_iterator it(theNames.begin());
  VectorName::const_iterator et(theNames.end());
  while(result.isSuccess() && it != et) {
    Gaudi::Utils::TypeNameString item(*it++);
    if( (result = svcManager()->addService(item, ServiceManager::DEFAULT_SVC_PRIORITY) ).isFailure()) {
      MsgStream log( m_messageSvc, m_name );
      log << MSG::ERROR << "decodeCreateSvcNameList: Cannot create service "
          << item.type() << "/" << item.name() << endmsg;
    } else {
      ON_DEBUG {
        MsgStream log( m_messageSvc, m_name );
        log << MSG::DEBUG << "decodeCreateSvcNameList: Created service "
            << item.type() << "/" << item.name() << endmsg;
      }
    }
  }
  return result;
}

//============================================================================
// External Service List handler
//============================================================================
void ApplicationMgr::extSvcNameListHandler( Property& /* theProp */ ) {
  if ( !(decodeExtSvcNameList( )).isSuccess() ) {
    throw GaudiException("Failed to declare ext services",
                         "MinimalEventLoopMgr::extSvcNameListHandler",
                         StatusCode::FAILURE);
  }
}

//============================================================================
//  decodeExtSvcNameList
//============================================================================
StatusCode ApplicationMgr::decodeExtSvcNameList( ) {
  StatusCode result = StatusCode::SUCCESS;

  std::vector<std::string> theNames = m_extSvcNameList.value( );

  VectorName::const_iterator it(theNames.begin());
  VectorName::const_iterator et(theNames.end());
  while(result.isSuccess() && it != et) {
    Gaudi::Utils::TypeNameString item(*it++);
    if (m_extSvcCreates == true) {
      if ( (result = svcManager()->addService(item, ServiceManager::DEFAULT_SVC_PRIORITY)).isFailure()) {
        MsgStream log( m_messageSvc, m_name );
        log << MSG::ERROR << "decodeExtSvcNameList: Cannot create service "
            << item.type() << "/" << item.name() << endmsg;
      }
    } else {
      if( ( result = svcManager()->declareSvcType(item.name(),
                                                  item.type()) ).isFailure()) {
        MsgStream log( m_messageSvc, m_name );
        log << MSG::ERROR << "decodeExtSvcNameList: Cannot declare service "
            << item.type() << "/" << item.name() << endmsg;
      }
    }
  }
  return result;
}

//============================================================================
// External Service List handler
//============================================================================
void ApplicationMgr::multiThreadSvcNameListHandler( Property& /* theProp */ ) {
  if ( !(decodeMultiThreadSvcNameList( )).isSuccess() ) {
    throw GaudiException("Failed to create copies of mt services",
                         "MinimalEventLoopMgr::multiThreadSvcNameListHandler",
                         StatusCode::FAILURE);
  }

}

//============================================================================
//  decodeMultiExtSvcNameList
//============================================================================
StatusCode ApplicationMgr::decodeMultiThreadSvcNameList( ) {
  StatusCode result = StatusCode::SUCCESS;
  const std::vector<std::string>& theNames = m_multiThreadSvcNameList.value( );
  for(int iCopy=0; iCopy<m_noOfEvtThreads; ++iCopy) {
    for (VectorName::const_iterator it = theNames.begin();
         it != theNames.end();
         ++it) {
      Gaudi::Utils::TypeNameString item(*it);
      result = addMultiSvc(item, ServiceManager::DEFAULT_SVC_PRIORITY);
      //FIXME SHOULD CLONE?
      if( result.isFailure() ) {
        MsgStream log( m_messageSvc, m_name );
        log << MSG::ERROR
            << "decodeMultiThreadSvcNameList: Cannot create service "
            << item.type() << "/" << item.name() << endmsg;
      } else {
        ON_VERBOSE {
          MsgStream log( m_messageSvc, m_name );
          log << MSG::VERBOSE
              << "decodeMultiThreadSvcNameList: created service "
              << item.type() << "/" << item.name() << endmsg;
        }
      }
    }
  }
  return result;
}
//=============================================================================
//  declareMultiSvcType
//=============================================================================
StatusCode ApplicationMgr::declareMultiSvcType(const std::string& name,
                                               const std::string& type) {
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( m_messageSvc, m_name );
  if (0 == m_noOfEvtThreads) {
    result = svcManager()->declareSvcType(name, type);
    if( result.isFailure() ) {
      log << MSG::ERROR << "declareMultiSvcType: Cannot declare service "
          << type << "/" << name << endmsg;
    } else {
      ON_VERBOSE
        log << MSG::VERBOSE << "declareMultiSvcType: declared service "
            << type << "/" << name << endmsg;
    }
  } else {
    for(int iCopy=0; iCopy<m_noOfEvtThreads; ++iCopy) {
      std::string thrName(name + getGaudiThreadIDfromID(iCopy));
      result = svcManager()->declareSvcType(thrName, type);
      if( result.isFailure() ) {
        log << MSG::ERROR << "declareMultiSvcType: Cannot declare service "
            << type << "/" << thrName << endmsg;
      } else {
        ON_VERBOSE
          log << MSG::VERBOSE << "declareMultiSvcType: declared service "
              << type << "/" << thrName << endmsg;
      }
    }
  }
  return result;
}
//=============================================================================
//  addMultiSvc
//=============================================================================
StatusCode ApplicationMgr::addMultiSvc(const Gaudi::Utils::TypeNameString &typeName,
                                       int prio) {
  using Gaudi::Utils::TypeNameString;
  StatusCode result = StatusCode::SUCCESS;
  MsgStream log( m_messageSvc, m_name );
  if (0 == m_noOfEvtThreads) {
    result = svcManager()->addService(typeName, prio);
    // result = svcManager()->addService(name, type, prio); // CHECKME???
    if( result.isFailure() ) {
      log << MSG::ERROR << "addMultiSvc: Cannot add service "
          << typeName.type() << "/" << typeName.name() << endmsg;
    } else {
      ON_VERBOSE
        log << MSG::VERBOSE << "addMultiSvc: added service "
            << typeName.type() << "/" << typeName.name() << endmsg;
    }
  } else {
    for(int iCopy=0; iCopy<m_noOfEvtThreads; ++iCopy) {
      const std::string &type = typeName.type();
      std::string thrName(typeName.name() + getGaudiThreadIDfromID(iCopy));
      result = svcManager()->addService(TypeNameString(thrName, type), prio);
      if( result.isFailure() ) {
        log << MSG::ERROR << "addMultiSvc: Cannot add service "
            << type << "/" << thrName << endmsg;
      } else {
        ON_VERBOSE
          log << MSG::VERBOSE << "addMultiSvc: added service "
              << type << "/" << thrName << endmsg;
      }
    }
  }
  return result;
}

//============================================================================
// Dll List handler
//============================================================================
void ApplicationMgr::dllNameListHandler( Property& /* theProp */ ) {
  if ( !(decodeDllNameList( )).isSuccess() ) {
    throw GaudiException("Failed to load DLLs.",
                         "MinimalEventLoopMgr::dllNameListHandler",
                         StatusCode::FAILURE);
  }
}

//============================================================================
//  decodeDllNameList
//============================================================================
StatusCode ApplicationMgr::decodeDllNameList() {

  MsgStream log( m_messageSvc, m_name );
  StatusCode result = StatusCode::SUCCESS;

  // Clean up multiple entries from DLL list
  // -------------------------------------------------------------------------
  std::vector<std::string> newList;
  std::map<std::string,unsigned int> dllInList, duplicateList;
  {for ( std::vector<std::string>::const_iterator it = m_dllNameList.value().begin();
        it != m_dllNameList.value().end(); ++it ) {
    if ( 0 == dllInList[*it] ) {
      newList.push_back(*it);        // first instance of this module
    } else { ++duplicateList[*it]; } // module listed multiple times
    ++dllInList[*it];                // increment count for this module
  }}
  //m_dllNameList = newList; // update primary list to new, filtered list (do not use the
                             // property itself otherwise we get called again infinitely)
  // List modules that were in there twice..
  ON_DEBUG if ( !duplicateList.empty() ) {
    log << MSG::DEBUG << "Removed duplicate entries for modules : ";
    for ( std::map<std::string,unsigned int>::const_iterator it = duplicateList.begin();
          it != duplicateList.end(); ++it ) {
      log << it->first << "(" << 1+it->second << ")";
      if ( it != --duplicateList.end() ) log << ", ";
    }
    log << endmsg;
  }
  // -------------------------------------------------------------------------

  const std::vector<std::string>& theNames = newList;

  // only load the new dlls or previously failed dlls
  ON_DEBUG log << MSG::DEBUG << "Loading declared DLL's" << endmsg;

  std::vector<std::string> successNames, failNames;
  std::vector<std::string>::const_iterator it;

  for (it = theNames.begin(); it != theNames.end(); it++) {
    if (std::find (m_okDlls.rbegin(), m_okDlls.rend(), *it) == m_okDlls.rend()){
      // found a new module name
      StatusCode status = m_classManager->loadModule( (*it) );
      if( status.isFailure() ) {
        failNames.push_back(*it);
        result = StatusCode::FAILURE;
      }
      else {
        successNames.push_back(*it);
      }
    }
  }

  // report back to the user and store the names of the succesfully loaded dlls
  if ( !successNames.empty() ) {
    log << MSG::INFO << "Successfully loaded modules : ";
    for (it = successNames.begin(); it != successNames.end(); it++) {
      log<< (*it);
      if( (it+1) != successNames.end())  log << ", ";
      // save name
      m_okDlls.push_back( *it );
    }
    log << endmsg;
  }

  if ( result == StatusCode::FAILURE ) {
    log << MSG::WARNING << "Failed to load modules: ";
    for (it = failNames.begin(); it != failNames.end(); it++) {
      log<< (*it);
      if( (it+1) != failNames.end())  log << ", ";
    }
    log << endmsg;
  }
  return result;
}

//============================================================================
// Reflex debug level handler
//============================================================================
void ApplicationMgr::reflexDebugPropertyHandler( Property& )
{
  // Setup debug level for Reflex plugin system
  MsgStream log (m_messageSvc, name());
  log << MSG::INFO
      << "Updating ROOT::Reflex::PluginService::SetDebug(level) to level="
      << (int)m_reflexDebugLevel
      << endmsg;
  ROOT::Reflex::PluginService::SetDebug(m_reflexDebugLevel);
}

//============================================================================
// Reflex debug level handler
//============================================================================
void ApplicationMgr::initLoopCheckHndlr(Property&) {
  svcManager()->setLoopCheckEnabled(m_loopCheck);
}

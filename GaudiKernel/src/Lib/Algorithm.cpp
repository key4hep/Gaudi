#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IProperty.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// Constructor
Algorithm::Algorithm( const std::string& name, ISvcLocator *pSvcLocator,
                      const std::string& version)
  : m_event_context(nullptr),
    m_name(name),
    m_version(version),
    m_index(123), // FIXME: to be fixed in the algorithmManager
    m_registerContext ( false ) ,
    m_pSvcLocator(pSvcLocator),
    m_filterPassed(true),
    m_isEnabled(true),
    m_isExecuted(false),
    m_toolHandlesInit(false),
    m_state(Gaudi::StateMachine::CONFIGURED),
    m_targetState(Gaudi::StateMachine::CONFIGURED)
{
  m_propertyMgr = new PropertyMgr();
  m_subAlgms = new std::vector<Algorithm *>();

  // Declare common Algorithm properties with their defaults
  declareProperty( "OutputLevel",        m_outputLevel = MSG::NIL);
  declareProperty( "Enable",             m_isEnabled = true);
  declareProperty( "ErrorMax",           m_errorMax  = 1);
  declareProperty( "ErrorCounter",       m_errorCount = 0);

  //declare input and output properties
  declareProperty( "Inputs", m_inputDataObjects);
  declareProperty( "Outputs", m_outputDataObjects);

  // Auditor monitoring properties

  // Get the default setting for service auditing from the AppMgr
  declareProperty( "AuditAlgorithms", m_auditInit );

  bool audit(false);
  SmartIF<IProperty> appMgr(serviceLocator()->service("ApplicationMgr"));
  if (appMgr.isValid()) {
    const Property& prop = appMgr->getProperty("AuditAlgorithms");
    Property &pr = const_cast<Property&>(prop);
    if (m_name != "IncidentSvc") {
      setProperty( pr ).ignore();
    }
    audit = m_auditInit.value();
  }

  declareProperty( "AuditInitialize"  , m_auditorInitialize   = audit ) ;
  declareProperty( "AuditReinitialize", m_auditorReinitialize = audit ) ;
  declareProperty( "AuditRestart"     , m_auditorRestart      = audit ) ;
  declareProperty( "AuditExecute"     , m_auditorExecute      = audit ) ;
  declareProperty( "AuditFinalize"    , m_auditorFinalize     = audit ) ;
  declareProperty( "AuditBeginRun"    , m_auditorBeginRun     = audit ) ;
  declareProperty( "AuditEndRun"      , m_auditorEndRun       = audit ) ;
  declareProperty( "AuditStart"       , m_auditorStart        = audit ) ;
  declareProperty( "AuditStop"        , m_auditorStop         = audit ) ;

  declareProperty( "MonitorService"   , m_monitorSvcName      = "MonitorSvc" );

  declareProperty
    ( "RegisterForContextService" ,
      m_registerContext  ,
      "The flag to enforce the registration for Algorithm Context Service") ;

  declareProperty( "IsClonable"       , m_isClonable = false, "Thread-safe enough for cloning?" );
  declareProperty( "Cardinality"      , m_cardinality = 1,    "How many clones to create" );
  declareProperty( "NeededResources"  , m_neededResources = std::vector<std::string>() );

  // update handlers.
  m_outputLevel.declareUpdateHandler(&Algorithm::initOutputLevel, this);
}

// Default Destructor
Algorithm::~Algorithm() {
  delete m_subAlgms;
  delete m_propertyMgr;
}

// IAlgorithm implementation
StatusCode Algorithm::sysInitialize() {

  MsgStream log ( msgSvc() , name() ) ;

  // Bypass the initialization if the algorithm
  // has already been initialized.
  if ( Gaudi::StateMachine::INITIALIZED <= FSMState() ) return StatusCode::SUCCESS;

  // Set the Algorithm's properties
  StatusCode sc = setProperties();
  if( sc.isFailure() ) return StatusCode::FAILURE;

  // Bypass the initialization if the algorithm is disabled.
  // Need to do this after setProperties.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::INITIALIZE,m_state);

  // Check current outputLevel to eventually inform the MessagsSvc
  //if( m_outputLevel != MSG::NIL ) {
  setOutputLevel( m_outputLevel );
  //}

  // TODO: (MCl) where shoud we do this? initialize or start?
  // Reset Error count
  //m_errorCount = 0;

  // lock the context service
//   Gaudi::Utils::AlgContext cnt
//     ( this , registerContext() ? contextSvc().get() : 0 ) ;

  // Get WhiteBoard interface if implemented by EventDataSvc
  m_WB = service("EventDataSvc");

  //check whether timeline should be done
  m_doTimeline = timelineSvc()->isEnabled();

  // Invoke initialize() method of the derived class inside a try/catch clause
  try {

    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard
        ( this,
          // check if we want to audit the initialize
          (m_auditorInitialize) ? auditorSvc().get() : 0,
          IAuditor::Initialize);
      // Invoke the initialize() method of the derived class
      sc = initialize();
    }

    if( sc.isSuccess() ) {
      // Now initialize care of any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if ((*it)->sysInitialize().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR << " Error initializing one or several sub-algorithms"
	    << endmsg;
      } else {
	// Update the state.
	m_state = m_targetState;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  // If Cardinality is 0, bring the value to 1
  if (m_cardinality == 0 ) {
    m_cardinality = 1;
  }
  // Set IsClonable to true if the Cardinality is greater than one
  else if (m_cardinality > 1 ) {
    m_isClonable = true;
  }


  //init data handle
  for (auto tag : m_inputDataObjects) {
    if (m_inputDataObjects[tag].isValid()) {
      if (m_inputDataObjects[tag].initialize().isSuccess())
        log << MSG::DEBUG << "Data Handle " << tag << " ("
            << m_inputDataObjects[tag].dataProductName()
            << ") initialized" << endmsg;
      else
        log << MSG::FATAL << "Data Handle " << tag << " ("
            << m_inputDataObjects[tag].dataProductName()
            << ") could NOT be initialized" << endmsg;
    }
  }
  for (auto tag : m_outputDataObjects) {
    if (m_outputDataObjects[tag].isValid()) {
      if (m_outputDataObjects[tag].initialize().isSuccess())
        log << MSG::DEBUG << "Data Handle " << tag << " ("
            << m_outputDataObjects[tag].dataProductName()
            << ") initialized" << endmsg;
      else
        log << MSG::FATAL << "Data Handle " << tag << " ("
            << m_outputDataObjects[tag].dataProductName()
            << ") could NOT be initialized" << endmsg;
    }
  }

  std::set<const IInterface*> visited_parents;
  //all TES accessing tools have been created in initialize() of derived class
  //query toolSvc for own tools and build dependencies
  std::function<void(const IInterface *)> addToolDOD = [&] (const IInterface * parent) {

    MsgStream log ( msgSvc() , name() ) ;

    if (visited_parents.find(parent) != end(visited_parents)) {
      log << MSG::DEBUG << "parent object already visited: stopping recursion"
          << endmsg;
      return;
    }
    visited_parents.insert(parent);

    std::vector<IAlgTool *> tools;

    const Algorithm * alg = dynamic_cast<const Algorithm *>(parent);
    if(alg != NULL)
      tools = alg->tools();
    else{
      const AlgTool * algTool = dynamic_cast<const AlgTool *>(parent);
      if(algTool != NULL)
        tools = algTool->tools();
      else
        log << MSG::FATAL << "Could not build data dependencies of algorithm, wrong parameter" << endmsg;
    }

    for(auto tool : tools){

      log << MSG::DEBUG << "Adding data dependencies for tool " << tool->name() << " (" << tool->type() << ")" << endmsg;
      auto inputs = tool->inputDataObjects();
      for(auto dod : inputs){
        log << MSG::DEBUG << "\tInput: " << dod << endmsg;
        m_inputDataObjects.insert(&inputs[dod]);
      }

      auto outputs = tool->outputDataObjects();
      for(auto dod : outputs){
        log << MSG::DEBUG << "\tOutput: " << dod << endmsg;
        m_outputDataObjects.insert(&outputs[dod]);
      }

      addToolDOD(tool);
    }
  };

  log << MSG::DEBUG << "Adding tools for " << this->name() << endmsg;
  addToolDOD(this);

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysStart() {

  // Bypass the startup if already running or disabled.
  if ( Gaudi::StateMachine::RUNNING == FSMState() ||
       !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::START,m_state);

  // TODO: (MCl) where shoud we do this? initialize or start?
  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke start() method of the derived class inside a try/catch clause
  try
  {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard
        (this,
         // check if we want to audit the initialize
         (m_auditorStart) ? auditorSvc().get() : 0,
         IAuditor::Start);
      // Invoke the start() method of the derived class
      sc = start();
    }
    if( sc.isSuccess() ) {

      // Now start any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if ((*it)->sysStart().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR << " Error starting one or several sub-algorithms"
	    << endmsg;
      } else {
	// Update the state.
	m_state = m_targetState;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStart(): exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStart(): standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStart(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysReinitialize() {

  // Bypass the initialization if the algorithm is disabled.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR
        << "sysReinitialize(): cannot reinitialize algorithm not initialized"
        << endmsg;
    return StatusCode::FAILURE;
  }

  // Check current outputLevel to evetually inform the MessagsSvc
  //if( m_outputLevel != MSG::NIL ) {
  setOutputLevel( m_outputLevel );
  //}

  // Reset Error count
  // m_errorCount = 0; // done during start

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::SUCCESS);
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorReinitialize) ? auditorSvc().get() : 0,
                                        IAuditor::ReInitialize);
      // Invoke the reinitialize() method of the derived class
      sc = reinitialize();
    }
    if( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if((*it)->sysReinitialize().isFailure()) fail = true;
      }

      if (fail) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR
	    << "sysReinitialize(): Error reinitializing one or several "
	    << "sub-algorithms" << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysReinitialize(): Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysReinitialize(): Standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysReinitialize(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysRestart() {

  // Bypass the initialization if the algorithm is disabled.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR
        << "sysRestart(): cannot restart algorithm not started"
        << endmsg;
    return StatusCode::FAILURE;
  }

  // Check current outputLevel to evetually inform the MessagsSvc
  //if( m_outputLevel != MSG::NIL ) {
  setOutputLevel( m_outputLevel );
  //}

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorRestart) ? auditorSvc().get() : 0,
                                        IAuditor::ReStart);
      // Invoke the reinitialize() method of the derived class
      sc = restart();
    }
    if( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if ((*it)->sysRestart().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR
	    << "sysRestart(): Error restarting one or several sub-algorithms"
	    << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysRestart(): Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysRestart(): Standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "sysRestart(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysBeginRun() {

  // Bypass the beginRun if the algorithm is disabled.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  // Check current outputLevel to evetually inform the MessagsSvc
  //if( m_outputLevel != MSG::NIL ) {
  setOutputLevel( m_outputLevel );
  //}

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke beginRun() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorBeginRun) ? auditorSvc().get() : 0,
                                        IAuditor::BeginRun);
      // Invoke the beginRun() method of the derived class
      sc = beginRun();
    }
    if( sc.isSuccess() ) {

      // Now call beginRun for any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if((*it)->sysBeginRun().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR << " Error executing BeginRun for one or several sub-algorithms"
          << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }
  return sc;
}

StatusCode Algorithm::beginRun() {
  return StatusCode::SUCCESS;
}

// IAlgorithm implementation
StatusCode Algorithm::sysEndRun() {

  // Bypass the endRun if the algorithm is disabled.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  // Check current outputLevel to eventually inform the MessagsSvc
  //if( m_outputLevel != MSG::NIL ) {
  setOutputLevel( m_outputLevel );
  //}

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  // Invoke endRun() method of the derived class inside a try/catch clause
  StatusCode sc(StatusCode::FAILURE);
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorEndRun) ? auditorSvc().get() : 0,
                                        IAuditor::EndRun);
      // Invoke the endRun() method of the derived class
      sc = endRun();
    }
    if( sc.isSuccess() ) {

      // Now call endRun for any sub-algorithms
      std::vector<Algorithm *>::iterator it;
      bool fail(false);
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	if ((*it)->sysEndRun().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	MsgStream log ( msgSvc() , name() );
	log << MSG::ERROR << " Error calling endRun for one or several sub-algorithms"
	    << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }
  return sc;
}

StatusCode Algorithm::endRun() {
  return StatusCode::SUCCESS;
}


StatusCode Algorithm::sysExecute() {
  if (!isEnabled()) {
    if (UNLIKELY(m_outputLevel <= MSG::VERBOSE)) {
      MsgStream log ( msgSvc() , name() );
      log << MSG::VERBOSE << ".sysExecute(): is not enabled. Skip execution" << endmsg;
    }
    return StatusCode::SUCCESS;
  }

  StatusCode status;

  // Should performance profile be performed ?
  // invoke execute() method of Algorithm class
  //   and catch all uncaught exceptions

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  // HiveWhiteBoard stuff here
  if(m_WB.isValid()) m_WB->selectStore(getContext() ? getContext()->slot() : 0).ignore();

  Gaudi::Guards::AuditorGuard guard(this,
                                    // check if we want to audit the initialize
                                    (m_auditorExecute) ? auditorSvc().get() : 0,
                                    IAuditor::Execute,
                                    status);

  TimelineEvent timeline;
  timeline.algorithm = this->name();
  //  timeline.thread = getContext() ? getContext()->m_thread_id : 0;
  timeline.slot = getContext() ? getContext()->slot() : 0;
  timeline.event = getContext() ? getContext()->evt() : 0;

  try {

	if(UNLIKELY(m_doTimeline))
		  timeline.start = Clock::now();
    status = execute();

    if(UNLIKELY(m_doTimeline))
    	timeline.end = Clock::now();

    setExecuted(true);  // set the executed flag

    if (status.isFailure()) {
      status = exceptionSvc()->handleErr(*this,status);
    }

  }
  catch( const GaudiException& Exception )
  {
    setExecuted(true);  // set the executed flag

    MsgStream log ( msgSvc() , name() );
    if (Exception.code() == StatusCode::FAILURE) {
      log << MSG::FATAL;
    } else {
      log << MSG::ERROR << " Recoverable";
    }

    log << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;

    log << MSG::ERROR << Exception  << endmsg;

    //Stat stat( chronoSvc() , Exception.tag() ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch( const std::exception& Exception )
  {
    setExecuted(true);  // set the executed flag

    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    //Stat stat( chronoSvc() , "*std::exception*" ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch(...)
  {
    setExecuted(true);  // set the executed flag

    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    //Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;

    status = exceptionSvc()->handle(*this);
  }

  if(UNLIKELY(m_doTimeline))
	  timelineSvc()->registerTimelineEvent(timeline);

  if( status.isFailure() ) {
    MsgStream log ( msgSvc() , name() );
    // Increment the error count
    m_errorCount++;
    // Check if maximum is exeeded
    if( m_errorCount < m_errorMax ) {
      log << MSG::WARNING << "Continuing from error (cnt=" << m_errorCount
          << ", max=" << m_errorMax << ")" << endmsg;
      // convert to success
      status = StatusCode::SUCCESS;
    }
  }
  return status;
}

// IAlgorithm implementation
StatusCode Algorithm::sysStop() {

  // Bypass the startup if already running or disabled.
  if ( Gaudi::StateMachine::INITIALIZED == FSMState() ||
       !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::STOP,m_state);

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke stop() method of the derived class inside a try/catch clause
  try {
    // Stop first any sub-algorithms (in reverse order)
    std::vector<Algorithm *>::iterator it;
    for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
      (*it)->sysStop().ignore();
    }
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorStop) ? auditorSvc().get() : 0,
                                        IAuditor::Stop);

      // Invoke the stop() method of the derived class
      sc = stop();
    }
    if( sc.isSuccess() ) {
      // Update the state.
      m_state = m_targetState;
    }
  }
  catch ( const GaudiException& Exception )  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStop(): exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStop(): standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "in sysStop(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

StatusCode Algorithm::sysFinalize() {

  // Bypass the finalialization if the algorithm hasn't been initilized.
  if ( Gaudi::StateMachine::CONFIGURED == FSMState() ||
       !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::FINALIZE,m_state);

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke finalize() method of the derived class inside a try/catch clause
  try {
    // Order changed (bug #3903 overview: finalize and nested algorithms)
    // Finalize first any sub-algoithms (it can be done more than once)
    std::vector<Algorithm *>::iterator it;
    bool fail(false);
    for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
      if (!(*it)->sysFinalize().isSuccess()) {
        fail = true;
      }
    }

    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorFinalize) ? auditorSvc().get() : 0,
                                        IAuditor::Finalize);
      // Invoke the finalize() method of the derived class
      sc = finalize();
    }
    if (fail) sc = StatusCode::FAILURE;

    if( sc.isSuccess() ) {

      // Release all sub-algorithms
      for (it = m_subAlgms->begin(); it != m_subAlgms->end(); it++) {
	(*it)->release();
      }
      // Indicate that this Algorithm has been finalized to prevent duplicate attempts
      m_state = m_targetState;
    }
  }
  catch( const GaudiException& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() ) ;
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" ) ;
    sc = StatusCode::FAILURE;
  }
  catch( ... )
  {
    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }
  return sc;
}

StatusCode Algorithm::reinitialize() {
  /* @TODO
   * MCl 2008-10-23: the implementation of reinitialize as finalize+initialize
   *                 is causing too many problems
   *
  // Default implementation is finalize+initialize
  StatusCode sc = finalize();
  if (sc.isFailure()) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR << "reinitialize(): cannot be finalized" << endmsg;
    return sc;
  }
  sc = initialize();
  if (sc.isFailure()) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR << "reinitialize(): cannot be initialized" << endmsg;
    return sc;
  }
  */
  return StatusCode::SUCCESS;
}

StatusCode Algorithm::restart() {
  // Default implementation is stop+start
  StatusCode sc = stop();
  if (sc.isFailure()) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR << "restart(): cannot be stopped" << endmsg;
    return sc;
  }
  sc = start();
  if (sc.isFailure()) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR << "restart(): cannot be started" << endmsg;
    return sc;
  }
  return StatusCode::SUCCESS;
}

const std::string& Algorithm::name() const {
  return m_name;
}

const std::string& Algorithm::version() const {
  return m_version;
}

unsigned int Algorithm::index() {
  return m_index;
}

bool Algorithm::isExecuted() const {
  return m_isExecuted;
}

void Algorithm::setExecuted( bool state ) {
  m_isExecuted = state;
}

void Algorithm::resetExecuted() {
  m_isExecuted   = false;
  m_filterPassed = true;
}

bool Algorithm::isEnabled() const {
  return m_isEnabled;
}

bool Algorithm::filterPassed() const {
  return m_filterPassed;
}

void Algorithm::setFilterPassed( bool state ) {
  m_filterPassed = state;
}

std::vector<Algorithm*>* Algorithm::subAlgorithms( ) const {
  return m_subAlgms;
}

void Algorithm::setOutputLevel( int level ) {
  if ( msgSvc() != 0 )
  {
    if ( MSG::NIL != level )
    { msgSvc()->setOutputLevel( name(), level ) ; }
    m_outputLevel = msgSvc()->outputLevel( name() );
  }
}

#define serviceAccessor(METHOD,INTERFACE,NAME,MEMBER) \
SmartIF<INTERFACE>& Algorithm::METHOD() const { \
  if ( !MEMBER.isValid() ) { \
    MEMBER = service(NAME); \
    if( !MEMBER.isValid() ) { \
      throw GaudiException("Service [" NAME  "] not found", name(), StatusCode::FAILURE); \
    } \
  } \
  return MEMBER; \
}

//serviceAccessor(msgSvc, IMessageSvc, "MessageSvc", m_MS)
// Message service needs a special treatment to avoid infinite recursion
SmartIF<IMessageSvc>& Algorithm::msgSvc() const {
  if ( !m_MS.isValid() ) {
    //can not use service() method (infinite recursion!)
    m_MS = serviceLocator(); // default message service
    if( !m_MS.isValid() ) {
      throw GaudiException("Service [MessageSvc] not found", name(), StatusCode::FAILURE);
    }
  }
  return m_MS;
}

serviceAccessor(auditorSvc, IAuditorSvc, "AuditorSvc", m_pAuditorSvc)
serviceAccessor(chronoSvc, IChronoStatSvc, "ChronoStatSvc", m_CSS)
serviceAccessor(detSvc, IDataProviderSvc, "DetectorDataSvc", m_DDS)
serviceAccessor(detCnvSvc, IConversionSvc, "DetectorPersistencySvc", m_DCS)
serviceAccessor(eventSvc, IDataProviderSvc, "EventDataSvc", m_EDS)
serviceAccessor(whiteboard, IHiveWhiteBoard, "EventDataSvc", m_WB)
serviceAccessor(eventCnvSvc, IConversionSvc, "EventPersistencySvc", m_ECS)
serviceAccessor(histoSvc, IHistogramSvc, "HistogramDataSvc", m_HDS)
serviceAccessor(exceptionSvc, IExceptionSvc, "ExceptionSvc", m_EXS)
serviceAccessor(ntupleSvc, INTupleSvc, "NTupleSvc", m_NTS)
//serviceAccessor(atupleSvc, IAIDATupleSvc, "AIDATupleSvc", m_ATS)
serviceAccessor(randSvc, IRndmGenSvc, "RndmGenSvc", m_RGS)
serviceAccessor(toolSvc, IToolSvc, "ToolSvc", m_ptoolSvc)
serviceAccessor(contextSvc, IAlgContextSvc,"AlgContextSvc", m_contextSvc)
serviceAccessor(timelineSvc, ITimelineSvc,"TimelineSvc", m_timelineSvc)


// Obsoleted name, kept due to the backwards compatibility
SmartIF<IChronoStatSvc>& Algorithm::chronoStatService() const {
  return chronoSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IDataProviderSvc>& Algorithm::detDataService() const {
  return detSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IConversionSvc>& Algorithm::detDataCnvService() const {
  return detCnvSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IDataProviderSvc>& Algorithm::eventDataService() const {
  return eventSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IConversionSvc>& Algorithm::eventDataCnvService() const {
  return eventCnvSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IHistogramSvc>& Algorithm::histogramDataService() const {
  return histoSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<IMessageSvc>& Algorithm::messageService() const {
  return msgSvc();
}
// Obsoleted name, kept due to the backwards compatibility
SmartIF<INTupleSvc>& Algorithm::ntupleService() const {
  return ntupleSvc();
}

#if 0
IAuditorSvc* Algorithm::auditorSvc() const {
  if ( 0 == m_pAuditorSvc ) {
    StatusCode sc = service( "AuditorSvc", m_pAuditorSvc, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [AuditorSvc] not found", name(), sc);
    }
  }
  return m_pAuditorSvc;
}

IChronoStatSvc* Algorithm::chronoSvc() const {
  if ( 0 == m_CSS ) {
    StatusCode sc = service( "ChronoStatSvc", m_CSS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [ChronoStatSvc] not found", name(), sc);
    }
  }
  return m_CSS;
}

IDataProviderSvc* Algorithm::detSvc() const {
  if ( 0 == m_DDS ) {
    StatusCode sc = service( "DetectorDataSvc", m_DDS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [DetectorDataSvc] not found", name(), sc);
    }
  }
  return m_DDS;
}

IConversionSvc* Algorithm::detCnvSvc() const {
  if ( 0 == m_DCS ) {
    StatusCode sc = service( "DetectorPersistencySvc", m_DCS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [DetectorPersistencySvc] not found",
                           name(), sc);
    }
  }
  return m_DCS;
}

IDataProviderSvc* Algorithm::eventSvc() const {
  if ( 0 == m_EDS ) {
    StatusCode sc = service( "EventDataSvc", m_EDS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [EventDataSvc] not found", name(), sc);
    }
  }
  return m_EDS;
}

IConversionSvc* Algorithm::eventCnvSvc() const {
  if ( 0 == m_ECS ) {
    StatusCode sc = service( "EventPersistencySvc", m_ECS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [EventPersistencySvc] not found",
                           name(), sc);
    }
  }
  return m_ECS;
}

IHistogramSvc* Algorithm::histoSvc() const {
  if ( 0 == m_HDS ) {
    StatusCode sc = service( "HistogramDataSvc", m_HDS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [HistogramDataSvc] not found", name(), sc);
    }
  }
  return m_HDS;
}

IExceptionSvc* Algorithm::exceptionSvc() const {
  if ( 0 == m_EXS ) {
    StatusCode sc = service( "ExceptionSvc", m_EXS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [ExceptionSvc] not found", name(), sc);
    }
  }
  return m_EXS;
}

IMessageSvc* Algorithm::msgSvc() const {
  if ( 0 == m_MS ) {
    //can not use service() method (infinite recursion!)
    StatusCode sc = serviceLocator()->service( "MessageSvc", m_MS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [MessageSvc] not found", name(), sc);
    }
  }
  return m_MS;
}

INTupleSvc* Algorithm::ntupleSvc() const {
  if ( 0 == m_NTS ) {
    StatusCode sc = service( "NTupleSvc", m_NTS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [NTupleSvc] not found", name(), sc);
    }
  }
  return m_NTS;
}

// AIDATupleSvc:
// IAIDATupleSvc* Algorithm::atupleSvc() const {
//   if ( 0 == m_ATS ) {
//     StatusCode sc = service( "AIDATupleSvc", m_ATS, true );
//     if( sc.isFailure() ) {
//       throw GaudiException("Service [AIDATupleSvc] not found", name(), sc);
//     }
//   }
//   return m_ATS;
// }

IRndmGenSvc* Algorithm::randSvc() const {
  if ( 0 == m_RGS ) {
    StatusCode sc = service( "RndmGenSvc", m_RGS, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [RndmGenSvc] not found", name(), sc);
    }
  }
  return m_RGS;
}

IToolSvc* Algorithm::toolSvc() const {
  if ( 0 == m_ptoolSvc ) {
    StatusCode sc = service( "ToolSvc", m_ptoolSvc, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [ToolSvc] not found", name(), sc);
    }
  }
  return m_ptoolSvc;
}
#endif

SmartIF<ISvcLocator>& Algorithm::serviceLocator() const {
  return *const_cast<SmartIF<ISvcLocator>*>(&m_pSvcLocator);
}

// Use the job options service to set declared properties
StatusCode Algorithm::setProperties() {
  if( m_pSvcLocator != 0 )    {
    SmartIF<IJobOptionsSvc> jos(m_pSvcLocator->service("JobOptionsSvc"));
    if( jos.isValid() ) {
      // set first generic Properties
      StatusCode sc = jos->setMyProperties( getGaudiThreadGenericName(name()), this );
      if( sc.isFailure() ) return StatusCode::FAILURE;

      // set specific Properties
      if (isGaudiThreaded(name())) {
        if(jos->setMyProperties( name(), this ).isFailure()) {
          return StatusCode::FAILURE;
        }
      }
      return sc;
    }
  }
  return StatusCode::FAILURE;
}

StatusCode Algorithm::createSubAlgorithm(const std::string& type,
                                         const std::string& name,
                                         Algorithm*& pSubAlgorithm) {
  if( m_pSvcLocator == 0 ) return StatusCode::FAILURE;

  SmartIF<IAlgManager> am(m_pSvcLocator);
  if ( !am.isValid() ) return StatusCode::FAILURE;

  // Maybe modify the AppMgr interface to return Algorithm* ??
  IAlgorithm *tmp;
  StatusCode sc = am->createAlgorithm
    (type, name+getGaudiThreadIDfromName(Algorithm::name()), tmp);
  if( sc.isFailure() ) return StatusCode::FAILURE;

  try{
    pSubAlgorithm = dynamic_cast<Algorithm*>(tmp);
    m_subAlgms->push_back(pSubAlgorithm);
  } catch(...){
    sc = StatusCode::FAILURE;
  }
  return sc;
}

// IProperty implementation
// Delegate to the Property manager
StatusCode Algorithm::setProperty(const Property& p) {
  return m_propertyMgr->setProperty(p);
}
StatusCode Algorithm::setProperty(const std::string& s) {
  return m_propertyMgr->setProperty(s);
}
StatusCode Algorithm::setProperty(const std::string& n, const std::string& v) {
  return m_propertyMgr->setProperty(n,v);
}
StatusCode Algorithm::getProperty(Property* p) const {
  return m_propertyMgr->getProperty(p);
}
const Property& Algorithm::getProperty( const std::string& name) const{
  return m_propertyMgr->getProperty(name);
}
StatusCode Algorithm::getProperty(const std::string& n, std::string& v ) const {
  return m_propertyMgr->getProperty(n,v);
}
const std::vector<Property*>& Algorithm::getProperties( ) const {
  return m_propertyMgr->getProperties();
}
bool Algorithm::hasProperty(const std::string& name) const {
  return m_propertyMgr->hasProperty(name);
}

const std::vector<MinimalDataObjectHandle*> Algorithm::handles(){

	std::vector<MinimalDataObjectHandle*> handles;

	for(auto it : m_inputDataObjects)
		handles.push_back(&m_inputDataObjects[it]);

	for(auto it : m_outputDataObjects)
		handles.push_back(&m_outputDataObjects[it]);

	return handles;

}

void Algorithm::initToolHandles() const{

	MsgStream log ( msgSvc() , name() ) ;

	for(auto th : m_toolHandles){
		IAlgTool * tool = nullptr;

		//if(th->retrieve().isFailure())
			//log << MSG::DEBUG << "Error in retrieving tool from ToolHandle" << endmsg;

		//get generic tool interface from ToolHandle
		if(th->retrieve(tool).isSuccess() && tool != nullptr){
			m_tools.push_back(tool);
			log << MSG::DEBUG << "Adding ToolHandle tool " << tool->name() << " (" << tool->type() << ")" << endmsg;
		} else {
			log << MSG::DEBUG << "Trying to add nullptr tool" << endmsg;
		}
	}

	m_toolHandlesInit = true;
}

const std::vector<IAlgTool *> & Algorithm::tools() const {
	if(UNLIKELY(!m_toolHandlesInit))
		initToolHandles();

	return m_tools;
}

std::vector<IAlgTool *> & Algorithm::tools() {
	if(UNLIKELY(!m_toolHandlesInit))
		initToolHandles();

	return m_tools;
}

void Algorithm::addSubAlgorithmDataObjectHandles(){
	//add all DOHs of SubAlgs to own collection

	MsgStream log ( msgSvc() , name() ) ;

	for(Algorithm * alg : *m_subAlgms){

		assert(alg->isInitialized());

		log << MSG::DEBUG << "Adding subAlg DOHs for " << alg->name() << endmsg;

		for(auto tag : alg->inputDataObjects()){
			auto doh = &alg->inputDataObjects()[tag];

			if(!doh->isValid())
				continue;

			//if the output of an previous algorithm produces the required input don't add it
			if(!m_outputDataObjects.contains(doh)){
				log << MSG::DEBUG << "\tinput handle " << doh->dataProductName() << " is real input to sequence, adding as "
					<< (alg->name() + "/" + doh->descriptor()->tag()) << endmsg;

				m_inputDataObjects.insert(alg->name() + "/" + doh->descriptor()->tag() ,doh);
			} else {
				log << MSG::DEBUG << "\tinput handle " << doh->dataProductName() << " is produced by algorithm in sequence" << endmsg;
			}
		}

		//add output after input to properly treat update DOH
		for(auto tag : alg->outputDataObjects()){
			auto doh = &alg->outputDataObjects()[tag];

			if(!doh->isValid())
				continue;

			log << MSG::DEBUG << "\toutput handle " << doh->dataProductName() << " is output of sequence, adding as "
				<< (alg->name() + "/" + doh->descriptor()->tag()) << endmsg;

			m_outputDataObjects.insert(alg->name() + "/" + doh->descriptor()->tag(), doh);
		}
	}
}

/**
 ** Protected Member Functions
 **/
void
Algorithm::initOutputLevel(Property& /*prop*/)
{
  // do nothing... yet ?
}

StatusCode
Algorithm::service_i(const std::string& svcName,
                     bool createIf,
                     const InterfaceID& iid,
                     void** ppSvc) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return helper.getService(svcName, createIf, iid, ppSvc);
}

StatusCode
Algorithm::service_i(const std::string& svcType,
                     const std::string& svcName,
                     const InterfaceID& iid,
                     void** ppSvc) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return helper.createService(svcType, svcName, iid, ppSvc);
}

SmartIF<IService> Algorithm::service(const std::string& name, const bool createIf, const bool quiet) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return helper.service(name, quiet, createIf);
}

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
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

// Constructor
Algorithm::Algorithm( const std::string& name, ISvcLocator *pSvcLocator,
                      const std::string& version)
  : m_name(name),
    m_version(version),
    m_registerContext ( false ) ,
    m_pSvcLocator(pSvcLocator),
    m_filterPassed(true),
    m_isEnabled(true),
    m_isExecuted(false),
    m_state(Gaudi::StateMachine::CONFIGURED),
    m_targetState(Gaudi::StateMachine::CONFIGURED)
{
  m_propertyMgr = new PropertyMgr();
  m_subAlgms = new std::vector<Algorithm *>();

  // Declare common Algorithm properties with their defaults
  declareProperty( "OutputLevel",        m_outputLevel = MSG::NIL);
  declareProperty( "Enable",             m_isEnabled = true);
  declareProperty( "ErrorMax",           m_errorMax  = 1);
  declareProperty( "ErrorCount",         m_errorCount = 0);
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
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : 0 ) ;

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

  Gaudi::Guards::AuditorGuard guard(this,
                                    // check if we want to audit the initialize
                                    (m_auditorExecute) ? auditorSvc().get() : 0,
                                    IAuditor::Execute,
                                    status);
  try {
    status = execute();
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

    Stat stat( chronoSvc() , Exception.tag() ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch( const std::exception& Exception )
  {
    setExecuted(true);  // set the executed flag

    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch(...)
  {
    setExecuted(true);  // set the executed flag

    MsgStream log ( msgSvc() , name() );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;

    status = exceptionSvc()->handle(*this);
  }

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
serviceAccessor(eventCnvSvc, IConversionSvc, "EventPersistencySvc", m_ECS)
serviceAccessor(histoSvc, IHistogramSvc, "HistogramDataSvc", m_HDS)
serviceAccessor(exceptionSvc, IExceptionSvc, "ExceptionSvc", m_EXS)
serviceAccessor(ntupleSvc, INTupleSvc, "NTupleSvc", m_NTS)
//serviceAccessor(atupleSvc, IAIDATupleSvc, "AIDATupleSvc", m_ATS)
serviceAccessor(randSvc, IRndmGenSvc, "RndmGenSvc", m_RGS)
serviceAccessor(toolSvc, IToolSvc, "ToolSvc", m_ptoolSvc)
serviceAccessor(contextSvc, IAlgContextSvc,"AlgContextSvc", m_contextSvc)


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

#include <algorithm>
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
#include "GaudiKernel/DataHandleHolderVisitor.h"

#include "GaudiKernel/DataObjIDProperty.h"

namespace {
    template <StatusCode (Algorithm::*f)(), typename C > bool for_algorithms(C& c) {
       return std::accumulate( std::begin(c), std::end(c), true,
                               [](bool b, Algorithm* a) { return (a->*f)().isSuccess() && b; } );
    }
}

// Constructor
Algorithm::Algorithm( const std::string& name, ISvcLocator *pSvcLocator,
                      const std::string& version)
  : m_event_context(nullptr),
    m_name(name),
    m_version(version),
    m_index(123), // FIXME: to be fixed in the algorithmManager
    m_pSvcLocator(pSvcLocator),
    m_propertyMgr( new PropertyMgr() )
{

  // Declare common Algorithm properties with their defaults
  declareProperty( "OutputLevel",        m_outputLevel = MSG::NIL);
  declareProperty( "Enable",             m_isEnabled = true);
  declareProperty( "ErrorMax",           m_errorMax  = 1);
  declareProperty( "ErrorCounter",       m_errorCount = 0);

  // FIXME: this should eventually be deprecated
  //declare Extra input and output properties
  declareProperty( "ExtraInputs",  m_extInputDataObjs);
  declareProperty( "ExtraOutputs", m_extOutputDataObjs);

  // Auditor monitoring properties

  // Get the default setting for service auditing from the AppMgr
  declareProperty( "AuditAlgorithms", m_auditInit );

  bool audit(false);
  auto appMgr = serviceLocator()->service<IProperty>("ApplicationMgr");
  if (appMgr) {
    const Property& prop = appMgr->getProperty("AuditAlgorithms");
    Property &pr = const_cast<Property&>(prop);
    if (m_name != "IncidentSvc") setProperty( pr ).ignore();
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
  declareProperty( "Timeline"         , m_doTimeline          = true  ) ;

  declareProperty( "MonitorService"   , m_monitorSvcName      = "MonitorSvc" );

  declareProperty
    ( "RegisterForContextService" ,
      m_registerContext  ,
      "The flag to enforce the registration for Algorithm Context Service") ;

  declareProperty( "IsClonable"       , m_isClonable = false, "Thread-safe enough for cloning?" );
  declareProperty( "Cardinality"      , m_cardinality = 1,    "How many clones to create" );
  declareProperty( "NeededResources"  , m_neededResources = std::vector<std::string>() );

  // update handlers.
  m_outputLevel.declareUpdateHandler([this](Property&) { this->updateMsgStreamOutputLevel(this->m_outputLevel); } );
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

  // TODO: (MCl) where shoud we do this? initialize or start?
  // Reset Error count
  //m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

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
          (m_auditorInitialize) ? auditorSvc().get() : nullptr,
          IAuditor::Initialize);
      // Invoke the initialize() method of the derived class
      sc = initialize();
    }

    if( sc.isSuccess() ) {
      // Now initialize care of any sub-algorithms
      bool fail(false);
      for (auto& it : m_subAlgms ) {
        if (it->sysInitialize().isFailure()) fail = true;
      }
      if( fail ) {
	sc = StatusCode::FAILURE;
	error() << " Error initializing one or several sub-algorithms"
	    << endmsg;
      } else {
	// Update the state.
	m_state = m_targetState;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
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

  //
  //// build list of data dependencies
  //

  if (UNLIKELY(msgLevel(MSG::DEBUG))) {
    debug() << "input handles: " << inputHandles().size() << endmsg;
    debug() << "output handles: " << outputHandles().size() << endmsg;
  }

  if (m_updateDataHandles)
	  acceptDHVisitor(m_updateDataHandles.get());

  // visit all sub-algs and tools, build full set
  DHHVisitor avis(m_inputDataObjs, m_outputDataObjs);
  acceptDHVisitor(&avis);

  if (UNLIKELY(msgLevel(MSG::DEBUG))) {
    debug() << "Data Deps for " << name();
    for (auto h : m_inputDataObjs) {
      debug() << "\n  + INPUT  " << h;
    }
    for (auto h : m_outputDataObjs) {
      debug() << "\n  + OUTPUT " << h;
    }
    debug() << endmsg;
  }

  return sc;
}

void
Algorithm::acceptDHVisitor(IDataHandleVisitor *vis) const {
  vis->visit(this);

  // loop through tools
  for (auto tool : tools()) {
    AlgTool* at = dynamic_cast<AlgTool*>(tool);
    vis->visit(at);
    }

  // loop through sub-algs
  for (auto alg : *subAlgorithms()) {
    vis->visit(alg);
  }

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
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke start() method of the derived class inside a try/catch clause
  try
  {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard
        (this,
         // check if we want to audit the initialize
         (m_auditorStart) ? auditorSvc().get() : nullptr,
         IAuditor::Start);
      // Invoke the start() method of the derived class
      sc = start();
    }
    if( sc.isSuccess() ) {

      // Now start any sub-algorithms
      if( !for_algorithms<&Algorithm::sysStart>( m_subAlgms ) ) {
	sc = StatusCode::FAILURE;
	error() << " Error starting one or several sub-algorithms"
	    << endmsg;
      } else {
	// Update the state.
	m_state = m_targetState;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << "in sysStart(): exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << "in sysStart(): standard std::exception is caught" << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "in sysStart(): UNKNOWN Exception is caught" << endmsg;
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
    error()
        << "sysReinitialize(): cannot reinitialize algorithm not initialized"
        << endmsg;
    return StatusCode::FAILURE;
  }

  // Reset Error count
  // m_errorCount = 0; // done during start

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::SUCCESS);
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorReinitialize) ? auditorSvc().get() : nullptr,
                                        IAuditor::ReInitialize);
      // Invoke the reinitialize() method of the derived class
      sc = reinitialize();
    }
    if( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysReinitialize>( m_subAlgms ) ) {
	sc = StatusCode::FAILURE;
	error()
	    << "sysReinitialize(): Error reinitializing one or several "
	    << "sub-algorithms" << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << "sysReinitialize(): Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << "sysReinitialize(): Standard std::exception is caught" << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "sysReinitialize(): UNKNOWN Exception is caught" << endmsg;
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
    error() << "sysRestart(): cannot restart algorithm not started" << endmsg;
    return StatusCode::FAILURE;
  }

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorRestart) ? auditorSvc().get() : nullptr,
                                        IAuditor::ReStart);
      // Invoke the reinitialize() method of the derived class
      sc = restart();
    }
    if( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      if( !for_algorithms<&Algorithm::sysRestart>( m_subAlgms ) ) {
	sc = StatusCode::FAILURE;
	error() << "sysRestart(): Error restarting one or several sub-algorithms" << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << "sysRestart(): Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << "sysRestart(): Standard std::exception is caught" << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "sysRestart(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysBeginRun() {

  // Bypass the beginRun if the algorithm is disabled.
  if ( !isEnabled( ) ) return StatusCode::SUCCESS;

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke beginRun() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorBeginRun) ? auditorSvc().get() : nullptr,
                                        IAuditor::BeginRun);
      // Invoke the beginRun() method of the derived class
      sc = beginRun();
    }
    if( sc.isSuccess() ) {

      // Now call beginRun for any sub-algorithms
      if( !for_algorithms<&Algorithm::sysBeginRun>( m_subAlgms ) ) {
	sc = StatusCode::FAILURE;
	error() << " Error executing BeginRun for one or several sub-algorithms"
          << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
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

  // Reset Error count
  m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  // Invoke endRun() method of the derived class inside a try/catch clause
  StatusCode sc(StatusCode::FAILURE);
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorEndRun) ? auditorSvc().get() : nullptr,
                                        IAuditor::EndRun);
      // Invoke the endRun() method of the derived class
      sc = endRun();
    }
    if( sc.isSuccess() ) {

      // Now call endRun for any sub-algorithms
      if( !for_algorithms<&Algorithm::sysEndRun>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << " Error calling endRun for one or several sub-algorithms"
            << endmsg;
      }
    }
  }
  catch ( const GaudiException& Exception )
  {
    fatal() << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...)
  {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
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
    if ( msgLevel(MSG::VERBOSE) ) {
      verbose() << ".sysExecute(): is not enabled. Skip execution" << endmsg;
    }
    return StatusCode::SUCCESS;
  }

  StatusCode status;

  // Should performance profile be performed ?
  // invoke execute() method of Algorithm class
  //   and catch all uncaught exceptions

  // lock the context service
  Gaudi::Utils::AlgContext cnt
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  // HiveWhiteBoard stuff here
  if(m_WB.isValid()) m_WB->selectStore(getContext() ? getContext()->slot() : 0).ignore();

  Gaudi::Guards::AuditorGuard guard(this,
                                    // check if we want to audit the initialize
                                    (m_auditorExecute) ? auditorSvc().get() : nullptr,
                                    IAuditor::Execute,
                                    status);

  TimelineEvent timeline;
  timeline.algorithm = this->name();
  //  timeline.thread = getContext() ? getContext()->m_thread_id : 0;
  timeline.thread = pthread_self();
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

    if (Exception.code() == StatusCode::FAILURE) {
      fatal();
    } else {
      error() << " Recoverable";
    }

    msgStream() << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;

    error() << Exception  << endmsg;

    //Stat stat( chronoSvc() , Exception.tag() ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch( const std::exception& Exception )
  {
    setExecuted(true);  // set the executed flag

    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what()  << endmsg;
    //Stat stat( chronoSvc() , "*std::exception*" ) ;
    status = exceptionSvc()->handle(*this,Exception);
  }
  catch(...)
  {
    setExecuted(true);  // set the executed flag

    fatal() << "UNKNOWN Exception is caught " << endmsg;
    //Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;

    status = exceptionSvc()->handle(*this);
  }

  if(UNLIKELY(m_doTimeline))
	  timelineSvc()->registerTimelineEvent(timeline);

  if( status.isFailure() ) {
    // Increment the error count
    m_errorCount++;
    // Check if maximum is exeeded
    if( m_errorCount < m_errorMax ) {
      warning() << "Continuing from error (cnt=" << m_errorCount
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
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke stop() method of the derived class inside a try/catch clause
  try {
    // Stop first any sub-algorithms (in reverse order -- not?)
    for_algorithms<&Algorithm::sysStop>( m_subAlgms );
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorStop) ? auditorSvc().get() : nullptr,
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
    fatal() << "in sysStop(): exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc() , Exception.tag() );
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception ) {
    fatal() << "in sysStop(): standard std::exception is caught" << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" );
    sc = StatusCode::FAILURE;
  }
  catch(...) {
    fatal() << "in sysStop(): UNKNOWN Exception is caught" << endmsg;
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
    ( this , registerContext() ? contextSvc().get() : nullptr ) ;

  StatusCode sc(StatusCode::FAILURE);
  // Invoke finalize() method of the derived class inside a try/catch clause
  try {
    // Order changed (bug #3903 overview: finalize and nested algorithms)
    // Finalize first any sub-algorithms (it can be done more than once)
    bool ok = for_algorithms<&Algorithm::sysFinalize>( m_subAlgms );
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard(this,
                                        // check if we want to audit the initialize
                                        (m_auditorFinalize) ? auditorSvc().get() : nullptr,
                                        IAuditor::Finalize);
      // Invoke the finalize() method of the derived class
      sc = finalize();
    }
    if (!ok) sc = StatusCode::FAILURE;

    if( sc.isSuccess() ) {

      // Release all sub-algorithms
      for (auto& it : m_subAlgms ) it->release();
      // Indicate that this Algorithm has been finalized to prevent duplicate attempts
      m_state = m_targetState;
    }
  }
  catch( const GaudiException& Exception )
  {
    fatal() << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    error() << Exception  << endmsg;
    Stat stat( chronoSvc() , Exception.tag() ) ;
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception& Exception )
  {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what()  << endmsg;
    Stat stat( chronoSvc() , "*std::exception*" ) ;
    sc = StatusCode::FAILURE;
  }
  catch( ... )
  {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
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
    error() << "reinitialize(): cannot be finalized" << endmsg;
    return sc;
  }
  sc = initialize();
  if (sc.isFailure()) {
    error() << "reinitialize(): cannot be initialized" << endmsg;
    return sc;
  }
  */
  return StatusCode::SUCCESS;
}

StatusCode Algorithm::restart() {
  // Default implementation is stop+start
  StatusCode sc = stop();
  if (sc.isFailure()) {
    error() << "restart(): cannot be stopped" << endmsg;
    return sc;
  }
  sc = start();
  if (sc.isFailure()) {
    error() << "restart(): cannot be started" << endmsg;
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

const std::vector<Algorithm*>* Algorithm::subAlgorithms( ) const {
  return &m_subAlgms;
}

std::vector<Algorithm*>* Algorithm::subAlgorithms( ) {
  return &m_subAlgms;
}


#define serviceAccessor(METHOD,INTERFACE,NAME,MEMBER) \
SmartIF<INTERFACE>& Algorithm::METHOD() const { \
  if ( !MEMBER ) { \
    MEMBER = service(NAME); \
    if( !MEMBER ) { \
      throw GaudiException("Service [" NAME  "] not found", name(), StatusCode::FAILURE); \
    } \
  } \
  return MEMBER; \
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
SmartIF<INTupleSvc>& Algorithm::ntupleService() const {
  return ntupleSvc();
}

SmartIF<ISvcLocator>& Algorithm::serviceLocator() const {
  return *const_cast<SmartIF<ISvcLocator>*>(&m_pSvcLocator);
}

// Use the job options service to set declared properties
StatusCode Algorithm::setProperties() {
  if( !m_pSvcLocator ) return StatusCode::FAILURE;
  auto jos = m_pSvcLocator->service<IJobOptionsSvc>("JobOptionsSvc");
  if( !jos )  return StatusCode::FAILURE;
  // set first generic Properties
  StatusCode sc = jos->setMyProperties( getGaudiThreadGenericName(name()), this );
  if( sc.isFailure() ) return StatusCode::FAILURE;

  // set specific Properties
  if (isGaudiThreaded(name())) {
    if(jos->setMyProperties( name(), this ).isFailure()) {
      return StatusCode::FAILURE;
    }
  }
  updateMsgStreamOutputLevel( m_outputLevel );
  return sc;
}

StatusCode Algorithm::createSubAlgorithm(const std::string& type,
                                         const std::string& name,
                                         Algorithm*& pSubAlgorithm) {
  if( !m_pSvcLocator ) return StatusCode::FAILURE;

  SmartIF<IAlgManager> am(m_pSvcLocator);
  if ( !am ) return StatusCode::FAILURE;

  // Maybe modify the AppMgr interface to return Algorithm* ??
  IAlgorithm *tmp;
  StatusCode sc = am->createAlgorithm
    (type, name+getGaudiThreadIDfromName(Algorithm::name()), tmp);
  if( sc.isFailure() ) return StatusCode::FAILURE;

  try{
    pSubAlgorithm = dynamic_cast<Algorithm*>(tmp);
    m_subAlgms.push_back(pSubAlgorithm);
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

void Algorithm::initToolHandles() const{

  IAlgTool* tool(0);
  for (auto thArr : m_toolHandleArrays) {
    if (! thArr->retrieved()) {
      if (UNLIKELY(msgLevel(MSG::DEBUG)))
        debug() << "ToolHandleArray " << thArr->propertyName()
                << " not used: not registering any of its Tools" << endmsg;
    } else {
      if (UNLIKELY(msgLevel(MSG::DEBUG)))
        debug() << "Registering all Tools in ToolHandleArray " 
                << thArr->propertyName() ;
      for (auto th_name : thArr->typesAndNames()) {
        if (UNLIKELY(msgLevel(MSG::DEBUG)))
          debug() << std::endl << "    + " << th_name;
        if (toolSvc()->retrieveTool(th_name, tool, this).isSuccess()) {
          if (UNLIKELY(msgLevel(MSG::DEBUG)))
            debug() << " (private)";
          m_tools.push_back(tool);
        } else if (toolSvc()->retrieveTool(th_name, tool, 0).isSuccess()) {
          if (UNLIKELY(msgLevel(MSG::DEBUG)))
            debug() << " (public)";
          m_tools.push_back(tool);
        } else {
          if (UNLIKELY(msgLevel(MSG::DEBUG)))
            debug() << " - ERROR" << endmsg;
          warning() <<  "Error retrieving Tool " << th_name 
                    << " in ToolHandleArray" << thArr->propertyName()
                    << ". Not registered" << endmsg;
        }
      }
      if (UNLIKELY(msgLevel(MSG::DEBUG))) debug() << endmsg;
    }
  }
    
  for(auto th : m_toolHandles){
    tool = th->get();
    if(tool){
      if (UNLIKELY(msgLevel(MSG::DEBUG)))
        debug() << "Adding "
        << (th->isPublic() ? "public" : "private" )
        << " ToolHandle tool " << tool->name()
        << " (" << tool->type() << ")" << endmsg;
      m_tools.push_back(tool);
    } else {
      if (UNLIKELY(msgLevel(MSG::DEBUG)))
        debug() << "ToolHandle " << th->typeAndName() << " not used" << endmsg;
    }
  }

  m_toolHandlesInit = true;
}

const std::vector<IAlgTool *> & Algorithm::tools() const {
	if(UNLIKELY(!m_toolHandlesInit)) initToolHandles();
	return m_tools;
}

std::vector<IAlgTool *> & Algorithm::tools() {
	if(UNLIKELY(!m_toolHandlesInit)) initToolHandles();
	return m_tools;
}

/**
 ** Protected Member Functions
 **/

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

//-----------------------------------------------------------------------------
void
Algorithm::commitHandles() {
  //-----------------------------------------------------------------------------

  for (auto h : m_outputHandles) {
    h->commit();
  }

  for (auto t : m_tools) {
    AlgTool* at = dynamic_cast<AlgTool*>(t);
    if (at != 0) at->commitHandles();
  }

  for (auto& a : m_subAlgms ) {
    a->commitHandles();
  }

}

void
Algorithm::registerTool(IAlgTool * tool) const {
    if (UNLIKELY(m_outputLevel <= MSG::DEBUG)) {
      MsgStream log(msgSvc(), name());
      log << MSG::DEBUG << "Registering tool " << tool->name() << endmsg;
    }
    m_tools.push_back(tool);
}


void
Algorithm::deregisterTool(IAlgTool * tool) const {
  std::vector<IAlgTool *>::iterator it = std::find(m_tools.begin(),
                                                   m_tools.end(), tool);

  MsgStream log(msgSvc(), name());
  if (it != m_tools.end()) {
    if (UNLIKELY(m_outputLevel <= MSG::DEBUG))
      log << MSG::DEBUG << "De-Registering tool " << tool->name()
        << endmsg;
    m_tools.erase(it);
  } else {
    if (UNLIKELY(m_outputLevel <= MSG::DEBUG))
      log << MSG::DEBUG << "Could not de-register tool " << tool->name()
        << endmsg;
  }
}

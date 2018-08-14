#include "GaudiKernel/Algorithm.h"

#include <algorithm>
#include <numeric>
#include <set>

#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/ToolHandle.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "GaudiKernel/Algorithm.h"
#pragma GCC diagnostic pop

namespace
{
  template <StatusCode ( Algorithm::*f )(), typename C>
  bool for_algorithms( C& c )
  {
    return std::accumulate( std::begin( c ), std::end( c ), true,
                            []( bool b, Algorithm* a ) { return ( a->*f )().isSuccess() && b; } );
  }
}

// Constructor
Algorithm::Algorithm( const std::string& name, ISvcLocator* pSvcLocator, const std::string& version )
    : m_name( name )
    , m_version( version )
    , m_index( 0 )
    , // incremented by AlgResourcePool
    m_pSvcLocator( pSvcLocator )
{
  // Auditor monitoring properties
  // Initialize the default value from ApplicationMgr AuditAlgorithms
  Gaudi::Property<bool> audit( false );
  auto                  appMgr = serviceLocator()->service<IProperty>( "ApplicationMgr" );
  if ( appMgr && appMgr->hasProperty( "AuditAlgorithms" ) ) {
    audit.assign( appMgr->getProperty( "AuditAlgorithms" ) );
  }
  m_auditInit           = audit;
  m_auditorInitialize   = audit;
  m_auditorReinitialize = audit;
  m_auditorRestart      = audit;
  m_auditorExecute      = audit;
  m_auditorFinalize     = audit;
  m_auditorBeginRun     = audit;
  m_auditorEndRun       = audit;
  m_auditorStart        = audit;
  m_auditorStop         = audit;
}

// IAlgorithm implementation
StatusCode Algorithm::sysInitialize()
{

  // Bypass the initialization if the algorithm
  // has already been initialized.
  if ( Gaudi::StateMachine::INITIALIZED <= FSMState() ) return StatusCode::SUCCESS;

  // Set the Algorithm's properties
  if ( !setProperties() ) return StatusCode::FAILURE;

  // Bypass the initialization if the algorithm is disabled.
  // Need to do this after setProperties.
  if ( !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::INITIALIZE, m_state );

  // TODO: (MCl) where shoud we do this? initialize or start?
  // Reset Error count
  // m_errorCount = 0;

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  // Get WhiteBoard interface if implemented by EventDataSvc
  m_WB = service( "EventDataSvc" );

  // check whether timeline should be done
  m_doTimeline = timelineSvc()->isEnabled();

  StatusCode sc;
  // Invoke initialize() method of the derived class inside a try/catch clause
  try {

    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorInitialize ) ? auditorSvc().get() : nullptr, IAuditor::Initialize );
      // Invoke the initialize() method of the derived class
      sc = initialize();
    }

    if ( sc.isSuccess() ) {
      // Now initialize care of any sub-algorithms
      bool fail( false );
      for ( auto& it : m_subAlgms ) {
        if ( it->sysInitialize().isFailure() ) fail = true;
      }
      if ( fail ) {
        sc = StatusCode::FAILURE;
        error() << " Error initializing one or several sub-algorithms" << endmsg;
      } else {
        // Update the state.
        m_state = m_targetState;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }
  if ( !sc ) return sc;

  algExecStateSvc()->addAlg( this );

  //
  //// build list of data dependencies
  //

  // ignore this step if we're a Sequence
  if ( this->isSequence() ) {
    return sc;
  }

  // Perform any scheduled dependency update
  if ( m_updateDependencies ) updateDataDependencies( m_updateDependencies );

  // Collect all explicit dependencies in a single place
  collectExplicitDataDependencies();

  // Print a summary of the Algorithm's inputs and outputs
  using AccessMode = Gaudi::v2::DataHandle::AccessMode;
  if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
    debug() << "input handles: " << dataDependencies( AccessMode::Read ).size() << endmsg;
    debug() << "output handles: " << dataDependencies( AccessMode::Write ).size() << endmsg;
  }

  // Check for explicit circular data dependencies
  sc = handleCircularDataDependencies( [this]( const DataObjID& key ) -> CircularDepAction {
    error() << "Explicit circular data dependency detected for id " << key << endmsg;
    return CircularDepAction::Abort;
  } );
  if ( !sc ) return sc;

  // Initialize ToolHandles if needed
  try {
    if ( !m_toolHandlesInit ) initToolHandles();
  } catch ( const GaudiException& Exception ) {
    error() << "Failing initializing ToolHandles : " << Exception << endmsg;
    return StatusCode::FAILURE;
  }

  // Add tool dependencies to our dependency list
  for ( auto tool : tools() ) collectImplicitDataDependencies( dynamic_cast<AlgTool*>( tool ) );

  // Add sub-Algorithm dependencies to our dependency list
  for ( auto alg : *subAlgorithms() ) collectImplicitDataDependencies( alg );

  // Check for implicit circular data deps from child Algs/AlgTools
  sc = handleCircularDataDependencies( [this]( const DataObjID& key ) -> CircularDepAction {
    if ( m_filterCircDeps ) {
      warning() << "Implicit circular data dependency detected for id " << key << endmsg;
      return CircularDepAction::Ignore;
    } else {
      error() << "Implicit circular data dependency detected for id " << key << endmsg;
      return CircularDepAction::Abort;
    }
  } );
  if ( !sc ) return sc;

  // Display the final data dependencies
  if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
    // sort out DataObjects by path so that logging is reproducable
    // we define a little helper creating an ordered set from a non ordered one
    auto sort     = []( const DataObjID a, const DataObjID b ) -> bool { return a.fullKey() < b.fullKey(); };
    auto orderset = [&sort]( const DataObjIDColl& in ) -> std::set<DataObjID, decltype( sort )> {
      return {in.begin(), in.end(), sort};
    };
    // Logging
    debug() << "Data Deps for " << name();
    for ( auto h : orderset( dataDependencies( AccessMode::Read ) ) ) {
      debug() << "\n  + INPUT  " << h;
    }
    for ( auto id : orderset( ignoredDataDependencies( AccessMode::Read ) ) ) {
      debug() << "\n  + INPUT IGNORED " << id;
    }
    for ( auto h : orderset( dataDependencies( AccessMode::Write ) ) ) {
      debug() << "\n  + OUTPUT " << h;
    }
    for ( auto id : orderset( ignoredDataDependencies( AccessMode::Write ) ) ) {
      debug() << "\n  + OUTPUT IGNORED " << id;
    }
    debug() << endmsg;
  }

  // Initialize the inner DataHandles
  initializeDataHandleHolder();

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysStart()
{

  // Bypass the startup if already running or disabled.
  if ( Gaudi::StateMachine::RUNNING == FSMState() || !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::START, m_state );

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::FAILURE );
  // Invoke start() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorStart ) ? auditorSvc().get() : nullptr, IAuditor::Start );
      // Invoke the start() method of the derived class
      sc = start();
    }
    if ( sc.isSuccess() ) {

      // Now start any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysStart>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << " Error starting one or several sub-algorithms" << endmsg;
      } else {
        // Update the state.
        m_state = m_targetState;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << "in sysStart(): exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << "in sysStart(): standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "in sysStart(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysReinitialize()
{

  // Bypass the initialization if the algorithm is disabled.
  if ( !isEnabled() ) return StatusCode::SUCCESS;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    error() << "sysReinitialize(): cannot reinitialize algorithm not initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  // Reset Error count
  // m_errorCount = 0; // done during start

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::SUCCESS );
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorReinitialize ) ? auditorSvc().get() : nullptr,
                                         IAuditor::ReInitialize );
      // Invoke the reinitialize() method of the derived class
      sc = reinitialize();
    }
    if ( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysReinitialize>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << "sysReinitialize(): Error reinitializing one or several "
                << "sub-algorithms" << endmsg;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << "sysReinitialize(): Exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << "sysReinitialize(): Standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "sysReinitialize(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysRestart()
{

  // Bypass the initialization if the algorithm is disabled.
  if ( !isEnabled() ) return StatusCode::SUCCESS;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    error() << "sysRestart(): cannot restart algorithm not started" << endmsg;
    return StatusCode::FAILURE;
  }

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::FAILURE );
  // Invoke reinitialize() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorRestart ) ? auditorSvc().get() : nullptr, IAuditor::ReStart );
      // Invoke the reinitialize() method of the derived class
      sc = restart();
    }
    if ( sc.isSuccess() ) {

      // Now initialize care of any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysRestart>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << "sysRestart(): Error restarting one or several sub-algorithms" << endmsg;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << "sysRestart(): Exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << "sysRestart(): Standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "sysRestart(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  return sc;
}

// IAlgorithm implementation
StatusCode Algorithm::sysBeginRun()
{

  // Bypass the beginRun if the algorithm is disabled.
  if ( !isEnabled() ) return StatusCode::SUCCESS;

  m_beginRunCalled = true;

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::FAILURE );
  // Invoke beginRun() method of the derived class inside a try/catch clause
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorBeginRun ) ? auditorSvc().get() : nullptr, IAuditor::BeginRun );
// Invoke the beginRun() method of the derived class
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      sc = beginRun();
#pragma GCC diagnostic pop
    }
    if ( sc.isSuccess() ) {

      // Now call beginRun for any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysBeginRun>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << " Error executing BeginRun for one or several sub-algorithms" << endmsg;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  if ( !isSequence() && m_beginRunCalled ) {
    warning() << "Algorithm::BeginRun is deprecated. Use Start instead" << endmsg;
  }

  return sc;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
StatusCode             Algorithm::beginRun()
{
  m_beginRunCalled = false;
  return StatusCode::SUCCESS;
}
#pragma GCC diagnostic pop

// IAlgorithm implementation
StatusCode Algorithm::sysEndRun()
{

  // Bypass the endRun if the algorithm is disabled.
  if ( !isEnabled() ) return StatusCode::SUCCESS;
  m_endRunCalled = true;

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  // Invoke endRun() method of the derived class inside a try/catch clause
  StatusCode sc( StatusCode::FAILURE );
  try {
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorEndRun ) ? auditorSvc().get() : nullptr, IAuditor::EndRun );
// Invoke the endRun() method of the derived class
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      sc = endRun();
#pragma GCC diagnostic pop
    }
    if ( sc.isSuccess() ) {

      // Now call endRun for any sub-algorithms
      if ( !for_algorithms<&Algorithm::sysEndRun>( m_subAlgms ) ) {
        sc = StatusCode::FAILURE;
        error() << " Error calling endRun for one or several sub-algorithms" << endmsg;
      }
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  if ( !isSequence() && m_endRunCalled ) {
    warning() << "Algorithm::EndRun is deprecated. Use Stop instead" << endmsg;
  }

  return sc;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
StatusCode             Algorithm::endRun()
{
  m_endRunCalled = false;
  return StatusCode::SUCCESS;
}
#pragma GCC diagnostic pop

StatusCode Algorithm::sysExecute( const EventContext& ctx )
{
  m_event_context = ctx;

  if ( !isEnabled() ) {
    if ( msgLevel( MSG::VERBOSE ) ) {
      verbose() << ".sysExecute(): is not enabled. Skip execution" << endmsg;
    }
    return StatusCode::SUCCESS;
  }

  AlgExecState& algState = execState( ctx );
  algState.setState( AlgExecState::State::Executing );
  StatusCode status;

  // Should performance profile be performed ?
  // invoke execute() method of Algorithm class
  //   and catch all uncaught exceptions

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  Gaudi::Guards::AuditorGuard guard( this,
                                     // check if we want to audit the initialize
                                     ( m_auditorExecute ) ? auditorSvc().get() : nullptr, IAuditor::Execute, status );

  try {
    ITimelineSvc::TimelineRecorder timelineRecoder;
    if ( UNLIKELY( m_doTimeline ) ) {
      timelineRecoder = timelineSvc()->getRecorder( name(), ctx );
    }

    status = execute();

    if ( status.isFailure() ) {
      status = exceptionSvc()->handleErr( *this, status );
    }

  } catch ( const GaudiException& Exception ) {

    if ( Exception.code() == StatusCode::FAILURE ) {
      fatal();
    } else {
      error() << " Recoverable";
    }

    msgStream() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;

    error() << Exception << endmsg;

    // Stat stat( chronoSvc() , Exception.tag() ) ;
    status = exceptionSvc()->handle( *this, Exception );
  } catch ( const std::exception& Exception ) {

    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    // Stat stat( chronoSvc() , "*std::exception*" ) ;
    status = exceptionSvc()->handle( *this, Exception );
  } catch ( ... ) {

    fatal() << "UNKNOWN Exception is caught " << endmsg;
    // Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;

    status = exceptionSvc()->handle( *this );
  }

  if ( status.isFailure() ) {
    // Increment the error count
    unsigned int nerr = m_aess->incrementErrorCount( this );
    // Check if maximum is exeeded
    if ( nerr < m_errorMax ) {
      warning() << "Continuing from error (cnt=" << nerr << ", max=" << m_errorMax << ")" << endmsg;
      // convert to success
      status = StatusCode::SUCCESS;
    } else {
      error() << "Maximum number of errors (" << m_errorMax << ") reached." << endmsg;
    }
  }

  algState.setState( AlgExecState::State::Done, status );

  return status;
}

// IAlgorithm implementation
StatusCode Algorithm::sysStop()
{

  // Bypass the startup if already running or disabled.
  if ( Gaudi::StateMachine::INITIALIZED == FSMState() || !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::STOP, m_state );

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::FAILURE );
  // Invoke stop() method of the derived class inside a try/catch clause
  try {
    // Stop first any sub-algorithms (in reverse order -- not?)
    for_algorithms<&Algorithm::sysStop>( m_subAlgms );
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorStop ) ? auditorSvc().get() : nullptr, IAuditor::Stop );

      // Invoke the stop() method of the derived class
      sc = stop();
    }
    if ( sc.isSuccess() ) {
      // Update the state.
      m_state = m_targetState;
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << "in sysStop(): exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << "in sysStop(): standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "in sysStop(): UNKNOWN Exception is caught" << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }

  return sc;
}

StatusCode Algorithm::sysFinalize()
{

  // Bypass the finalialization if the algorithm hasn't been initilized.
  if ( Gaudi::StateMachine::CONFIGURED == FSMState() || !isEnabled() ) return StatusCode::SUCCESS;

  m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::FINALIZE, m_state );

  // lock the context service
  Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

  StatusCode sc( StatusCode::FAILURE );
  // Invoke finalize() method of the derived class inside a try/catch clause
  try {
    // Order changed (bug #3903 overview: finalize and nested algorithms)
    // Finalize first any sub-algorithms (it can be done more than once)
    bool ok = for_algorithms<&Algorithm::sysFinalize>( m_subAlgms );
    { // limit the scope of the guard
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorFinalize ) ? auditorSvc().get() : nullptr, IAuditor::Finalize );
      // Invoke the finalize() method of the derived class
      sc = finalize();
    }
    if ( !ok ) sc = StatusCode::FAILURE;

    if ( sc.isSuccess() ) {

      // Release all sub-algorithms
      for ( auto& it : m_subAlgms ) it->release();
      // Indicate that this Algorithm has been finalized to prevent duplicate attempts
      m_state = m_targetState;
    }
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    Stat stat( chronoSvc(), Exception.tag() );
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    Stat stat( chronoSvc(), "*std::exception*" );
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    Stat stat( chronoSvc(), "*UNKNOWN Exception*" );
    sc = StatusCode::FAILURE;
  }
  return sc;
}

StatusCode Algorithm::reinitialize()
{
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

StatusCode Algorithm::restart()
{
  // Default implementation is stop+start
  StatusCode sc = stop();
  if ( sc.isFailure() ) {
    error() << "restart(): cannot be stopped" << endmsg;
    return sc;
  }
  sc = start();
  if ( sc.isFailure() ) {
    error() << "restart(): cannot be started" << endmsg;
    return sc;
  }
  return StatusCode::SUCCESS;
}

const std::string& Algorithm::name() const { return m_name.str(); }

const Gaudi::StringKey& Algorithm::nameKey() const { return m_name; }

const std::string& Algorithm::version() const { return m_version; }

unsigned int Algorithm::index() const { return m_index; }

void Algorithm::setIndex( const unsigned int& idx ) { m_index = idx; }

bool Algorithm::isExecuted() const
{
  return execState( Gaudi::Hive::currentContext() ).state() == AlgExecState::State::Done;
}

void Algorithm::setExecuted( bool state ) const
{
  execState( Gaudi::Hive::currentContext() ).setState( state ? AlgExecState::State::Done : AlgExecState::State::None );
}

void Algorithm::resetExecuted() { execState( Gaudi::Hive::currentContext() ).reset(); }

bool Algorithm::isEnabled() const { return m_isEnabled; }

bool Algorithm::filterPassed() const { return execState( Gaudi::Hive::currentContext() ).filterPassed(); }

void Algorithm::setFilterPassed( bool state ) const
{
  execState( Gaudi::Hive::currentContext() ).setFilterPassed( state );
}

AlgExecState& Algorithm::execState( const EventContext& ctx ) const
{
  return algExecStateSvc()->algExecState( const_cast<IAlgorithm*>( (const IAlgorithm*)this ), ctx );
}

const std::vector<Algorithm*>* Algorithm::subAlgorithms() const { return &m_subAlgms; }

std::vector<Algorithm*>* Algorithm::subAlgorithms() { return &m_subAlgms; }

template <typename IFace>
SmartIF<IFace>& Algorithm::get_svc_( SmartIF<IFace>& p, const char* service_name ) const
{
  if ( UNLIKELY( !p ) ) {
    p = this->service( service_name );
    if ( !p ) {
      throw GaudiException( "Service [" + std::string{service_name} + "] not found", this->name(),
                            StatusCode::FAILURE );
    }
  }
  return p;
}

SmartIF<IAlgExecStateSvc>& Algorithm::algExecStateSvc() const { return get_svc_( m_aess, "AlgExecStateSvc" ); }
SmartIF<IAuditorSvc>&      Algorithm::auditorSvc() const { return get_svc_( m_pAuditorSvc, "AuditorSvc" ); }
SmartIF<IChronoStatSvc>&   Algorithm::chronoSvc() const { return get_svc_( m_CSS, "ChronoStatSvc" ); }
SmartIF<IDataProviderSvc>& Algorithm::detSvc() const { return get_svc_( m_DDS, "DetectorDataSvc" ); }
SmartIF<IConversionSvc>&   Algorithm::detCnvSvc() const { return get_svc_( m_DCS, "DetectorPersistencySvc" ); }
SmartIF<IDataProviderSvc>& Algorithm::eventSvc() const { return get_svc_( m_EDS, "EventDataSvc" ); }
SmartIF<IConversionSvc>&   Algorithm::eventCnvSvc() const { return get_svc_( m_ECS, "EventPersistencySvc" ); }
SmartIF<IHistogramSvc>&    Algorithm::histoSvc() const { return get_svc_( m_HDS, "HistogramDataSvc" ); }
SmartIF<INTupleSvc>&       Algorithm::ntupleSvc() const { return get_svc_( m_NTS, "NTupleSvc" ); }
SmartIF<IRndmGenSvc>&      Algorithm::randSvc() const { return get_svc_( m_RGS, "RndmGenSvc" ); }
SmartIF<IToolSvc>&         Algorithm::toolSvc() const { return get_svc_( m_ptoolSvc, "ToolSvc" ); }
SmartIF<IExceptionSvc>&    Algorithm::exceptionSvc() const { return get_svc_( m_EXS, "ExceptionSvc" ); }
SmartIF<IAlgContextSvc>&   Algorithm::contextSvc() const { return get_svc_( m_contextSvc, "AlgContextSvc" ); }
SmartIF<ITimelineSvc>&     Algorithm::timelineSvc() const { return get_svc_( m_timelineSvc, "TimelineSvc" ); }
SmartIF<IHiveWhiteBoard>&  Algorithm::whiteboard() const { return get_svc_( m_WB, "EventDataSvc" ); }

SmartIF<ISvcLocator>& Algorithm::serviceLocator() const { return *const_cast<SmartIF<ISvcLocator>*>( &m_pSvcLocator ); }

// Use the job options service to set declared properties
StatusCode Algorithm::setProperties()
{
  if ( !m_pSvcLocator ) return StatusCode::FAILURE;
  auto jos = m_pSvcLocator->service<IJobOptionsSvc>( "JobOptionsSvc" );
  if ( !jos ) return StatusCode::FAILURE;

  // this initializes the messaging, in case property update handlers need to print
  // and update the property value bypassing the update handler
  m_outputLevel.value() = setUpMessaging();

  return jos->setMyProperties( name(), this );
}

StatusCode Algorithm::createSubAlgorithm( const std::string& type, const std::string& name, Algorithm*& pSubAlgorithm )
{
  if ( !m_pSvcLocator ) return StatusCode::FAILURE;

  SmartIF<IAlgManager> am( m_pSvcLocator );
  if ( !am ) return StatusCode::FAILURE;

  // Maybe modify the AppMgr interface to return Algorithm* ??
  IAlgorithm* tmp;
  StatusCode  sc = am->createAlgorithm( type, name, tmp );
  if ( sc.isFailure() ) return StatusCode::FAILURE;

  try {
    pSubAlgorithm = dynamic_cast<Algorithm*>( tmp );
    m_subAlgms.push_back( pSubAlgorithm );
  } catch ( ... ) {
    sc = StatusCode::FAILURE;
  }
  return sc;
}

void Algorithm::initToolHandles() const
{

  IAlgTool* tool = nullptr;
  for ( auto thArr : m_toolHandleArrays ) {
    if ( !thArr->retrieved() ) {
      if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) )
        debug() << "ToolHandleArray " << thArr->propertyName() << " not used: not registering any of its Tools"
                << endmsg;
    } else {
      if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) )
        debug() << "Registering all Tools in ToolHandleArray " << thArr->propertyName() << endmsg;
      // Iterate over its tools:
      for ( auto toolHandle : thArr->getBaseArray() ) {
        // Try to cast it into a BaseToolHandle pointer:
        BaseToolHandle* bth = dynamic_cast<BaseToolHandle*>( toolHandle );
        if ( bth ) {
          // If the cast was successful, the code is pretty simple:
          tool = bth->get();
          if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
            debug() << "Adding " << ( bth->isPublic() ? "public" : "private" ) << " ToolHandle tool " << tool->name()
                    << " (" << tool->type() << ") from ToolHandleArray " << thArr->propertyName() << endmsg;
          }
          m_tools.push_back( tool );
        } else {
          // If it wasn't for some strange reason, then fall back on the
          // logic implemented previously:
          if ( toolSvc()->retrieveTool( toolHandle->typeAndName(), tool, this, false ).isSuccess() ) {
            if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
              debug() << "Adding private"
                      << " ToolHandle tool " << tool->name() << " (" << tool->type() << ") from ToolHandleArray "
                      << thArr->propertyName() << endmsg;
            }
            m_tools.push_back( tool );
          } else if ( toolSvc()->retrieveTool( toolHandle->typeAndName(), tool, 0, false ).isSuccess() ) {
            if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
              debug() << "Adding public"
                      << " ToolHandle tool " << tool->name() << " (" << tool->type() << ") from ToolHandleArray "
                      << thArr->propertyName() << endmsg;
            }
            m_tools.push_back( tool );
          } else {
            warning() << "Error retrieving Tool " << toolHandle->typeAndName() << " in ToolHandleArray "
                      << thArr->propertyName() << ". Not registered" << endmsg;
          }
        }
      }
    }
  }

  for ( auto th : m_toolHandles ) {
    if ( !th->isEnabled() ) {
      if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) && !th->typeAndName().empty() )
        debug() << "ToolHandle " << th->typeAndName() << " not used" << endmsg;
      continue;
    }
    if ( !th->get() ) {
      auto sc = th->retrieve();
      if ( UNLIKELY( sc.isFailure() ) ) {
        throw GaudiException( "Failed to retrieve tool " + th->typeAndName(), this->name(), StatusCode::FAILURE );
      }
    }
    tool = th->get();
    if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) )
      debug() << "Adding " << ( th->isPublic() ? "public" : "private" ) << " ToolHandle tool " << tool->name() << " ("
              << tool->type() << ")" << endmsg;
    m_tools.push_back( tool );
  }

  m_toolHandlesInit = true;
}

const std::vector<IAlgTool*>& Algorithm::tools() const
{
  if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();
  return m_tools;
}

std::vector<IAlgTool*>& Algorithm::tools()
{
  if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();
  return m_tools;
}

/**
 ** Protected Member Functions
 **/

StatusCode Algorithm::service_i( const std::string& svcName, bool createIf, const InterfaceID& iid, void** ppSvc ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.getService( svcName, createIf, iid, ppSvc );
}

StatusCode Algorithm::service_i( const std::string& svcType, const std::string& svcName, const InterfaceID& iid,
                                 void** ppSvc ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.createService( svcType, svcName, iid, ppSvc );
}

SmartIF<IService> Algorithm::service( const std::string& name, const bool createIf, const bool quiet ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.service( name, quiet, createIf );
}

void Algorithm::registerTool( IAlgTool* tool ) const
{
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Registering tool " << tool->name() << endmsg;
  }
  m_tools.push_back( tool );
}

void Algorithm::deregisterTool( IAlgTool* tool ) const
{
  std::vector<IAlgTool*>::iterator it = std::find( m_tools.begin(), m_tools.end(), tool );
  if ( it != m_tools.end() ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "De-Registering tool " << tool->name() << endmsg;
    m_tools.erase( it );
  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Could not de-register tool " << tool->name() << endmsg;
  }
}

std::ostream& Algorithm::toControlFlowExpression( std::ostream& os ) const
{
  return os << type() << "('" << name() << "')";
}

unsigned int Algorithm::errorCount() const { return m_aess->algErrorCount( static_cast<const IAlgorithm*>( this ) ); }

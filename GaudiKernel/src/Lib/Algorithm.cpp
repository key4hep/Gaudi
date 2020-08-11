/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>

#include <algorithm>
#include <numeric>
#include <set>

#include "GaudiKernel/FunctionalFilterDecision.h"
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

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/ToolHandle.h"

namespace Gaudi {
  namespace Details {
    bool getDefaultAuditorValue( ISvcLocator* loc ) {
      assert( loc != nullptr );
      Gaudi::Property<bool> audit{false};
      auto                  appMgr = loc->service<IProperty>( "ApplicationMgr" );
      if ( appMgr && appMgr->hasProperty( "AuditAlgorithms" ) ) {
        audit.assign( appMgr->getProperty( "AuditAlgorithms" ) );
      }
      return audit.value();
    }
  } // namespace Details

  // IAlgorithm implementation
  StatusCode Algorithm::sysInitialize() {

    // Bypass the initialization if the algorithm
    // has already been initialized.
    if ( Gaudi::StateMachine::INITIALIZED <= FSMState() ) return StatusCode::SUCCESS;

    // this initializes the messaging, in case property update handlers need to print
    // and update the property value bypassing the update handler
    m_outputLevel.value() = setUpMessaging();

    // Set the Algorithm's properties
    bindPropertiesTo( serviceLocator()->getOptsSvc() );

    // Bypass the initialization if the algorithm is disabled.
    // Need to do this after setProperties.
    if ( !isEnabled() ) return StatusCode::SUCCESS;

    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::INITIALIZE, m_state );

    // TODO: (MCl) where should we do this? initialize or start?
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
                                           ( m_auditorInitialize ) ? auditorSvc().get() : nullptr,
                                           IAuditor::Initialize );
        // Invoke the initialize() method of the derived class
        sc = initialize();
      }

      if ( sc.isSuccess() ) {
        // Update the state.
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

    algExecStateSvc()->addAlg( this );

    //
    //// build list of data dependencies
    //

    // ignore this step if we're a Sequence
    if ( this->isSequence() ) { return sc; }

    if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
      debug() << "input handles: " << inputHandles().size() << endmsg;
      debug() << "output handles: " << outputHandles().size() << endmsg;
    }

    // check for explicit circular data dependencies in declared handles
    DataObjIDColl out;
    for ( auto& h : outputHandles() ) {
      if ( !h->objKey().empty() ) out.emplace( h->fullKey() );
    }
    for ( auto& h : inputHandles() ) {
      if ( !h->objKey().empty() && out.find( h->fullKey() ) != out.end() ) {
        // TODO: this case leads to a segfault as the SC, seemingly, is not propagated up properly
        error() << "Explicit circular data dependency detected for id " << h->fullKey() << endmsg;
        sc = StatusCode::FAILURE;
      }
    }

    if ( !sc ) return sc;

    if ( m_updateDataHandles ) acceptDHVisitor( m_updateDataHandles.get() );

    // visit all sub-algs and tools, build full set. First initialize ToolHandles if needed
    try {
      if ( !m_toolHandlesInit ) initToolHandles();
    } catch ( const GaudiException& Exception ) {
      error() << "Failing initializing ToolHandles : " << Exception << endmsg;
      return StatusCode::FAILURE;
    }
    DHHVisitor avis( m_inputDataObjs, m_outputDataObjs );
    acceptDHVisitor( &avis );

    // check for implicit circular data deps from child Algs/AlgTools
    for ( auto& h : m_outputDataObjs ) {
      auto i = m_inputDataObjs.find( h );
      if ( i != m_inputDataObjs.end() ) {
        if ( m_filterCircDeps ) {
          warning() << "Implicit circular data dependency detected for id " << h << endmsg;
          m_inputDataObjs.erase( i );
        } else {
          error() << "Implicit circular data dependency detected for id " << h << endmsg;
          sc = StatusCode::FAILURE;
        }
      }
    }

    if ( !sc ) return sc;

    if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) ) {
      // sort out DataObjects by path so that logging is reproducable
      // we define a little helper creating an ordered set from a non ordered one
      auto sort     = []( const DataObjID a, const DataObjID b ) -> bool { return a.fullKey() < b.fullKey(); };
      auto orderset = [&sort]( const DataObjIDColl& in ) -> std::set<DataObjID, decltype( sort )> {
        return {in.begin(), in.end(), sort};
      };
      // Logging
      debug() << "Data Deps for " << name();
      for ( auto h : orderset( m_inputDataObjs ) ) { debug() << "\n  + INPUT  " << h; }
      for ( auto id : orderset( avis.ignoredInpKeys() ) ) { debug() << "\n  + INPUT IGNORED " << id; }
      for ( auto h : orderset( m_outputDataObjs ) ) { debug() << "\n  + OUTPUT " << h; }
      for ( auto id : orderset( avis.ignoredOutKeys() ) ) { debug() << "\n  + OUTPUT IGNORED " << id; }
      debug() << endmsg;
    }

    // initialize handles
    initDataHandleHolder();

    return sc;
  }

  void Algorithm::acceptDHVisitor( IDataHandleVisitor* vis ) const {
    vis->visit( this );

    // loop through tools
    for ( auto tool : tools() ) vis->visit( dynamic_cast<AlgTool*>( tool ) );
  }

  // IAlgorithm implementation
  StatusCode Algorithm::sysStart() {

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
        // Update the state.
        m_state = m_targetState;
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
  StatusCode Algorithm::sysReinitialize() {

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
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorReinitialize ) ? auditorSvc().get() : nullptr,
                                         IAuditor::ReInitialize );
      // Invoke the reinitialize() method of the derived class
      sc = reinitialize();
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
  StatusCode Algorithm::sysRestart() {

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
      Gaudi::Guards::AuditorGuard guard( this,
                                         // check if we want to audit the initialize
                                         ( m_auditorRestart ) ? auditorSvc().get() : nullptr, IAuditor::ReStart );
      // Invoke the reinitialize() method of the derived class
      sc = restart();
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

  StatusCode Algorithm::sysExecute( const EventContext& ctx ) {
    if ( !isEnabled() ) {
      if ( msgLevel( MSG::VERBOSE ) ) { verbose() << ".sysExecute(): is not enabled. Skip execution" << endmsg; }
      return StatusCode::SUCCESS;
    }

    AlgExecState& algState = execState( ctx );
    algState.setState( AlgExecState::State::Executing );
    StatusCode status;

    // Should performance profile be performed ?
    // invoke execute() method of Algorithm class
    //   and catch all uncaught exceptions

    // lock the context service
    Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr, ctx );

    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorExecute ) ? auditorSvc().get() : nullptr, IAuditor::Execute, status );

    try {
      ITimelineSvc::TimelineRecorder timelineRecoder;
      if ( UNLIKELY( m_doTimeline ) ) { timelineRecoder = timelineSvc()->getRecorder( name(), ctx ); }

      status = execute( ctx );

      if ( status == Gaudi::Functional::FilterDecision::FAILED ) {
        algState.setFilterPassed( false );
      } else if ( status.isFailure() ) {
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
  StatusCode Algorithm::sysStop() {

    // Bypass the startup if already running or disabled.
    if ( Gaudi::StateMachine::INITIALIZED == FSMState() || !isEnabled() ) return StatusCode::SUCCESS;

    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::STOP, m_state );

    // lock the context service
    Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

    StatusCode sc( StatusCode::FAILURE );
    // Invoke stop() method of the derived class inside a try/catch clause
    try {
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

  StatusCode Algorithm::sysFinalize() {

    // Bypass the finalialization if the algorithm hasn't been initilized.
    if ( Gaudi::StateMachine::CONFIGURED == FSMState() || !isEnabled() ) return StatusCode::SUCCESS;

    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::FINALIZE, m_state );

    // lock the context service
    Gaudi::Utils::AlgContext cnt( this, registerContext() ? contextSvc().get() : nullptr );

    StatusCode sc( StatusCode::FAILURE );
    // Invoke finalize() method of the derived class inside a try/catch clause
    try {
      { // limit the scope of the guard
        Gaudi::Guards::AuditorGuard guard( this,
                                           // check if we want to audit the initialize
                                           ( m_auditorFinalize ) ? auditorSvc().get() : nullptr, IAuditor::Finalize );
        // Invoke the finalize() method of the derived class
        sc = finalize();
      }
      if ( sc.isSuccess() ) {
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

  bool Algorithm::isEnabled() const { return m_isEnabled; }

  AlgExecState& Algorithm::execState( const EventContext& ctx ) const {
    return algExecStateSvc()->algExecState( const_cast<IAlgorithm*>( (const IAlgorithm*)this ), ctx );
  }

  template <typename IFace>
  SmartIF<IFace>& Algorithm::get_svc_( SmartIF<IFace>& p, const char* service_name ) const {
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

  SmartIF<ISvcLocator>& Algorithm::serviceLocator() const {
    return *const_cast<SmartIF<ISvcLocator>*>( &m_pSvcLocator );
  }

  void Algorithm::initToolHandles() const {

    auto init_one = [&]( BaseToolHandle* th ) {
      if ( !th->isEnabled() ) {
        if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) && !th->typeAndName().empty() )
          debug() << "ToolHandle " << th->typeAndName() << " not used" << endmsg;
        return;
      }
      if ( !th->get() ) {
        auto sc = th->retrieve();
        if ( UNLIKELY( sc.isFailure() ) ) {
          throw GaudiException( "Failed to retrieve tool " + th->typeAndName(), this->name(), StatusCode::FAILURE );
        }
      }
      auto tool = th->get();
      if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) )
        debug() << "Adding " << ( th->isPublic() ? "public" : "private" ) << " ToolHandle tool " << tool->name() << " ("
                << tool->type() << ")" << endmsg;
      m_tools.push_back( tool );
    };

    for ( auto thArr : m_toolHandleArrays ) {
      if ( UNLIKELY( msgLevel( MSG::DEBUG ) ) )
        debug() << "Registering all Tools in ToolHandleArray " << thArr->propertyName() << endmsg;
      // Iterate over its tools:
      for ( auto toolHandle : thArr->getBaseArray() ) {
        BaseToolHandle* bth = dynamic_cast<BaseToolHandle*>( toolHandle );
        if ( bth ) {
          init_one( bth );
        } else {
          error() << "Error retrieving ToolHandle " << toolHandle->typeAndName() << " in ToolHandleArray "
                  << thArr->propertyName() << ". Not registered" << endmsg;
        }
      }
    }

    for ( BaseToolHandle* th : m_toolHandles ) init_one( th );

    m_toolHandlesInit = true;
  }

  const std::vector<IAlgTool*>& Algorithm::tools() const {
    if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();
    return m_tools;
  }

  std::vector<IAlgTool*>& Algorithm::tools() {
    if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();
    return m_tools;
  }

  /**
   ** Protected Member Functions
   **/

  StatusCode Algorithm::service_i( std::string_view svcName, bool createIf, const InterfaceID& iid,
                                   void** ppSvc ) const {
    const ServiceLocatorHelper helper( *serviceLocator(), *this );
    return helper.getService( svcName, createIf, iid, ppSvc );
  }

  StatusCode Algorithm::service_i( std::string_view svcType, std::string_view svcName, const InterfaceID& iid,
                                   void** ppSvc ) const {
    const ServiceLocatorHelper helper( *serviceLocator(), *this );
    return helper.createService( svcType, svcName, iid, ppSvc );
  }

  SmartIF<IService> Algorithm::service( std::string_view name, const bool createIf, const bool quiet ) const {
    const ServiceLocatorHelper helper( *serviceLocator(), *this );
    return helper.service( name, quiet, createIf );
  }

  void Algorithm::registerTool( IAlgTool* tool ) const {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "Registering tool " << tool->name() << endmsg; }
    m_tools.push_back( tool );
  }

  void Algorithm::deregisterTool( IAlgTool* tool ) const {
    std::vector<IAlgTool*>::iterator it = std::find( m_tools.begin(), m_tools.end(), tool );
    if ( it != m_tools.end() ) {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "De-Registering tool " << tool->name() << endmsg;
      m_tools.erase( it );
    } else {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "Could not de-register tool " << tool->name() << endmsg;
    }
  }

  std::ostream& Algorithm::toControlFlowExpression( std::ostream& os ) const {
    return os << type() << "('" << name() << "')";
  }

  unsigned int Algorithm::errorCount() const { return m_aess->algErrorCount( static_cast<const IAlgorithm*>( this ) ); }
} // namespace Gaudi

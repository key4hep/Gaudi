// Include files
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ToolHandle.h"

//------------------------------------------------------------------------------
namespace
{
  template <typename FUN>
  StatusCode attempt( AlgTool& tool, const char* label, FUN&& fun )
  {
    try {
      return fun();
    } catch ( const GaudiException& Exception ) {
      MsgStream log( tool.msgSvc(), tool.name() + "." + label );
      log << MSG::FATAL << " Exception with tag=" << Exception.tag() << " is caught" << endmsg;
      log << MSG::ERROR << Exception << endmsg;
    } catch ( const std::exception& Exception ) {
      MsgStream log( tool.msgSvc(), tool.name() + "." + label );
      log << MSG::FATAL << " Standard std::exception is caught" << endmsg;
      log << MSG::ERROR << Exception.what() << endmsg;
    } catch ( ... ) {
      MsgStream log( tool.msgSvc(), tool.name() + "." + label );
      log << MSG::FATAL << "UNKNOWN Exception is caught" << endmsg;
    }
    return StatusCode::FAILURE;
  }
}

//------------------------------------------------------------------------------
StatusCode AlgTool::queryInterface( const InterfaceID& riid, void** ppvi )
//------------------------------------------------------------------------------
{
  if ( !ppvi ) {
    return StatusCode::FAILURE;
  } // RETURN
  StatusCode sc = base_class::queryInterface( riid, ppvi );
  if ( sc.isSuccess() ) return sc;
  auto i = std::find_if( std::begin( m_interfaceList ), std::end( m_interfaceList ),
                         [&]( const std::pair<InterfaceID, void*>& item ) { return item.first.versionMatch( riid ); } );
  if ( i == std::end( m_interfaceList ) ) {
    *ppvi = nullptr;
    return Status::NO_INTERFACE; // RETURN
  }
  *ppvi = i->second;
  addRef();
  return StatusCode::SUCCESS; // RETURN
}
//------------------------------------------------------------------------------
const std::string& AlgTool::name() const
//------------------------------------------------------------------------------
{
  return m_name;
}

//------------------------------------------------------------------------------
const std::string& AlgTool::type() const
//------------------------------------------------------------------------------
{
  return m_type;
}

//------------------------------------------------------------------------------
const IInterface* AlgTool::parent() const
//------------------------------------------------------------------------------
{
  return m_parent;
}

//------------------------------------------------------------------------------
SmartIF<ISvcLocator>& AlgTool::serviceLocator() const
//------------------------------------------------------------------------------
{
  return m_svcLocator;
}

// ============================================================================
// accessor to event service  service
// ============================================================================
IDataProviderSvc* AlgTool::evtSvc() const
{
  if ( !m_evtSvc ) {
    m_evtSvc = service( "EventDataSvc", true );
    if ( !m_evtSvc ) {
      throw GaudiException( "Service [EventDataSvc] not found", name(), StatusCode::FAILURE );
    }
  }
  return m_evtSvc.get();
}
//------------------------------------------------------------------------------
IToolSvc* AlgTool::toolSvc() const
//------------------------------------------------------------------------------
{
  if ( !m_ptoolSvc ) {
    m_ptoolSvc = service( "ToolSvc", true );
    if ( !m_ptoolSvc ) {
      throw GaudiException( "Service [ToolSvc] not found", name(), StatusCode::FAILURE );
    }
  }
  return m_ptoolSvc.get();
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperties()
//------------------------------------------------------------------------------
{
  if ( !m_svcLocator ) return StatusCode::FAILURE;
  auto jos = m_svcLocator->service<IJobOptionsSvc>( "JobOptionsSvc" );
  if ( !jos ) return StatusCode::FAILURE;

  return jos->setMyProperties( name(), this );
}

//------------------------------------------------------------------------------
AlgTool::AlgTool( const std::string& type, const std::string& name, const IInterface* parent )
    //------------------------------------------------------------------------------
    : m_type( type ),
      m_name( name ),
      m_parent( parent )
{
  addRef(); // Initial count set to 1

  IInterface* _p = const_cast<IInterface*>( parent );

  if ( Algorithm* _alg = dynamic_cast<Algorithm*>( _p ) ) {
    m_svcLocator = _alg->serviceLocator();
  } else if ( Service* _svc = dynamic_cast<Service*>( _p ) ) {
    m_svcLocator = _svc->serviceLocator();
  } else if ( AlgTool* _too = dynamic_cast<AlgTool*>( _p ) ) {
    m_svcLocator = _too->serviceLocator();
  } else if ( Auditor* _aud = dynamic_cast<Auditor*>( _p ) ) {
    m_svcLocator = _aud->serviceLocator();
  } else {
    throw GaudiException( "Failure to create tool '" + type + "/" + name + "': illegal parent type '" +
                              System::typeinfoName( typeid( *_p ) ) + "'",
                          "AlgTool", StatusCode::FAILURE );
  }

  // initialize output level from MessageSvc and initialize messaging (before enabling update handler)
  m_outputLevel.value() = setUpMessaging();
  m_outputLevel.declareUpdateHandler(
      [this]( Gaudi::Details::PropertyBase& ) { this->updateMsgStreamOutputLevel( this->m_outputLevel ); } );

  // inherit output level from parent
  { // get the "OutputLevel" property from parent
    SmartIF<IProperty> pprop( _p );
    if ( pprop && pprop->hasProperty( "OutputLevel" ) ) {
      m_outputLevel.assign( pprop->getProperty( "OutputLevel" ) );
    }
  }

  {
    // Auditor monitoring properties
    // Initialize the default value from ApplicationMgr AuditAlgorithms
    Gaudi::Property<bool> audit( false );
    // note that here we need that the service locator is already defined
    auto appMgr = serviceLocator()->service<IProperty>( "ApplicationMgr" );
    if ( appMgr && appMgr->hasProperty( "AuditTools" ) ) {
      audit.assign( appMgr->getProperty( "AuditTools" ) );
    }
    m_auditInit           = audit;
    m_auditorInitialize   = audit;
    m_auditorStart        = audit;
    m_auditorStop         = audit;
    m_auditorFinalize     = audit;
    m_auditorReinitialize = audit;
    m_auditorRestart      = audit;
  }
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysInitialize()
{
  //-----------------------------------------------------------------------------
  return attempt( *this, "sysInitialize", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::INITIALIZE, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorInitialize ? auditorSvc() : nullptr, IAuditor::Initialize );
    StatusCode sc = initialize();
    if ( !sc ) return sc;

    m_state = m_targetState;
    if ( m_updateDataHandles ) acceptDHVisitor( m_updateDataHandles.get() );

    // check for explicit circular data dependencies in declared handles
    DataObjIDColl out;
    for ( auto& h : outputHandles() ) {
      if ( !h->objKey().empty() ) out.emplace( h->fullKey() );
    }
    for ( auto& h : inputHandles() ) {
      if ( !h->objKey().empty() && out.find( h->fullKey() ) != out.end() ) {
        error() << "Explicit circular data dependency found for id " << h->fullKey() << endmsg;
        sc = StatusCode::FAILURE;
      }
    }

    if ( !sc ) return sc;

    // visit all sub-tools, build full set
    DHHVisitor avis( m_inputDataObjs, m_outputDataObjs );
    acceptDHVisitor( &avis );

    // initialize handles
    initDataHandleHolder(); // this should 'freeze' the handle configuration.

    return sc;
  } );
}
//------------------------------------------------------------------------------
StatusCode AlgTool::initialize()
//------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  // Setting the properties is done by the ToolSvc calling setProperties()
  // explicitly.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysStart()
{
  //-----------------------------------------------------------------------------
  return attempt( *this, "sysStart", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::START, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorStart ? auditorSvc() : nullptr, IAuditor::Start );
    StatusCode sc                 = start();
    if ( sc.isSuccess() ) m_state = m_targetState;
    return sc;
  } );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::start()
//------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysStop()
{
  //-----------------------------------------------------------------------------
  return attempt( *this, "sysStop", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::STOP, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorStop ? auditorSvc() : nullptr, IAuditor::Stop );
    StatusCode sc                 = stop();
    if ( sc.isSuccess() ) m_state = m_targetState;
    return sc;
  } );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::stop()
//------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysFinalize()
{
  //-----------------------------------------------------------------------------
  return attempt( *this, "sysFinalize", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::FINALIZE, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorFinalize ? auditorSvc() : nullptr, IAuditor::Finalize );
    StatusCode sc                 = finalize();
    if ( sc.isSuccess() ) m_state = m_targetState;
    return sc;
  } );
}
//------------------------------------------------------------------------------
StatusCode AlgTool::finalize()
//------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysReinitialize()
{
  //-----------------------------------------------------------------------------

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    error() << "sysReinitialize(): cannot reinitialize tool not initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  return attempt( *this, "SysReinitialize()", [&]() {
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorReinitialize ? auditorSvc() : nullptr, IAuditor::ReInitialize );
    return reinitialize();
  } );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::reinitialize()
//------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysRestart()
{
  //-----------------------------------------------------------------------------

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    error() << "sysRestart(): cannot reinitialize tool not started" << endmsg;
    return StatusCode::FAILURE;
  }

  return attempt( *this, "sysRestart", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::START, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       m_auditorRestart ? auditorSvc() : nullptr, IAuditor::ReStart );
    return restart();
  } );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::restart()
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
AlgTool::~AlgTool()
//------------------------------------------------------------------------------
{
  if ( m_pMonitorSvc ) {
    m_pMonitorSvc->undeclareAll( this );
  }
}

void AlgTool::initToolHandles() const
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

const std::vector<IAlgTool*>& AlgTool::tools() const
{
  if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();

  return m_tools;
}

std::vector<IAlgTool*>& AlgTool::tools()
{
  if ( UNLIKELY( !m_toolHandlesInit ) ) initToolHandles();

  return m_tools;
}

//------------------------------------------------------------------------------
/// implementation of service method
StatusCode AlgTool::service_i( const std::string& svcName, bool createIf, const InterfaceID& iid, void** ppSvc ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.getService( svcName, createIf, iid, ppSvc );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::service_i( const std::string& svcType, const std::string& svcName, const InterfaceID& iid,
                               void** ppSvc ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.createService( svcType, svcName, iid, ppSvc );
}

SmartIF<IService> AlgTool::service( const std::string& name, const bool createIf, const bool quiet ) const
{
  const ServiceLocatorHelper helper( *serviceLocator(), *this );
  return helper.service( name, quiet, createIf );
}

//-----------------------------------------------------------------------------
IAuditorSvc* AlgTool::auditorSvc() const
{
  //---------------------------------------------------------------------------
  if ( !m_pAuditorSvc ) {
    m_pAuditorSvc = service( "AuditorSvc", true );
    if ( !m_pAuditorSvc ) {
      throw GaudiException( "Service [AuditorSvc] not found", name(), StatusCode::FAILURE );
    }
  }
  return m_pAuditorSvc.get();
}

//-----------------------------------------------------------------------------
void AlgTool::acceptDHVisitor( IDataHandleVisitor* vis ) const
{
  //-----------------------------------------------------------------------------
  vis->visit( this );

  for ( auto tool : tools() ) vis->visit( dynamic_cast<AlgTool*>( tool ) );
}
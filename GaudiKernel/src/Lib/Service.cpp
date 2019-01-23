// Include Files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/ServiceLocatorHelper.h"

using std::string;

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

Service::~Service()
{
  if ( m_svcManager ) m_svcManager->removeService( this ).ignore();
}

// IService::sysInitialize
StatusCode Service::sysInitialize()
{
  std::call_once( m_initFlag, &Service::sysInitialize_imp, this );
  return m_initSC;
}

void Service::sysInitialize_imp()
{

  try {
    m_targetState = Gaudi::StateMachine::INITIALIZED;
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorInitialize ) ? auditorSvc().get() : nullptr, IAuditor::Initialize );

    m_initSC = setProperties();
    if ( !m_initSC ) return;

    m_initSC = initialize(); // This should change the state to Gaudi::StateMachine::CONFIGURED
    if ( m_initSC.isSuccess() ) m_state = m_targetState;
    return;
  } catch ( const GaudiException& Exception ) {
    fatal() << "in sysInitialize(): exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    //	  Stat stat( chronoSvc() , Exception.tag() );
  } catch ( const std::exception& Exception ) {
    fatal() << "in sysInitialize(): standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    //	  Stat stat( chronoSvc() , "*std::exception*" );
  } catch ( ... ) {
    fatal() << "in sysInitialize(): UNKNOWN Exception is caught" << endmsg;
    //	  Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }

  m_initSC = StatusCode::FAILURE;
}

//--- IService::initialize
StatusCode Service::initialize()
{
  ON_DEBUG debug() << "Service base class initialized successfully" << endmsg;
  m_state = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::CONFIGURE, m_state );
  return StatusCode::SUCCESS;
}

// IService::sysStart
StatusCode Service::sysStart()
{
  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::START, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorStart ) ? auditorSvc().get() : nullptr, IAuditor::Start );
    sc                            = start();
    if ( sc.isSuccess() ) m_state = m_targetState;
    return sc;
  } catch ( const GaudiException& Exception ) {
    fatal() << "in sysStart(): exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    //    Stat stat( chronoSvc() , Exception.tag() );
  } catch ( const std::exception& Exception ) {
    fatal() << "in sysStart(): standard std::exception is caught" << endmsg;
    fatal() << Exception.what() << endmsg;
    //    Stat stat( chronoSvc() , "*std::exception*" );
  } catch ( ... ) {
    fatal() << "in sysStart(): UNKNOWN Exception is caught" << endmsg;
    //    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }

  return StatusCode::FAILURE;
}

// IService::sysStop
StatusCode Service::sysStop()
{
  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::STOP, m_state );
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorStop ) ? auditorSvc().get() : nullptr, IAuditor::Stop );
    sc                            = stop();
    if ( sc.isSuccess() ) m_state = m_targetState;
    return sc;
  } catch ( const GaudiException& Exception ) {
    fatal() << "in sysStop(): exception with tag=" << Exception.tag() << " is caught" << endmsg;
    error() << Exception << endmsg;
    //    Stat stat( chronoSvc() , Exception.tag() );
  } catch ( const std::exception& Exception ) {
    fatal() << "in sysStop(): standard std::exception is caught" << endmsg;
    error() << Exception.what() << endmsg;
    //    Stat stat( chronoSvc() , "*std::exception*" );
  } catch ( ... ) {
    fatal() << "in sysStop(): UNKNOWN Exception is caught" << endmsg;
    //    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }

  return StatusCode::FAILURE;
}

//--- IService::stop
StatusCode Service::stop()
{
  // stub implementation
  return StatusCode::SUCCESS;
}

//--- IService::start
StatusCode Service::start()
{
  // stub implementation
  return StatusCode::SUCCESS;
}

//--- IService::sysFinalize
StatusCode Service::sysFinalize()
{

  StatusCode sc( StatusCode::FAILURE );

  try {
    m_targetState = Gaudi::StateMachine::OFFLINE;
    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorFinalize ) ? auditorSvc().get() : nullptr, IAuditor::Finalize );
    sc                            = finalize();
    if ( sc.isSuccess() ) m_state = m_targetState;
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    //    Stat stat( chronoSvc() , Exception.tag() ) ;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    //    Stat stat( chronoSvc() , "*std::exception*" ) ;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    //    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }

  m_pAuditorSvc = nullptr;
  return sc;
}

//--- IService::finalize
StatusCode Service::finalize()
{
  // m_state = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::TERMINATE,m_state);
  return StatusCode::SUCCESS;
}

//--- IService::sysReinitialize
StatusCode Service::sysReinitialize()
{

  StatusCode sc;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    error() << "sysReinitialize(): cannot reinitialize service not initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  try {

    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorReinitialize ) ? auditorSvc().get() : nullptr,
                                       IAuditor::ReInitialize );
    sc = reinitialize();
    return sc;
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    //    Stat stat( chronoSvc() , Exception.tag() ) ;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    //    Stat stat( chronoSvc() , "*std::exception*" ) ;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    //    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }
  return StatusCode::FAILURE;
}

//--- IService::sysRestart
StatusCode Service::sysRestart()
{

  StatusCode sc;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    error() << "sysRestart(): cannot restart service in state " << FSMState() << " -- must be RUNNING " << endmsg;
    return StatusCode::FAILURE;
  }

  try {

    Gaudi::Guards::AuditorGuard guard( this,
                                       // check if we want to audit the initialize
                                       ( m_auditorRestart ) ? auditorSvc().get() : nullptr, IAuditor::ReStart );
    sc = restart();
    return sc;
  } catch ( const GaudiException& Exception ) {
    fatal() << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
    error() << Exception << endmsg;
    //    Stat stat( chronoSvc() , Exception.tag() ) ;
  } catch ( const std::exception& Exception ) {
    fatal() << " Standard std::exception is caught " << endmsg;
    error() << Exception.what() << endmsg;
    //    Stat stat( chronoSvc() , "*std::exception*" ) ;
  } catch ( ... ) {
    fatal() << "UNKNOWN Exception is caught " << endmsg;
    //    Stat stat( chronoSvc() , "*UNKNOWN Exception*" ) ;
  }
  return StatusCode::FAILURE;
}

//--- IService::reinitialize
StatusCode Service::reinitialize()
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

//--- IService::restart
StatusCode Service::restart()
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

//--- IService::getServiceName
const std::string& Service::name() const { return m_name; }

//--- Retrieve pointer to service locator
SmartIF<ISvcLocator>& Service::serviceLocator() const { return m_svcLocator; }

// Use the job options service to set declared properties
StatusCode Service::setProperties()
{
  const bool CREATEIF( true );
  auto       jos = serviceLocator()->service<IJobOptionsSvc>( "JobOptionsSvc", CREATEIF );
  if ( !jos ) {
    throw GaudiException( "Service [JobOptionsSvc] not found", name(), StatusCode::FAILURE );
  }

  // initialize messaging (except for MessageSvc)
  if ( name() != "MessageSvc" ) {
    // this initializes the messaging, in case property update handlers need to print
    // and update the property value bypassing the update handler
    m_outputLevel.value() = setUpMessaging();
  }

  return jos->setMyProperties( name(), this );
}

//--- Local methods
// Standard Constructor
Service::Service( std::string name, ISvcLocator* svcloc ) : m_name( std::move( name ) ), m_svcLocator( svcloc )
{
  if ( m_name != "MessageSvc" ) { // the MessageSvc should not notify itself
    m_outputLevel.declareUpdateHandler(
        [this]( Gaudi::Details::PropertyBase& ) { this->updateMsgStreamOutputLevel( this->m_outputLevel ); } );
  }

  // Initialize the default value from ApplicationMgr AuditAlgorithms
  Gaudi::Property<bool> audit( false );
  auto                  appMgr = serviceLocator()->service<IProperty>( "ApplicationMgr" );
  if ( appMgr && appMgr->hasProperty( "AuditServices" ) ) {
    audit.assign( appMgr->getProperty( "AuditServices" ) );
  }
  m_auditInit           = audit;
  m_auditorInitialize   = audit;
  m_auditorStart        = audit;
  m_auditorStop         = audit;
  m_auditorFinalize     = audit;
  m_auditorReinitialize = audit;
  m_auditorRestart      = audit;
}

SmartIF<IAuditorSvc>& Service::auditorSvc() const
{
  if ( !m_pAuditorSvc ) {
    m_pAuditorSvc = serviceLocator()->service( "AuditorSvc" );
    if ( !m_pAuditorSvc ) {
      throw GaudiException( "Service [AuditorSvc] not found", name(), StatusCode::FAILURE );
    }
  }
  return m_pAuditorSvc;
}

void Service::setServiceManager( ISvcManager* ism ) { m_svcManager = ism; }

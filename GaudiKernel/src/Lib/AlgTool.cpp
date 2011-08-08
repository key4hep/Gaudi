// Include files
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IJobOptionsSvc.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/Guards.h"

//------------------------------------------------------------------------------
StatusCode AlgTool::queryInterface
( const InterfaceID& riid ,
  void**             ppvi )
  //------------------------------------------------------------------------------
{
  if ( 0 == ppvi ) { return StatusCode::FAILURE ; } // RETURN
  StatusCode sc = base_class::queryInterface(riid,ppvi);
  if (sc.isSuccess()) {
    return sc;
  }
  else {
    for ( InterfaceList::iterator it = m_interfaceList.begin() ;
          m_interfaceList.end() != it ; ++it )
    {
      if ( !it->first.versionMatch ( riid ) ) { continue ; }
      // OK
      *ppvi = it->second ;
      addRef() ;
      return SUCCESS ;     // RETURN
    }
    *ppvi = 0 ;
    return NO_INTERFACE ;  // RETURN
  }
  // cannot reach this point
}
//------------------------------------------------------------------------------
void AlgTool::declInterface( const InterfaceID& iid, void* ii)
  //------------------------------------------------------------------------------
{
  m_interfaceList.push_back(std::make_pair(iid, ii));
}


//------------------------------------------------------------------------------
const std::string& AlgTool::name()   const
  //------------------------------------------------------------------------------
{
  return m_name;
}

//------------------------------------------------------------------------------
const std::string& AlgTool::type()  const
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
ISvcLocator* AlgTool::serviceLocator()  const
  //------------------------------------------------------------------------------
{
  return m_svcLocator;
}

//------------------------------------------------------------------------------
IMessageSvc* AlgTool::msgSvc()  const
  //------------------------------------------------------------------------------
{
  return m_messageSvc;
}

//------------------------------------------------------------------------------
IToolSvc* AlgTool::toolSvc() const
  //------------------------------------------------------------------------------
{
  if ( 0 == m_ptoolSvc ) {
    StatusCode sc = service( "ToolSvc", m_ptoolSvc, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [ToolSvc] not found", name(), sc);
    }
  }
  return m_ptoolSvc;
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperty(const Property& p)
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->setProperty(p);
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperty(const std::string& s)
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->setProperty(s);
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperty(const std::string& n, const std::string& v)
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->setProperty(n,v);
}

//------------------------------------------------------------------------------
StatusCode AlgTool::getProperty(Property* p) const
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->getProperty(p);
}

//------------------------------------------------------------------------------
const Property& AlgTool::getProperty(const std::string& n) const
{
  return m_propertyMgr->getProperty(n);
}

//------------------------------------------------------------------------------
StatusCode AlgTool::getProperty(const std::string& n, std::string& v ) const
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->getProperty(n,v);
}

//------------------------------------------------------------------------------
const std::vector<Property*>& AlgTool::getProperties() const
  //------------------------------------------------------------------------------
{
  return m_propertyMgr->getProperties();
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperties()
  //------------------------------------------------------------------------------
{
  if( m_svcLocator == 0) {
    return StatusCode::FAILURE;
  }
  SmartIF<IJobOptionsSvc> jos(m_svcLocator->service("JobOptionsSvc"));
  if( !jos.isValid() )  return StatusCode::FAILURE;

  // set first generic Properties
  StatusCode sc = jos->setMyProperties( getGaudiThreadGenericName(name()), this );
  if( sc.isFailure() ) return StatusCode::FAILURE;

  // set specific Properties
  if (isGaudiThreaded(name())) {
    if(jos->setMyProperties( name(), this ).isFailure()) {
      return StatusCode::FAILURE;
    }
  }

  // Change my own outputlevel
  if ( 0 != m_messageSvc )
  {
    if ( MSG::NIL != m_outputLevel )
    { m_messageSvc -> setOutputLevel ( name () , m_outputLevel ) ; }
    m_outputLevel = m_messageSvc -> outputLevel ( name () ) ;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
AlgTool::AlgTool( const std::string& type,
                  const std::string& name,
                  const IInterface* parent)
  //------------------------------------------------------------------------------
  : m_outputLevel ( MSG::NIL )
  , m_type          ( type )
  , m_name          ( name )
  , m_parent        ( parent )
  , m_svcLocator    ( 0 )
  , m_messageSvc    ( 0 )
  , m_ptoolSvc      ( 0 )
  , m_pMonitorSvc   ( NULL )
  , m_propertyMgr   ( new PropertyMgr() )
  , m_interfaceList (       )
  , m_threadID      (       )
  , m_pAuditorSvc   ( 0     )
  , m_auditInit     ( false )
  , m_state         ( Gaudi::StateMachine::CONFIGURED )
  , m_targetState   ( Gaudi::StateMachine::CONFIGURED )
{
  addRef(); // Initial count set to 1

  declareProperty( "MonitorService", m_monitorSvcName = "MonitorSvc" );

  { // get the "OutputLevel" property from parent
    const Property* _p = Gaudi::Utils::getProperty ( parent , "OutputLevel") ;
    if ( 0 != _p ) { m_outputLevel.assign( *_p ) ; }
    declareProperty ( "OutputLevel"     , m_outputLevel ) ;
     m_outputLevel.declareUpdateHandler(&AlgTool::initOutputLevel, this);
  }

  IInterface* _p = const_cast<IInterface*> ( parent ) ;

  if      ( Algorithm* _alg = dynamic_cast<Algorithm*> ( _p ) )
  {
    m_svcLocator  = _alg -> serviceLocator  () ;
    m_messageSvc  = _alg -> msgSvc          () ;
    m_threadID    = getGaudiThreadIDfromName ( _alg -> name() ) ;
  }
  else if ( Service*   _svc = dynamic_cast<Service*>  ( _p ) )
  {
    m_svcLocator  = _svc -> serviceLocator () ;
    m_messageSvc  = _svc -> msgSvc         () ;
    m_threadID    = getGaudiThreadIDfromName ( _svc -> name() ) ;
  }
  else if ( AlgTool*   _too = dynamic_cast<AlgTool*>  ( _p ) )
  {
    m_svcLocator  = _too -> m_svcLocator;
    m_messageSvc  = _too -> m_messageSvc;
    m_threadID    = getGaudiThreadIDfromName ( _too ->m_threadID ) ;
  }
  else if ( Auditor*   _aud = dynamic_cast<Auditor*>  ( _p ) )
  {
    m_svcLocator  = _aud -> serviceLocator() ;
    m_messageSvc  = _aud -> msgSvc()         ;
    m_threadID    = getGaudiThreadIDfromName ( _aud -> name() )    ;
  }
  else
  {
    throw GaudiException
      ( "Failure to create tool '"
        + type + "/" + name + "': illegal parent type '"
        + System::typeinfoName(typeid(*_p)) + "'", "AlgTool", 0 );
  }


  { // audit tools
    SmartIF<IProperty> appMgr(m_svcLocator->service("ApplicationMgr"));
    if ( !appMgr.isValid() ) {
      throw GaudiException("Could not locate ApplicationMgr","AlgTool",0);
    }
    const Property* p = Gaudi::Utils::getProperty( appMgr , "AuditTools");
    if ( 0 != p ) { m_auditInit.assign ( *p ) ; }
    declareProperty ( "AuditTools", m_auditInit );
    bool audit = m_auditInit.value();
    // Declare common AlgTool properties with their defaults
    declareProperty ( "AuditInitialize" , m_auditorInitialize = audit ) ;
    declareProperty ( "AuditStart"      , m_auditorStart      = audit ) ;
    declareProperty ( "AuditStop"       , m_auditorStop       = audit ) ;
    declareProperty ( "AuditFinalize"   , m_auditorFinalize   = audit ) ;
  }

  // check thread ID and try if tool name indicates thread ID
  if ( m_threadID.empty() )
  { m_threadID = getGaudiThreadIDfromName ( AlgTool::name() ) ; }
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysInitialize() {
  //-----------------------------------------------------------------------------
  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::INITIALIZE,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorInitialize) ? auditorSvc() : 0,
                                      IAuditor::Initialize);
    sc = initialize();
    if (sc.isSuccess())
      m_state = m_targetState;
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysInitialize()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysInitialize()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysInitialize()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
  }
  return StatusCode::FAILURE ;

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
StatusCode AlgTool::sysStart() {
  //-----------------------------------------------------------------------------
  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::START,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorStart) ? auditorSvc() : 0,
                                      IAuditor::Start);
    sc = start();
    if (sc.isSuccess())
      m_state = m_targetState;
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysStart()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysStart()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysStart()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
  }
  return StatusCode::FAILURE ;

}

//------------------------------------------------------------------------------
StatusCode AlgTool::start()
  //------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysStop() {
  //-----------------------------------------------------------------------------
  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::STOP,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorStop) ? auditorSvc() : 0,
                                      IAuditor::Stop);
    sc = stop();
    if (sc.isSuccess())
      m_state = m_targetState;
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysStop()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysStop()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysStop()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
  }
  return StatusCode::FAILURE ;

}

//------------------------------------------------------------------------------
StatusCode AlgTool::stop()
  //------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysFinalize() {
  //-----------------------------------------------------------------------------

  StatusCode sc;

  try {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::FINALIZE,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorFinalize) ? auditorSvc() : 0,
                                      IAuditor::Finalize);
    sc = finalize();
    if (sc.isSuccess())
      m_state = m_targetState;
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysFinalize()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught " << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysFinalize()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysFinalize()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught " << endmsg;
  }
  return StatusCode::FAILURE;

}
//------------------------------------------------------------------------------
StatusCode  AlgTool::finalize()
  //------------------------------------------------------------------------------
{
  // For the time being there is nothing to be done here.
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysReinitialize() {
  //-----------------------------------------------------------------------------
  StatusCode sc;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR
        << "sysReinitialize(): cannot reinitialize tool not initialized"
        << endmsg;
    return StatusCode::FAILURE;
  }

  try {
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorReinitialize) ? auditorSvc() : 0,
                                      IAuditor::ReInitialize);
    sc = reinitialize();
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysReinitialize()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysReinitialize()" );
    log << MSG::FATAL << " Standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysReinitialize()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught" << endmsg;
  }
  return StatusCode::FAILURE ;

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

//-----------------------------------------------------------------------------
StatusCode AlgTool::sysRestart() {
  //-----------------------------------------------------------------------------
  StatusCode sc;

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::ERROR
        << "sysRestart(): cannot reinitialize tool not started"
        << endmsg;
    return StatusCode::FAILURE;
  }

  try {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::START,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      (m_auditorRestart) ? auditorSvc() : 0,
                                      IAuditor::ReStart);
    sc = restart();
    return sc;
  }
  catch( const GaudiException& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysRestart()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( msgSvc() , name() + ".sysRestart()" );
    log << MSG::FATAL << " Standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( msgSvc() , name() + ".sysRestart()" );
    log << MSG::FATAL << "UNKNOWN Exception is caught" << endmsg;
  }
  return StatusCode::FAILURE ;

}

//------------------------------------------------------------------------------
StatusCode AlgTool::restart()
  //------------------------------------------------------------------------------
{
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

//------------------------------------------------------------------------------
AlgTool::~AlgTool()
  //------------------------------------------------------------------------------
{
  delete m_propertyMgr;
  if( m_ptoolSvc ) m_ptoolSvc->release();
  if( m_pAuditorSvc ) m_pAuditorSvc->release();
  if ( m_pMonitorSvc ) { m_pMonitorSvc->undeclareAll(this); m_pMonitorSvc->release(); }
}

//------------------------------------------------------------------------------
/// implementation of service method
StatusCode
AlgTool::service_i(const std::string& svcName,
                   bool createIf,
                   const InterfaceID& iid,
                   void** ppSvc) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return helper.getService(svcName, createIf, iid, ppSvc);
}

//------------------------------------------------------------------------------
StatusCode
AlgTool::service_i(const std::string& svcType,
                   const std::string& svcName,
                   const InterfaceID& iid,
                   void** ppSvc) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return  helper.createService(svcType, svcName, iid, ppSvc);
}

SmartIF<IService> AlgTool::service(const std::string& name, const bool createIf, const bool quiet) const {
  const ServiceLocatorHelper helper(*serviceLocator(), *this);
  return helper.service(name, quiet, createIf);
}

//-----------------------------------------------------------------------------
IAuditorSvc* AlgTool::auditorSvc() const {
//---------------------------------------------------------------------------
  if ( 0 == m_pAuditorSvc ) {
    StatusCode sc = service( "AuditorSvc", m_pAuditorSvc, true );
    if( sc.isFailure() ) {
      throw GaudiException("Service [AuditorSvc] not found", name(), sc);
    }
  }
  return m_pAuditorSvc;
}


//-----------------------------------------------------------------------------
void  AlgTool::initOutputLevel(Property& /*prop*/) {
//-----------------------------------------------------------------------------
   // do nothing... yet ?
}


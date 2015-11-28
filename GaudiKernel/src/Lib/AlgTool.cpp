// Include files
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/ToolHandle.h"

//------------------------------------------------------------------------------
namespace {
template <typename FUN>
StatusCode attempt( AlgTool& tool, const char* label, FUN&& fun ) {
  try { return fun(); }
  catch( const GaudiException& Exception ) {
    MsgStream log ( tool.msgSvc(), tool.name() + "." + label );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag()
        << " is caught" << endmsg;
    log << MSG::ERROR << Exception  << endmsg;
  }
  catch( const std::exception& Exception ) {
    MsgStream log ( tool.msgSvc(), tool.name() + "." + label );
    log << MSG::FATAL << " Standard std::exception is caught" << endmsg;
    log << MSG::ERROR << Exception.what()  << endmsg;
  }
  catch( ... ) {
    MsgStream log ( tool.msgSvc(), tool.name() + "." + label );
    log << MSG::FATAL << "UNKNOWN Exception is caught" << endmsg;
  }
  return StatusCode::FAILURE ;
}
}

//------------------------------------------------------------------------------
StatusCode AlgTool::queryInterface
( const InterfaceID& riid,
  void**             ppvi )
//------------------------------------------------------------------------------
{
  if ( !ppvi ) { return StatusCode::FAILURE ; } // RETURN
  StatusCode sc = base_class::queryInterface(riid,ppvi);
  if (sc.isSuccess()) return sc;
  auto i = std::find_if( std::begin(m_interfaceList), std::end(m_interfaceList),
                         [&](const std::pair<InterfaceID,void*>& item) {
                             return item.first.versionMatch(riid);
  } );
  if ( i == std::end(m_interfaceList) ) {
    *ppvi = nullptr ;
    return NO_INTERFACE ;  // RETURN
  }
  *ppvi = i->second ;
  addRef() ;
  return SUCCESS ;     // RETURN
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
SmartIF<ISvcLocator>& AlgTool::serviceLocator()  const
//------------------------------------------------------------------------------
{
  return m_svcLocator;
}

// ============================================================================
// accessor to event service  service
// ============================================================================
IDataProviderSvc* AlgTool::evtSvc    () const
{
  if ( !m_evtSvc ) {
    m_evtSvc = service("EventDataSvc", true);
    if ( !m_evtSvc ) {
      throw GaudiException("Service [EventDataSvc] not found", name(), StatusCode::FAILURE);
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
    if( !m_ptoolSvc ) {
      throw GaudiException("Service [ToolSvc] not found", name(), StatusCode::FAILURE);
    }
  }
  return m_ptoolSvc.get();
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

bool AlgTool::hasProperty(const std::string& name) const {
  return m_propertyMgr->hasProperty(name);
}

//------------------------------------------------------------------------------
StatusCode AlgTool::setProperties()
//------------------------------------------------------------------------------
{
  if( !m_svcLocator ) return StatusCode::FAILURE;
  auto jos = m_svcLocator->service<IJobOptionsSvc>("JobOptionsSvc");
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
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
AlgTool::AlgTool( const std::string& type,
                  const std::string& name,
                  const IInterface* parent)
//------------------------------------------------------------------------------
  : m_type          ( type )
  , m_name          ( name )
  , m_parent        ( parent )
  , m_propertyMgr   ( new PropertyMgr() )
{
  addRef(); // Initial count set to 1

  declareProperty( "MonitorService", m_monitorSvcName = "MonitorSvc" );

  { // get the "OutputLevel" property from parent
    const Property* _p = Gaudi::Utils::getProperty ( parent , "OutputLevel") ;
    if ( _p ) { m_outputLevel.assign( *_p ) ; }
    declareProperty ( "OutputLevel"     , m_outputLevel ) ;
     m_outputLevel.declareUpdateHandler([this](Property&) { this->updateMsgStreamOutputLevel(this->m_outputLevel) ; } );
  }

  IInterface* _p = const_cast<IInterface*> ( parent ) ;

  if      ( Algorithm* _alg = dynamic_cast<Algorithm*> ( _p ) )
  {
    m_svcLocator  = _alg -> serviceLocator  () ;
    m_threadID    = getGaudiThreadIDfromName ( _alg -> name() ) ;
  }
  else if ( Service*   _svc = dynamic_cast<Service*>  ( _p ) )
  {
    m_svcLocator  = _svc -> serviceLocator () ;
    m_threadID    = getGaudiThreadIDfromName ( _svc -> name() ) ;
  }
  else if ( AlgTool*   _too = dynamic_cast<AlgTool*>  ( _p ) )
  {
    m_svcLocator  = _too -> serviceLocator ();
    m_threadID    = getGaudiThreadIDfromName ( _too ->m_threadID ) ;
  }
  else if ( Auditor*   _aud = dynamic_cast<Auditor*>  ( _p ) )
  {
    m_svcLocator  = _aud -> serviceLocator() ;
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
    auto appMgr = m_svcLocator->service<IProperty>("ApplicationMgr");
    if ( !appMgr ) {
      throw GaudiException("Could not locate ApplicationMgr","AlgTool",0);
    }
    const Property* p = Gaudi::Utils::getProperty( appMgr , "AuditTools");
    if ( p ) { m_auditInit.assign ( *p ) ; }
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
  return attempt( *this, "sysInitialize", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::INITIALIZE, m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorInitialize ? auditorSvc() : nullptr,
                                      IAuditor::Initialize);
    StatusCode sc = initialize();
    if (sc.isSuccess()) m_state = m_targetState;

    // update DataHandles to point to full TES location
    // init data handle
    for (auto tag : m_inputDataObjects) {
      if (m_inputDataObjects[tag].isValid()) {
        if (m_inputDataObjects[tag].initialize().isSuccess())
          debug() << "Data Handle " << tag << " ("
          << m_inputDataObjects[tag].dataProductName()
          << ") initialized" << endmsg;
        else
          fatal() << "Data Handle " << tag << " ("
          << m_inputDataObjects[tag].dataProductName()
          << ") could NOT be initialized" << endmsg;
      }
    }
    for (auto tag : m_outputDataObjects) {
      if (m_outputDataObjects[tag].isValid()) {
        if (m_outputDataObjects[tag].initialize().isSuccess())
          debug() << "Data Handle " << tag << " ("
          << m_outputDataObjects[tag].dataProductName()
          << ") initialized" << endmsg;
        else
          fatal() << "Data Handle " << tag << " ("
          << m_outputDataObjects[tag].dataProductName()
          << ") could NOT be initialized" << endmsg;
      }
    }

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
StatusCode AlgTool::sysStart() {
//-----------------------------------------------------------------------------
  return attempt( *this, "sysInitialize", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::START, m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorStart ? auditorSvc() : nullptr,
                                      IAuditor::Start);
    StatusCode sc = start();
    if (sc.isSuccess()) m_state = m_targetState;
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
StatusCode AlgTool::sysStop() {
//-----------------------------------------------------------------------------
  return attempt( *this, "sysStop", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::STOP, m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorStop ? auditorSvc() : nullptr,
                                      IAuditor::Stop);
    StatusCode sc = stop();
    if (sc.isSuccess()) m_state = m_targetState;
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
StatusCode AlgTool::sysFinalize() {
//-----------------------------------------------------------------------------
  return attempt( *this, "sysFinalize", [&]() {
    m_targetState = Gaudi::StateMachine::ChangeState( Gaudi::StateMachine::FINALIZE, m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorFinalize ? auditorSvc() : nullptr,
                                      IAuditor::Finalize);
    StatusCode sc = finalize();
    if (sc.isSuccess()) m_state = m_targetState;
    return sc;
  } );
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

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::INITIALIZED != FSMState() ) {
    error()
        << "sysReinitialize(): cannot reinitialize tool not initialized"
        << endmsg;
    return StatusCode::FAILURE;
  }

  return attempt(*this, "SysReinitialize()", [&]() {
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorReinitialize ? auditorSvc() : nullptr,
                                      IAuditor::ReInitialize);
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
StatusCode AlgTool::sysRestart() {
//-----------------------------------------------------------------------------

  // Check that the current status is the correct one.
  if ( Gaudi::StateMachine::RUNNING != FSMState() ) {
    error()
        << "sysRestart(): cannot reinitialize tool not started"
        << endmsg;
    return StatusCode::FAILURE;
  }

  return attempt(*this, "sysRestart", [&]()  {
    m_targetState = Gaudi::StateMachine::ChangeState(Gaudi::StateMachine::START,m_state);
    Gaudi::Guards::AuditorGuard guard(this,
                                      // check if we want to audit the initialize
                                      m_auditorRestart ? auditorSvc() : nullptr,
                                      IAuditor::ReStart);
    return restart();
  } );
}

//------------------------------------------------------------------------------
StatusCode AlgTool::restart()
//------------------------------------------------------------------------------
{
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

//------------------------------------------------------------------------------
AlgTool::~AlgTool()
//------------------------------------------------------------------------------
{
  if( m_pMonitorSvc ) { m_pMonitorSvc->undeclareAll(this); }
}


void AlgTool::initToolHandles() const{


	for(auto th : m_toolHandles){
		IAlgTool * tool = nullptr;

		//if(th->retrieve().isFailure())
			//debug() << "Error in retrieving tool from ToolHandle" << endmsg;

		//get generic tool interface from ToolHandle
		if(th->retrieve(tool).isSuccess() && tool != nullptr){
			m_tools.push_back(tool);
			debug() << "Adding ToolHandle tool " << tool->name() << " (" << tool->type() << ")" << endmsg;
		} else {
			debug() << "Trying to add nullptr tool" << endmsg;
		}
	}

	m_toolHandlesInit = true;
}

const std::vector<IAlgTool *> & AlgTool::tools() const {
	if(UNLIKELY(!m_toolHandlesInit))
		initToolHandles();

	return m_tools;
}

std::vector<IAlgTool *> & AlgTool::tools() {
	if(UNLIKELY(!m_toolHandlesInit))
		initToolHandles();

	return m_tools;
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
  if ( !m_pAuditorSvc ) {
    m_pAuditorSvc = service( "AuditorSvc", true );
    if( !m_pAuditorSvc ) {
      throw GaudiException("Service [AuditorSvc] not found", name(), StatusCode::FAILURE);
    }
  }
  return m_pAuditorSvc.get();
}

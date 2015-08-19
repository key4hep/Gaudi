#define  GAUDIKERNEL_MINIMALEVENTLOOPMGR_CPP

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/AppReturnCode.h"

#include "GaudiKernel/MinimalEventLoopMgr.h"

#include <algorithm>

namespace {
  class AbortEventListener: public implements1<IIncidentListener> {
  public:
    AbortEventListener(bool &flag, std::string &source):m_flag(flag),
                                                        m_source(source){
      addRef(); // Initial count set to 1
    }
    ~AbortEventListener() override = default;
    /// Inform that a new incident has occurred
    void handle(const Incident& i) override {
      if (i.type() == IncidentType::AbortEvent) {
        m_flag = true;
        m_source = i.source();
      }
    }

  private:
    /// flag to set
    bool &m_flag;
    /// string where to store the source of the incident
    std::string &m_source;
  };
}

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

//--------------------------------------------------------------------------------------------
// Standard Constructor
//--------------------------------------------------------------------------------------------
MinimalEventLoopMgr::MinimalEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc)
  : base_class(nam, svcLoc), m_appMgrUI(svcLoc)
{
  declareProperty("TopAlg",         m_topAlgNames );
  declareProperty("OutStream",      m_outStreamNames );
  declareProperty("OutStreamType",  m_outStreamType = "OutputStream");
  m_topAlgNames.declareUpdateHandler   ( &MinimalEventLoopMgr::topAlgHandler, this );
  m_outStreamNames.declareUpdateHandler( &MinimalEventLoopMgr::outStreamHandler, this );
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::initialize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::initialize()    {

  if ( !m_appMgrUI.isValid() ) return StatusCode::FAILURE;

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() )   {
    error() << "Failed to initialize Service Base class." << endmsg;
    return StatusCode::FAILURE;
  }

  SmartIF<IProperty> prpMgr(serviceLocator());
  if ( ! prpMgr.isValid() )   {
    error() << "Error retrieving AppMgr interface IProperty." << endmsg;
    return StatusCode::FAILURE;
  }
  if ( m_topAlgNames.value().empty() )    {
    setProperty(prpMgr->getProperty("TopAlg")).ignore();
  }
  if ( m_outStreamNames.value().empty() )   {
    setProperty(prpMgr->getProperty("OutStream")).ignore();
  }

  // Get the references to the services that are needed by the ApplicationMgr itself
  m_incidentSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incidentSvc.isValid() )  {
    fatal() << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  m_abortEventListener = new AbortEventListener(m_abortEvent,m_abortEventSource);
  m_incidentSvc->addListener(m_abortEventListener,IncidentType::AbortEvent);

  // The state is changed at this moment to allow decodeXXXX() to do something
  m_state = INITIALIZED;

  //--------------------------------------------------------------------------------------------
  // Create output streams. Do not initialize them yet.
  // The state is updated temporarily in order to enable the handler, which
  // is also triggered by a change to the "OutputStream" Property.
  //--------------------------------------------------------------------------------------------
  sc = decodeOutStreams();
  if ( !sc.isSuccess() )    {
    error() << "Failed to initialize Output streams." << endmsg;
    m_state = CONFIGURED;
    return sc;
  }
  //--------------------------------------------------------------------------------------------
  // Create all needed Top Algorithms. Do not initialize them yet.
  // The state is updated temporarily in order to enable the handler, which
  // is also triggered by a change to the "TopAlg" Property.
  //--------------------------------------------------------------------------------------------
  sc = decodeTopAlgs();
  if ( !sc.isSuccess() )    {
    error() << "Failed to initialize Top Algorithms streams." << endmsg;
    m_state = CONFIGURED;
    return sc;
  }

  // Initialize all the new TopAlgs. In fact Algorithms are protected against getting
  // initialized twice.
  for (auto&  ita : m_topAlgList ) {
    sc = ita->sysInitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to initialize Algorithm: " << ita->name() << endmsg;
      return sc;
    }
  }
  for (auto& ita : m_outStreamList ) {
    sc = ita->sysInitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to initialize Output Stream: " << ita->name() << endmsg;
      return sc;
    }
  }

  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::start
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::start()    {

  StatusCode sc = Service::start();
  if ( ! sc.isSuccess() ) return sc;

  // Start all the new TopAlgs. In fact Algorithms are protected against getting
  // started twice.
  for (auto& ita : m_topAlgList) {
    sc = ita->sysStart();
    if( !sc.isSuccess() ) {
      error() << "Unable to start Algorithm: " << ita->name() << endmsg;
      return sc;
    }
  }
  for (auto& ita : m_outStreamList ) {
    sc = ita->sysStart();
    if( !sc.isSuccess() ) {
      error() << "Unable to start Output Stream: " << ita->name() << endmsg;
      return sc;
    }
  }
  return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::stop
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::stop()    {

  StatusCode sc = StatusCode::SUCCESS;

  // Stop all the TopAlgs. In fact Algorithms are protected against getting
  // stopped twice.
  for (auto & ita : m_topAlgList ) {
    sc = ita->sysStop();
    if( !sc.isSuccess() ) {
      error() << "Unable to stop Algorithm: " << ita->name() << endmsg;
      return sc;
    }
  }
  for (auto &ita : m_outStreamList ) {
    sc = ita->sysStop();
    if( !sc.isSuccess() ) {
      error() << "Unable to stop Output Stream: " << ita->name() << endmsg;
      return sc;
    }
  }

  return Service::stop();
}

//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::reinitialize() {
  StatusCode sc = StatusCode::SUCCESS;

  // Reinitialize all the TopAlgs.
  for (auto& ita : m_topAlgList) {
    sc = ita->sysReinitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to reinitialize Algorithm: " << ita->name() << endmsg;
      return sc;
    }
  }
  for (auto &ita : m_outStreamList ) {
    sc = ita->sysReinitialize();
    if( !sc.isSuccess() ) {
      error() << "Unable to reinitialize Output Stream: " << ita->name() << endmsg;
      return sc;
    }
  }

  return sc;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::restart
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::restart() {
  StatusCode sc = StatusCode::SUCCESS;

  // Restart all the TopAlgs.
  for (auto& ita : m_topAlgList ) {
    sc = ita->sysRestart();
    if( !sc.isSuccess() ) {
      error() << "Unable to restart Algorithm: " << ita->name() << endmsg;
      return sc;
    }
  }
  for (auto& ita : m_outStreamList ) {
    sc = ita->sysRestart();
    if( !sc.isSuccess() ) {
      error() << "Unable to restart Output Stream: " << ita->name() << endmsg;
      return sc;
    }
  }

  return sc;
}

//--------------------------------------------------------------------------------------------
// implementation of IService::finalize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::finalize() {
  StatusCode sc = StatusCode::SUCCESS;
  StatusCode scRet = StatusCode::SUCCESS;
  // Call the finalize() method of all top algorithms
  for (auto&  ita : m_topAlgList ) {
    sc = ita->sysFinalize();
    if( !sc.isSuccess() ) {
      scRet = StatusCode::FAILURE;
      warning() << "Finalization of algorithm " << ita->name() << " failed" << endmsg;
    }
  }
  // Call the finalize() method of all Output streams
  for (auto &ita : m_outStreamList) {
    sc = ita->sysFinalize();
    if( !sc.isSuccess() ) {
      scRet = StatusCode::FAILURE;
      warning() << "Finalization of algorithm " << ita->name() << " failed" << endmsg;
    }
  }
  // release all top algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  for (auto &ita : m_topAlgList ) {
    if (algMan->removeAlgorithm(ita).isFailure()) {
      scRet = StatusCode::FAILURE;
      warning() << "Problems removing Algorithm " << ita->name() << endmsg;
    }
  }
  m_topAlgList.clear();

  // release all output streams
  for (auto& ita : m_outStreamList) {
    ita->release();
  }
  m_outStreamList.clear();
  if ( sc.isSuccess() ) m_state = FINALIZED;

  m_incidentSvc->removeListener(m_abortEventListener, IncidentType::AbortEvent);
  m_abortEventListener = nullptr; // release

  m_incidentSvc = nullptr; // release
  m_appMgrUI = nullptr; // release

  sc = Service::finalize();

  if (sc.isFailure()) {
    scRet = StatusCode::FAILURE;
    error() << "Problems finalizing Service base class" << endmsg;
  }

  return scRet;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::nextEvent(int /* maxevt */)   {
  error() << "This method cannot be called on an object of type "
          << System::typeinfoName(typeid(*this)) << endmsg;
  return StatusCode::FAILURE;
}

//--------------------------------------------------------------------------------------------
// IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::executeRun( int maxevt ) {
  StatusCode  sc;
  bool eventfailed = false;

  // Call the beginRun() method of all top algorithms
  for (auto& ita : m_topAlgList ) {
    sc = ita->sysBeginRun();
    if (!sc.isSuccess()) {
      warning() << "beginRun() of algorithm " << ita->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // Call now the nextEvent(...)
  sc = nextEvent(maxevt);
  if (!sc.isSuccess()) eventfailed = true;

  // Call the endRun() method of all top algorithms
  for (auto& ita : m_topAlgList) {
    sc = ita->sysEndRun();
    if (!sc.isSuccess()) {
      warning() << "endRun() of algorithm " << ita->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  return eventfailed ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

namespace {
  /// Helper class to set the application return code in case of early exit
  /// (e.g. exception).
  class RetCodeGuard {
  public:
    inline RetCodeGuard(const SmartIF<IProperty> &appmgr, int retcode):
      m_appmgr(appmgr), m_retcode(retcode) {}
    inline void ignore() {
      m_retcode = Gaudi::ReturnCode::Success;
    }
    inline ~RetCodeGuard() {
      if (UNLIKELY(Gaudi::ReturnCode::Success != m_retcode)) {
        Gaudi::setAppReturnCode(m_appmgr, m_retcode);
      }
    }
  private:
    SmartIF<IProperty> m_appmgr;
    int m_retcode;
  };
}
//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::executeEvent(void* /* par */)    {
  bool eventfailed = false;

  // Call the resetExecuted() method of ALL "known" algorithms
  // (before we were reseting only the topalgs)
  SmartIF<IAlgManager> algMan(serviceLocator());
  if (LIKELY(algMan.isValid())) {
    const auto& allAlgs = algMan->getAlgorithms();
    std::for_each( std::begin(allAlgs), std::end(allAlgs),
                   [](IAlgorithm* alg) {
                        if (LIKELY(alg!=nullptr)) alg->resetExecuted();
    } );
  }

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const SmartIF<IProperty> appmgr(serviceLocator());
  // Call the execute() method of all top algorithms
  for (auto&  ita : m_topAlgList ) {
    StatusCode sc(StatusCode::FAILURE);
    try {
      if (UNLIKELY(m_abortEvent)) {
        DEBMSG << "AbortEvent incident fired by "
               << m_abortEventSource << endmsg;
        m_abortEvent = false;
        sc.ignore();
        break;
      }
      RetCodeGuard rcg(appmgr, Gaudi::ReturnCode::UnhandledException);
      sc = ita->sysExecute();
      rcg.ignore(); // disarm the guard
    } catch ( const GaudiException& Exception ) {
      fatal() << ".executeEvent(): Exception with tag=" << Exception.tag()
              << " thrown by " << ita->name() << endmsg;
      error() << Exception << endmsg;
    } catch ( const std::exception& Exception ) {
      fatal() << ".executeEvent(): Standard std::exception thrown by "
              << ita->name() << endmsg;
      error() << Exception.what()  << endmsg;
    } catch(...) {
      fatal() << ".executeEvent(): UNKNOWN Exception thrown by "
              << ita->name() << endmsg;
    }

    if (UNLIKELY(!sc.isSuccess()))  {
      warning() << "Execution of algorithm " << ita->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // ensure that the abortEvent flag is cleared before the next event
  if (UNLIKELY(m_abortEvent)) {
    DEBMSG << "AbortEvent incident fired by " << m_abortEventSource << endmsg;
    m_abortEvent = false;
  }

  // Call the execute() method of all output streams
  for (auto&  ito : m_outStreamList) {
    ito->resetExecuted();
    StatusCode sc;
    sc = ito->sysExecute();
    if (UNLIKELY(!sc.isSuccess()))  {
      warning() << "Execution of output stream " << ito->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // Check if there was an error processing current event
  if (UNLIKELY(eventfailed)){
    error() << "Error processing event loop." << endmsg;
    return StatusCode(StatusCode::FAILURE,true);
  }
  return StatusCode(StatusCode::SUCCESS,true);
}
//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::stopRun()
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::stopRun() {
  // Set the application return code
  SmartIF<IProperty> appmgr(serviceLocator());
  if(Gaudi::setAppReturnCode(appmgr, Gaudi::ReturnCode::ScheduledStop).isFailure()) {
    error() << "Could not set return code of the application ("
            << Gaudi::ReturnCode::ScheduledStop << ")" << endmsg;
  }
  m_scheduledStop = true;
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
// Top algorithm List handler
//--------------------------------------------------------------------------------------------
void MinimalEventLoopMgr::topAlgHandler( Property& /* theProp */ )  {
  if ( !(decodeTopAlgs( )).isSuccess() ) {
    throw GaudiException("Failed to initialize Top Algorithms streams.",
                         "MinimalEventLoopMgr::topAlgHandler",
                         StatusCode::FAILURE);
  }
}

//--------------------------------------------------------------------------------------------
// decodeTopAlgNameList & topAlgNameListHandler
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::decodeTopAlgs()    {
  StatusCode sc = StatusCode::SUCCESS;
  if ( CONFIGURED == m_state || INITIALIZED == m_state ) {
    SmartIF<IAlgManager> algMan(serviceLocator());
    if ( algMan.isValid())   {
      // Reset the existing Top Algorithm List
      m_topAlgList.clear( );
      m_topAlgList.reserve( m_topAlgNames.value().size() );
      for (const auto& it : m_topAlgNames.value( )) {
        Gaudi::Utils::TypeNameString item{it};
        // Got the type and name. Now creating the algorithm, avoiding duplicate creation.
        std::string item_name = item.name() + getGaudiThreadIDfromName(name());
        const bool CREATE = false;
        SmartIF<IAlgorithm> alg = algMan->algorithm(item_name, CREATE);
        if (alg.isValid()) {
          DEBMSG << "Top Algorithm " << item_name << " already exists" << endmsg;
        }
        else {
          DEBMSG << "Creating Top Algorithm " << item.type() << " with name " << item_name << endmsg;
          IAlgorithm *ialg = nullptr;
          StatusCode sc1 = algMan->createAlgorithm(item.type(), item_name, ialg);
          if( !sc1.isSuccess() ) {
            error() << "Unable to create Top Algorithm " << item.type() << " with name " << item_name << endmsg;
            return sc1;
          }
          alg = ialg; // manage reference counting
        }
        m_topAlgList.push_back(alg);
      }
      return sc;
    }
    sc = StatusCode::FAILURE;
  }
  return sc;
}

//--------------------------------------------------------------------------------------------
// Output stream List handler
//--------------------------------------------------------------------------------------------
void MinimalEventLoopMgr::outStreamHandler( Property& /* theProp */ )         {
  if ( !(decodeOutStreams( )).isSuccess() ) {
    throw GaudiException("Failed to initialize output streams.",
                         "MinimalEventLoopMgr::outStreamHandler",
                         StatusCode::FAILURE);
  }
}

//--------------------------------------------------------------------------------------------
// decodeOutStreamNameList & outStreamNameListHandler
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::decodeOutStreams( )    {
  StatusCode sc = StatusCode::SUCCESS;
  if ( CONFIGURED == m_state || INITIALIZED == m_state ) {
    SmartIF<IAlgManager> algMan(serviceLocator());
    if ( algMan.isValid() )   {
      // Reset the existing Top Algorithm List
      m_outStreamList.clear();
      for (const auto& it : m_outStreamNames.value( ) ) {
        Gaudi::Utils::TypeNameString item(it, m_outStreamType);
        DEBMSG << "Creating " << m_outStreamType <<  it << endmsg;
        const bool CREATE = false;
        SmartIF<IAlgorithm> os = algMan->algorithm( item, CREATE );
        if (os.isValid()) {
          DEBMSG << "Output Stream " << item.name() << " already exists" << endmsg;
        } else {
          DEBMSG << "Creating Output Stream " << it << endmsg;
          IAlgorithm* ios = 0;
          StatusCode sc1 = algMan->createAlgorithm( item.type(), item.name(), ios );
          if( !sc1.isSuccess() ) {
            error() << "Unable to create Output Stream " << it << endmsg;
            return sc1;
          }
          os = ios; // manage reference counting
        }
        m_outStreamList.push_back( os );
      }
     return sc;
    }
    sc = StatusCode::FAILURE;
  }
  return sc;
}



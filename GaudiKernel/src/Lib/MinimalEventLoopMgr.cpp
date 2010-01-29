// $Id: MinimalEventLoopMgr.cpp,v 1.7 2008/07/15 12:48:18 marcocle Exp $
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

#include "GaudiKernel/MinimalEventLoopMgr.h"

namespace {
  class AbortEventListener: public implements1<IIncidentListener> {
  public:
    AbortEventListener(bool &flag, std::string &source):m_flag(flag),
                                                        m_source(source){
      addRef(); // Initial count set to 1
    }
    virtual ~AbortEventListener() {}
    /// Inform that a new incident has occurred
    virtual void handle(const Incident& i) {
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
  m_state = OFFLINE;
  m_scheduledStop = false;
  m_abortEvent = false;
}

//--------------------------------------------------------------------------------------------
// Standard Destructor
//--------------------------------------------------------------------------------------------
MinimalEventLoopMgr::~MinimalEventLoopMgr()   {
  m_state = OFFLINE;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::initialize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::initialize()    {

  MsgStream log(msgSvc(), name());

  if ( !m_appMgrUI.isValid() ) {
    return StatusCode::FAILURE;
  }

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() )   {
    log << MSG::ERROR << "Failed to initialize Service Base class." << endmsg;
    return StatusCode::FAILURE;
  }

  SmartIF<IProperty> prpMgr(serviceLocator());
  if ( ! prpMgr.isValid() )   {
    log << MSG::ERROR << "Error retrieving AppMgr interface IProperty." << endmsg;
    return StatusCode::FAILURE;
  }
  else {
    if ( m_topAlgNames.value().size() == 0 )    {
      setProperty(prpMgr->getProperty("TopAlg")).ignore();
    }
    if ( m_outStreamNames.value().size() == 0 )   {
      setProperty(prpMgr->getProperty("OutStream")).ignore();
    }
  }

  // Get the references to the services that are needed by the ApplicationMgr itself
  m_incidentSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incidentSvc.isValid() )  {
    log << MSG::FATAL << "Error retrieving IncidentSvc." << endmsg;
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
    log << MSG::ERROR << "Failed to initialize Output streams." << endmsg;
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
    log << MSG::ERROR << "Failed to initialize Top Algorithms streams." << endmsg;
    m_state = CONFIGURED;
    return sc;
  }

  ListAlg::iterator ita;
  // Initialize all the new TopAlgs. In fact Algorithms are protected against getting
  // initialized twice.
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysInitialize();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to initialize Algorithm: " << (*ita)->name() << endmsg;
      return sc;
    }
  }
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysInitialize();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to initialize Output Stream: " << (*ita)->name() << endmsg;
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

  MsgStream log(msgSvc(), name());

  ListAlg::iterator ita;
  // Start all the new TopAlgs. In fact Algorithms are protected against getting
  // started twice.
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysStart();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to start Algorithm: " << (*ita)->name() << endmsg;
      return sc;
    }
  }
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysStart();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to start Output Stream: " << (*ita)->name() << endmsg;
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

  MsgStream log(msgSvc(), name());

  ListAlg::iterator ita;
  // Stop all the TopAlgs. In fact Algorithms are protected against getting
  // stopped twice.
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysStop();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to stop Algorithm: " << (*ita)->name() << endmsg;
      return sc;
    }
  }
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysStop();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to stop Output Stream: " << (*ita)->name() << endmsg;
      return sc;
    }
  }

  return Service::stop();
}

//--------------------------------------------------------------------------------------------
// implementation of IService::reinitialize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::reinitialize() {
  MsgStream log( msgSvc(), name() );
  StatusCode sc = StatusCode::SUCCESS;
  ListAlg::iterator ita;

  // Reinitialize all the TopAlgs.
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysReinitialize();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to reinitialize Algorithm: " << (*ita)->name() << endmsg;
      return sc;
    }
  }
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysReinitialize();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to reinitialize Output Stream: " << (*ita)->name() << endmsg;
      return sc;
    }
  }

  return sc;
}
//--------------------------------------------------------------------------------------------
// implementation of IService::restart
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::restart() {
  MsgStream log( msgSvc(), name() );
  StatusCode sc = StatusCode::SUCCESS;
  ListAlg::iterator ita;

  // Restart all the TopAlgs.
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysRestart();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to restart Algorithm: " << (*ita)->name() << endmsg;
      return sc;
    }
  }
  for (ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysRestart();
    if( !sc.isSuccess() ) {
      log << MSG::ERROR << "Unable to restart Output Stream: " << (*ita)->name() << endmsg;
      return sc;
    }
  }

  return sc;
}

//--------------------------------------------------------------------------------------------
// implementation of IService::finalize
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::finalize()    {
  MsgStream log( msgSvc(), name() );
  StatusCode sc = StatusCode::SUCCESS;
  StatusCode scRet = StatusCode::SUCCESS;
  // Call the finalize() method of all top algorithms
  ListAlg::iterator ita;
  for ( ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysFinalize();
    if( !sc.isSuccess() ) {
      scRet = StatusCode::FAILURE;
      log << MSG::WARNING << "Finalization of algorithm " << (*ita)->name() << " failed" << endmsg;
    }
  }
  // Call the finalize() method of all Output streams
  for ( ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    sc = (*ita)->sysFinalize();
    if( !sc.isSuccess() ) {
      scRet = StatusCode::FAILURE;
      log << MSG::WARNING << "Finalization of algorithm " << (*ita)->name() << " failed" << endmsg;
    }
  }
  // release all top algorithms
  SmartIF<IAlgManager> algMan(serviceLocator());
  for ( ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    if (algMan->removeAlgorithm(*ita).isFailure()) {
      scRet = StatusCode::FAILURE;
      log << MSG::ERROR << "Problems removing Algorithm " << (*ita)->name()
          << endmsg;
    }
  }
  m_topAlgList.clear();

  // release all output streams
  for ( ita = m_outStreamList.begin(); ita != m_outStreamList.end(); ita++ ) {
    (*ita)->release();
  }
  m_outStreamList.clear();
  if ( sc.isSuccess() ) m_state = FINALIZED;

  m_incidentSvc->removeListener(m_abortEventListener, IncidentType::AbortEvent);
  m_abortEventListener = 0; // release

  m_incidentSvc = 0; // release
  m_appMgrUI = 0; // release

  sc = Service::finalize();

  if (sc.isFailure()) {
    scRet = StatusCode::FAILURE;
    log << MSG::ERROR << "Problems finalizing Service base class" << endmsg;
  }

  return scRet;
}

//--------------------------------------------------------------------------------------------
// implementation of IAppMgrUI::nextEvent
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::nextEvent(int /* maxevt */)   {
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "This method cannot be called on an object of type "
      << System::typeinfoName(typeid(*this)) << endmsg;
  return StatusCode::FAILURE;
}

//--------------------------------------------------------------------------------------------
// IEventProcessing::executeRun
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::executeRun( int maxevt ) {
  StatusCode  sc;
  MsgStream log( msgSvc(), name() );
  ListAlg::iterator ita;
  bool eventfailed = false;

  // Call the beginRun() method of all top algorithms
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysBeginRun();
    if( !sc.isSuccess() ) {
      log << MSG::WARNING << "beginRun() of algorithm " << (*ita)->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // Call now the nextEvent(...)
  sc = nextEvent(maxevt);
  if( !sc.isSuccess() ) {
    eventfailed = true;
  }

  // Call the endRun() method of all top algorithms
  for (ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    sc = (*ita)->sysEndRun();
    if( !sc.isSuccess() ) {
      log << MSG::WARNING << "endRun() of algorithm " << (*ita)->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  if( eventfailed ){
    return StatusCode::FAILURE;
  }
  else {
    return StatusCode::SUCCESS;
  }
}

//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::executeEvent(void* par)
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::executeEvent(void* /* par */)    {
  bool eventfailed = false;

  // Call the resetExecuted() method of ALL "known" algorithms
  // (before we were reseting only the topalgs)
  SmartIF<IAlgManager> algMan(serviceLocator());
  if ( algMan.isValid() ) {
    const ListAlgPtrs& allAlgs = algMan->getAlgorithms() ;
    for( ListAlgPtrs::const_iterator ialg = allAlgs.begin() ; allAlgs.end() != ialg ; ++ialg ) {
      if ( 0 != *ialg ) (*ialg)->resetExecuted();
    }
  }

  // Call the execute() method of all top algorithms
  for (ListAlg::iterator ita = m_topAlgList.begin(); ita != m_topAlgList.end(); ita++ ) {
    StatusCode sc(StatusCode::FAILURE);
    try {
      if (m_abortEvent){
        MsgStream log ( msgSvc() , name() );
        log << MSG::DEBUG << "AbortEvent incident fired by "
                          << m_abortEventSource << endmsg;
        m_abortEvent = false;
        sc.ignore();
        break;
      }
      sc = (*ita)->sysExecute();
    } catch ( const GaudiException& Exception ) {
      MsgStream log ( msgSvc() , "MinimalEventLoopMgr.executeEvent()" );
      log << MSG::FATAL << " Exception with tag=" << Exception.tag()
          << " thrown by " << (*ita)->name() << endmsg;
      log << MSG::ERROR << Exception << endmsg;
    } catch ( const std::exception& Exception ) {
      MsgStream log ( msgSvc() , "MinimalEventLoopMgr.executeEvent()" );
      log << MSG::FATAL << " Standard std::exception thrown by "
          << (*ita)->name() << endmsg;
      log << MSG::ERROR << Exception.what()  << endmsg;
    } catch(...) {
      MsgStream log ( msgSvc() , "MinimalEventLoopMgr.executeEvent()" );
      log << MSG::FATAL << "UNKNOWN Exception thrown by "
          << (*ita)->name() << endmsg;
    }

    if( !sc.isSuccess() )  {
      MsgStream log( msgSvc(), name() );
      log << MSG::WARNING << "Execution of algorithm " << (*ita)->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // ensure that the abortEvent flag is cleared before the next event
  if (m_abortEvent){
    if (outputLevel() <= MSG::DEBUG) {
      MsgStream log ( msgSvc() , name() );
      log << MSG::DEBUG << "AbortEvent incident fired by "
                        << m_abortEventSource << endmsg;
    }
    m_abortEvent = false;
  }

  // Call the execute() method of all output streams
  for (ListAlg::iterator ito = m_outStreamList.begin(); ito != m_outStreamList.end(); ito++ ) {
    (*ito)->resetExecuted();
      StatusCode sc;
      sc = (*ito)->sysExecute();
    if( !sc.isSuccess() )  {
      MsgStream log( msgSvc(), name() );
      log << MSG::WARNING << "Execution of output stream " << (*ito)->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  // Check if there was an error processing current event
  if( eventfailed ){
    MsgStream log( msgSvc(), name() );
    log << MSG::ERROR << "Error processing event loop." << endmsg;
    return StatusCode(StatusCode::FAILURE,true);
  }
  return StatusCode(StatusCode::SUCCESS,true);
}
//--------------------------------------------------------------------------------------------
// Implementation of IEventProcessor::stopRun()
//--------------------------------------------------------------------------------------------
StatusCode MinimalEventLoopMgr::stopRun() {
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
    MsgStream log(msgSvc(), name());
    if ( algMan.isValid())   {
      // Reset the existing Top Algorithm List
      m_topAlgList.clear( );
      const std::vector<std::string>& algNames = m_topAlgNames.value( );
      for (VectorName::const_iterator it = algNames.begin(); it != algNames.end(); it++) {
        Gaudi::Utils::TypeNameString item(*it);
        // Got the type and name. Now creating the algorithm, avoiding duplicate creation.
        std::string item_name = item.name() + getGaudiThreadIDfromName(name());
        const bool CREATE = false;
        SmartIF<IAlgorithm> alg = algMan->algorithm(item_name, CREATE);
        if (alg.isValid()) {
          log << MSG::DEBUG << "Top Algorithm " << item_name << " already exists" << endmsg;
        }
        else {
          log << MSG::DEBUG << "Creating Top Algorithm " << item.type() << " with name " << item_name << endmsg;
          IAlgorithm *ialg = 0;
          StatusCode sc1 = algMan->createAlgorithm(item.type(), item_name, ialg);
          if( !sc1.isSuccess() ) {
            log << MSG::ERROR << "Unable to create Top Algorithm " << item.type() << " with name " << item_name << endmsg;
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
    MsgStream log(msgSvc(), name());
    SmartIF<IAlgManager> algMan(serviceLocator());
    if ( algMan.isValid() )   {
      // Reset the existing Top Algorithm List
      m_outStreamList.clear();
      const std::vector<std::string>& algNames = m_outStreamNames.value( );
      for (VectorName::const_iterator it = algNames.begin(); it != algNames.end(); it++) {
        Gaudi::Utils::TypeNameString item(*it, m_outStreamType);
        log << MSG::DEBUG << "Creating " << m_outStreamType <<  (*it) << endmsg;
        const bool CREATE = false;
        SmartIF<IAlgorithm> os = algMan->algorithm( item, CREATE );
        if (os.isValid()) {
          log << MSG::DEBUG << "Output Stream " << item.name() << " already exists" << endmsg;
        }
        else {
          log << MSG::DEBUG << "Creating Output Stream " << (*it) << endmsg;
          IAlgorithm* ios = 0;
          StatusCode sc1 = algMan->createAlgorithm( item.type(), item.name(), ios );
          if( !sc1.isSuccess() ) {
            log << MSG::ERROR << "Unable to create Output Stream " << (*it) << endmsg;
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



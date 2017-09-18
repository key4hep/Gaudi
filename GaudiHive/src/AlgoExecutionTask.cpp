// local includes
#include "AlgoExecutionTask.h"
#include "RetCodeGuard.h"

// Framework
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

#include <functional>

tbb::task* AlgoExecutionTask::execute() {

  IAlgorithm *ialg = m_algorithm.get();
  Algorithm* this_algo = dynamic_cast<Algorithm*>(ialg);  
  if (!this_algo){
    throw GaudiException ("Cast to Algorithm failed!","AlgoExecutionTask",
                          StatusCode::FAILURE);
  }
  
  bool eventfailed=false;
  Gaudi::Hive::setCurrentContext( m_evtCtx );

  // TODO reproduce the commented out functionality in a different service
  //m_schedSvc->addAlg(this_algo, m_evtCtx, pthread_self());

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const SmartIF<IProperty> appmgr(m_serviceLocator);
  
  SmartIF<IMessageSvc> messageSvc (m_serviceLocator);
  MsgStream log(messageSvc, "AlgoExecutionTask");

  // select the appropriate store
  this_algo->whiteboard()->selectStore(m_evtCtx->valid() ? 
                                       m_evtCtx->slot() : 0).ignore();

  StatusCode sc(StatusCode::FAILURE);
  try {
    RetCodeGuard rcg(appmgr, Gaudi::ReturnCode::UnhandledException);
    sc = m_algorithm->sysExecute(*m_evtCtx);
    if (UNLIKELY(!sc.isSuccess()))  {
      log << MSG::WARNING << "Execution of algorithm " 
          << m_algorithm->name() << " failed" << endmsg;
      eventfailed = true;
    }    
    rcg.ignore(); // disarm the guard
  } catch ( const GaudiException& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag()
            << " thrown by " << m_algorithm->name() << endmsg;
    log << MSG::ERROR << Exception << endmsg;
    eventfailed = true;
  } catch ( const std::exception& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by "
            << m_algorithm->name() << endmsg;
    log << MSG::ERROR <<  Exception.what()  << endmsg;
    eventfailed = true;
  } catch(...) {
    log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by "
            << m_algorithm->name() << endmsg;
    eventfailed = true;
  }  

  // Commit all DataHandles
  this_algo->commitHandles();

  // DP it is important to propagate the failure of an event.
  // We need to stop execution when this happens so that execute run can 
  // then receive the FAILURE
  m_aess->algExecState(ialg,*m_evtCtx).setExecuted(true);
  m_aess->algExecState(ialg,*m_evtCtx).setExecStatus(sc);
  m_aess->updateEventStatus(eventfailed,*m_evtCtx);

  // TODO reproduce the commented out functionality in a different service
  //m_schedSvc->delAlg(this_algo);

  Gaudi::Hive::setCurrentContextEvt(-1);

  return nullptr;
}

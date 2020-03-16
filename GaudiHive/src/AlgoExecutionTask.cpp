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
// local includes
#include "AlgoExecutionTask.h"
#include "RetCodeGuard.h"

// Framework
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <Gaudi/Algorithm.h>

#include <functional>

namespace Gaudi {
  namespace Concurrency {
    extern thread_local bool ThreadInitDone;
  }
} // namespace Gaudi

tbb::task* AlgoExecutionTask::execute() {

  // Get queue data
  AvalancheSchedulerSvc::TaskSpec taskSpec;
  if ( !m_scheduler->m_scheduledQueue.try_pop( taskSpec ) ) return nullptr;

  std::string   algName  = m_scheduler->index2algname( taskSpec.algIndex );
  EventContext& evtCtx   = *taskSpec.contextPtr;
  IAlgorithm*   iAlgoPtr = taskSpec.algPtr;

  Gaudi::Algorithm* this_algo = dynamic_cast<Gaudi::Algorithm*>( iAlgoPtr );
  if ( !this_algo ) { throw GaudiException( "Cast to Algorithm failed!", "AlgoExecutionTask", StatusCode::FAILURE ); }

  bool eventfailed = false;
  Gaudi::Hive::setCurrentContext( evtCtx );

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const SmartIF<IProperty> appmgr( m_serviceLocator );

  SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
  MsgStream            log( messageSvc, "AlgoExecutionTask" );

  if ( !Gaudi::Concurrency::ThreadInitDone ) {
    log << MSG::DEBUG << "New thread detected: 0x" << std::hex << pthread_self() << std::dec
        << ". Doing thread local initialization." << endmsg;
    SmartIF<IThreadPoolSvc> tps;
    tps = m_serviceLocator->service( "ThreadPoolSvc" );
    if ( !tps.isValid() ) {
      log << MSG::ERROR << "unable to get the ThreadPoolSvc to trigger thread local initialization" << endmsg;
      throw GaudiException( "retrieval of ThrePoolSvc failed", "AlgoExecutionTask", StatusCode::FAILURE );
    }

    tps->initThisThread();
  }

  // select the appropriate store
  this_algo->whiteboard()->selectStore( evtCtx.valid() ? evtCtx.slot() : 0 ).ignore();
  StatusCode sc( StatusCode::FAILURE );
  try {
    RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );
    sc = iAlgoPtr->sysExecute( evtCtx );
    if ( UNLIKELY( !sc.isSuccess() ) ) {
      log << MSG::WARNING << "Execution of algorithm " << algName << " failed" << endmsg;
      eventfailed = true;
    }
    rcg.ignore(); // disarm the guard
  } catch ( const GaudiException& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by " << algName
        << endmsg;
    log << MSG::ERROR << Exception << endmsg;
    eventfailed = true;
  } catch ( const std::exception& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << algName << endmsg;
    log << MSG::ERROR << Exception.what() << endmsg;
    eventfailed = true;
  } catch ( ... ) {
    log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << algName << endmsg;
    eventfailed = true;
  }

  // DP it is important to propagate the failure of an event.
  // We need to stop execution when this happens so that execute run can
  // then receive the FAILURE
  m_aess->updateEventStatus( eventfailed, evtCtx );

  // Release algorithm
  m_scheduler->m_algResourcePool->releaseAlgorithm( algName, iAlgoPtr ).ignore();

  // Create a lambda to update scheduler state
  auto schedulerPtr = m_scheduler; // can't capture m_scheduler directly for some reason (implied this* ?)
  m_scheduler->m_actionsQueue.push( [schedulerPtr, taskSpec]() -> StatusCode {
    return schedulerPtr->promoteToExecuted( taskSpec.algIndex, taskSpec.slotIndex, taskSpec.contextPtr );
  } );

  Gaudi::Hive::setCurrentContextEvt( -1 );

  return nullptr;
}

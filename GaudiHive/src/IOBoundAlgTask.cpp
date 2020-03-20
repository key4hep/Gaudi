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
#include "IOBoundAlgTask.h"
#include "RetCodeGuard.h"

// Framework
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <Gaudi/Algorithm.h>

namespace Gaudi {
  namespace Concurrency {
    extern thread_local bool ThreadInitDone;
  }
} // namespace Gaudi

void IOBoundAlgTask::operator()() {

  IAlgorithm*       ialg      = m_algorithm.get();
  Gaudi::Algorithm* this_algo = dynamic_cast<Gaudi::Algorithm*>( ialg );
  if ( !this_algo ) { throw GaudiException( "Cast to Algorithm failed!", "BlockingTask", StatusCode::FAILURE ); }

  bool eventfailed = false;
  Gaudi::Hive::setCurrentContext( m_evtCtx );

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const SmartIF<IProperty> appmgr( m_serviceLocator );

  SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
  MsgStream            log( messageSvc, "BlockingTask" );

  if ( !Gaudi::Concurrency::ThreadInitDone ) {
    log << MSG::DEBUG << "New thread detected: 0x" << std::hex << pthread_self() << std::dec
        << ". Doing thread local initialization." << endmsg;
    if ( SmartIF<IThreadPoolSvc> tps{m_serviceLocator->service( "ThreadPoolSvc" )} ) {
      tps->initThisThread();
    } else {
      log << MSG::ERROR << "Unable to get the ThreadPoolSvc to trigger thread local initialization" << endmsg;
      throw GaudiException( "Retrieval of ThreadPoolSvc failed", "AlgoExecutionTask", StatusCode::FAILURE );
    }
  }

  // select the appropriate store
  this_algo->whiteboard()->selectStore( m_evtCtx.valid() ? m_evtCtx.slot() : 0 ).ignore();

  try {
    RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );

    if ( auto sc = m_algorithm->sysExecute( m_evtCtx ); UNLIKELY( !sc ) ) {
      log << MSG::WARNING << "Execution of algorithm " << m_algorithm->name() << " failed" << endmsg;
      eventfailed = true;
    }
    rcg.ignore(); // disarm the guard
  } catch ( const GaudiException& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by "
        << m_algorithm->name() << endmsg;
    log << MSG::ERROR << Exception << endmsg;
    eventfailed = true;
  } catch ( const std::exception& Exception ) {
    log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << m_algorithm->name() << endmsg;
    log << MSG::ERROR << Exception.what() << endmsg;
    eventfailed = true;
  } catch ( ... ) {
    log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << m_algorithm->name() << endmsg;
    eventfailed = true;
  }

  // DP it is important to propagate the failure of an event.
  // We need to stop execution when this happens so that execute run can
  // then receive the FAILURE
  m_aess->updateEventStatus( eventfailed, m_evtCtx );

  // schedule a sign-off of what happened in this task
  m_promote2ExecutedClosure();

  Gaudi::Hive::setCurrentContextEvt( -1 );
}

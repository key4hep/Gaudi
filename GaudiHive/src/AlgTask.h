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
#ifndef GAUDIHIVE_ALGTASK_H
#define GAUDIHIVE_ALGTASK_H

#include "AvalancheSchedulerSvc.h"
#include "RetCodeGuard.h"

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <Gaudi/Algorithm.h>

#include <functional>

namespace Gaudi {
  namespace Concurrency {
    extern thread_local bool ThreadInitDone;
  }
} // namespace Gaudi

class AlgTask {
public:
  AlgTask( AvalancheSchedulerSvc* scheduler, ISvcLocator* svcLocator, IAlgExecStateSvc* aem, bool blocking = false )
      : m_scheduler( scheduler ), m_aess( aem ), m_serviceLocator( svcLocator ), m_blocking( blocking ){};

  void operator()() const {

    SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
    MsgStream            log( messageSvc, "AlgTask" );

    // Get task specification dynamically if it was not provided statically
    AvalancheSchedulerSvc::TaskSpec ts;
    if ( !m_scheduler->next( ts, m_blocking ) ) {
      log << MSG::WARNING << "Missing specification while task is running" << endmsg;
      return;
    }

    EventContext& evtCtx   = *( ts.contextPtr );
    IAlgorithm*&  iAlgoPtr = ts.algPtr;

    Gaudi::Algorithm* this_algo = dynamic_cast<Gaudi::Algorithm*>( iAlgoPtr );
    if ( !this_algo ) { throw GaudiException( "Cast to Algorithm failed!", "AlgTask", StatusCode::FAILURE ); }

    bool eventfailed = false;
    Gaudi::Hive::setCurrentContext( evtCtx );

    // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
    const SmartIF<IProperty> appmgr( m_serviceLocator );

    if ( !Gaudi::Concurrency::ThreadInitDone ) {
      log << MSG::DEBUG << "New thread detected: 0x" << std::hex << pthread_self() << std::dec
          << ". Doing thread local initialization." << endmsg;
      if ( SmartIF<IThreadPoolSvc> tps{ m_serviceLocator->service( "ThreadPoolSvc" ) } ) {
        tps->initThisThread();
      } else {
        log << MSG::ERROR << "Unable to get the ThreadPoolSvc to trigger thread local initialization" << endmsg;
        throw GaudiException( "Retrieval of ThreadPoolSvc failed", "AlgTask", StatusCode::FAILURE );
      }
    }

    // select the appropriate store
    this_algo->whiteboard()->selectStore( evtCtx.valid() ? evtCtx.slot() : 0 ).ignore();
    try {
      RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );

      if ( UNLIKELY( iAlgoPtr->sysExecute( evtCtx ).isFailure() ) ) {
        log << MSG::WARNING << "Execution of algorithm " << ts.algName << " failed" << endmsg;
        eventfailed = true;
      }
      rcg.ignore(); // disarm the guard
    } catch ( const GaudiException& Exception ) {
      log << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by " << ts.algName
          << endmsg;
      log << MSG::ERROR << Exception << endmsg;
      eventfailed = true;
    } catch ( const std::exception& Exception ) {
      log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << ts.algName << endmsg;
      log << MSG::ERROR << Exception.what() << endmsg;
      eventfailed = true;
    } catch ( ... ) {
      log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << ts.algName << endmsg;
      eventfailed = true;
    }

    // A FAILURE in algorithm execution must be communicated to the framework
    m_aess->updateEventStatus( eventfailed, evtCtx );

    // Release algorithm
    m_scheduler->m_algResourcePool->releaseAlgorithm( ts.algName, iAlgoPtr ).ignore();

    // schedule a sign-off of the Algorithm execution
    m_scheduler->m_actionsQueue.push(
        [schdlr = this->m_scheduler, ts = std::move( ts )]() { return schdlr->signoff( ts ); } );

    Gaudi::Hive::setCurrentContextEvt( -1 );
  }

private:
  // Shortcuts to services
  AvalancheSchedulerSvc* m_scheduler;
  IAlgExecStateSvc*      m_aess;
  SmartIF<ISvcLocator>   m_serviceLocator;
  // Marks the task as CPU-blocking or not
  bool m_blocking{ false };
};

#endif

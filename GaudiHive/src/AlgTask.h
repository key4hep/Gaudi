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

template <class T>
class AlgTask : public T {
public:
  AlgTask( AvalancheSchedulerSvc* scheduler, ISvcLocator* svcLocator, IAlgExecStateSvc* aem )
      : m_scheduler( scheduler ), m_aess( aem ), m_serviceLocator( svcLocator ){};

  AlgTask( AvalancheSchedulerSvc::TaskSpec&& ts, AvalancheSchedulerSvc* scheduler, ISvcLocator* svcLocator,
           IAlgExecStateSvc* aem )
      : m_ts( std::move( ts ) ), m_scheduler( scheduler ), m_aess( aem ), m_serviceLocator( svcLocator ){};

  T* execute() override {

    SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
    MsgStream            log( messageSvc, "AlgTask" );

    // Get task specification dynamically if it was not provided statically
    if ( !m_ts.algPtr )
      if ( !m_scheduler->m_scheduledQueue.try_pop( m_ts ) ) {
        log << MSG::WARNING << "Specification not complete or void while task is running" << endmsg;
        return nullptr;
      }

    EventContext& evtCtx   = *( m_ts.contextPtr );
    IAlgorithm*&  iAlgoPtr = m_ts.algPtr;

    Gaudi::Algorithm* this_algo = dynamic_cast<Gaudi::Algorithm*>( iAlgoPtr );
    if ( !this_algo ) { throw GaudiException( "Cast to Algorithm failed!", "AlgTask", StatusCode::FAILURE ); }

    bool eventfailed = false;
    Gaudi::Hive::setCurrentContext( evtCtx );

    // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
    const SmartIF<IProperty> appmgr( m_serviceLocator );

    if ( !Gaudi::Concurrency::ThreadInitDone ) {
      log << MSG::DEBUG << "New thread detected: 0x" << std::hex << pthread_self() << std::dec
          << ". Doing thread local initialization." << endmsg;
      if ( SmartIF<IThreadPoolSvc> tps{m_serviceLocator->service( "ThreadPoolSvc" )} ) {
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

      if ( auto sc = iAlgoPtr->sysExecute( evtCtx ); UNLIKELY( sc.isFailure() ) ) {
        log << MSG::WARNING << "Execution of algorithm " << m_ts.algName << " failed" << endmsg;
        eventfailed = true;
      }
      rcg.ignore(); // disarm the guard
    } catch ( const GaudiException& Exception ) {
      log << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by " << m_ts.algName
          << endmsg;
      log << MSG::ERROR << Exception << endmsg;
      eventfailed = true;
    } catch ( const std::exception& Exception ) {
      log << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << m_ts.algName << endmsg;
      log << MSG::ERROR << Exception.what() << endmsg;
      eventfailed = true;
    } catch ( ... ) {
      log << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << m_ts.algName << endmsg;
      eventfailed = true;
    }

    // A FAILURE in algorithm execution must be communicated to the framework
    m_aess->updateEventStatus( eventfailed, evtCtx );

    // Release algorithm
    m_scheduler->m_algResourcePool->releaseAlgorithm( m_ts.algName, iAlgoPtr ).ignore();

    // schedule a sign-off of the Algorithm execution
    m_scheduler->m_actionsQueue.push(
        std::move( [schdlr = this->m_scheduler, ts = std::move( this->m_ts )]() { return schdlr->signoff( ts ); } ) );

    Gaudi::Hive::setCurrentContextEvt( -1 );

    return nullptr;
  }

  void operator()() { execute(); };

private:
  AvalancheSchedulerSvc::TaskSpec m_ts;
  AvalancheSchedulerSvc*          m_scheduler;
  IAlgExecStateSvc*               m_aess;
  SmartIF<ISvcLocator>            m_serviceLocator;
};

#endif

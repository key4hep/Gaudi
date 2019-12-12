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
#include "ThreadInitTask.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IThreadInitTool.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ToolHandle.h"
#include <thread>

namespace Gaudi {
  namespace Concurrency {
    thread_local bool ThreadInitDone{false};
  }
} // namespace Gaudi

std::atomic<bool> ThreadInitTask::m_execFailed( false );

tbb::task* ThreadInitTask::execute() {

  SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
  MsgStream            log( messageSvc, "ThreadInitTask" );

  const auto debug = log.level() <= MSG::DEBUG;

  if ( debug )
    log << MSG::DEBUG << "execute() in thread 0x" << std::hex << pthread_self() << " at " << this << std::dec
        << "  state: " << ( m_terminate ? "terminate" : "initialize" ) << endmsg;

  if ( !m_terminate && Gaudi::Concurrency::ThreadInitDone ) {
    log << MSG::WARNING << "thread initialization has already been done on thread " << std::hex << pthread_self()
        << endmsg;
  }

  if ( m_tools.retrieve().isFailure() ) {
    log << MSG::ERROR << "unable to retrieve ToolHandleArray " << m_tools << endmsg;
    m_execFailed = true;
  } else {

    if ( m_tools.empty() ) {
      log << MSG::DEBUG << "no entries in Tool Array" << endmsg;
      if ( !m_terminate ) { Gaudi::Concurrency::ThreadInitDone = true; }
    } else {
      if ( debug ) log << MSG::DEBUG << "executing in thread 0x" << std::hex << pthread_self() << std::dec << endmsg;

      // only call terminate for threads that have been initialized
      if ( m_terminate && !Gaudi::Concurrency::ThreadInitDone ) {
        log << MSG::INFO << "Not calling terminateThread for thread 0x" << std::hex << pthread_self()
            << " as it has not been initialized" << endmsg;
      } else {

        for ( auto& t : m_tools ) {
          try {

            if ( debug ) log << MSG::DEBUG << "calling IThreadInitTool " << t << endmsg;

            if ( !m_terminate ) {
              t->initThread();
              Gaudi::Concurrency::ThreadInitDone = true;
            } else {
              t->terminateThread();
            }

          } catch ( const GaudiException& exc ) {
            log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x" << std::hex << pthread_self() << std::dec
                << " threw GaudiException: " << exc << endmsg;
            m_execFailed = true;
          } catch ( const std::exception& exc ) {
            log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x" << std::hex << pthread_self() << std::dec
                << " threw std::exception: " << exc.what() << endmsg;
            m_execFailed = true;
          } catch ( ... ) {
            log << MSG::ERROR << "ThreadInitTool " << t << " in thread 0x" << std::hex << pthread_self() << std::dec
                << " threw unknown exception" << endmsg;
            m_execFailed = true;
          }
        }
      }

      m_tools.release().ignore();
    }
  }

  if ( m_barrier ) {
    log << MSG::DEBUG << "waiting at barrier in thread 0x" << std::hex << pthread_self() << std::dec << endmsg;
    m_barrier->wait();
  }

  return nullptr;
}

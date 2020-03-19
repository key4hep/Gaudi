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
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ContextSpecificPtr.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"

StatusCode IOBoundAlgTask::operator()() {

  IAlgorithm* ialg      = m_algorithm.get();
  Algorithm*  this_algo = dynamic_cast<Algorithm*>( ialg );
  if ( !this_algo ) { throw GaudiException( "Cast to Algorithm failed!", "AlgoExecutionTask", StatusCode::FAILURE ); }

  bool eventfailed = false;
  Gaudi::Hive::setCurrentContext( m_evtCtx );

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const SmartIF<IProperty> appmgr( m_serviceLocator );

  SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
  MsgStream            log( messageSvc, "AccelAlgoExecutionTask" );

  // select the appropriate store
  this_algo->whiteboard()->selectStore( m_evtCtx.valid() ? m_evtCtx.slot() : 0 ).ignore();

  StatusCode sc( StatusCode::FAILURE );
  try {
    RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );
    log << MSG::DEBUG << "Starting execution of algorithm " << m_algorithm->name() << endmsg;
    sc = m_algorithm->sysExecute( m_evtCtx );
    if ( UNLIKELY( !sc.isSuccess() ) ) {
      log << MSG::WARNING << "Execution of algorithm " << m_algorithm->name() << " failed" << endmsg;
      eventfailed = true;
    }
    log << MSG::DEBUG << "Stopped execution of algorithm " << m_algorithm->name() << endmsg;
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

  // update scheduler state
  sc = m_promote2ExecutedClosure();
  if ( UNLIKELY( sc.isFailure() ) ) {
    log << MSG::WARNING << "Cannot schedule update of algorithm state for " << m_algorithm->name() << endmsg;
  }

  Gaudi::Hive::setCurrentContextEvt( -1 );

  return sc;
}

/***********************************************************************************\
* (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/AsynchronousAlgorithm.h>
#include <Gaudi/IAuditor.h>
#include <GaudiKernel/IHiveWhiteBoard.h>

StatusCode Gaudi::AsynchronousAlgorithm::sysInitialize() {
  setAsynchronous( true );
  debug() << "Starting sysInitialize for AsynchronousAlgorithm" << endmsg;
  // Disable synchronous timeline recording and enable asynchronous timeline recording if synchronous was enabled before
  if ( !isInitialized() ) { m_doAsyncTimeline = setAsynchronousTimeline() && timelineSvc()->isEnabled(); }

  return Gaudi::Algorithm::sysInitialize();
}

StatusCode Gaudi::AsynchronousAlgorithm::sysExecute( const EventContext& ctx ) try {
  debug() << "Starting sysExecute for AsynchronousAlgorithm for event " << ctx << endmsg;

  s_ctx.reset( new EventContext( ctx ) );

  if ( m_doAsyncTimeline ) {
    s_timelineRecorder.reset( new ITimelineSvc::TimelineRecorder( timelineSvc()->getRecorder( name(), ctx ) ) );
  }

  auto sc = Gaudi::Algorithm::sysExecute( ctx );

  if ( m_doAsyncTimeline ) { s_timelineRecorder.reset( nullptr ); }

  return sc;
} catch ( const GaudiException& e ) {
  error() << "EXCEPTION MESSAGE IS: " << e.what() << endmsg;
  if ( m_doAsyncTimeline ) { s_timelineRecorder.reset( nullptr ); } // still need to reset the timeline recorder
  throw;
}

StatusCode Gaudi::AsynchronousAlgorithm::restoreAfterSuspend() const {
  if ( !whiteboard()->selectStore( s_ctx->slot() ).isSuccess() ) {
    error() << "Resetting slot from fiber_specific_ptr failed" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode Gaudi::AsynchronousAlgorithm::preSuspension() const {
  if ( m_doAsyncTimeline ) { s_timelineRecorder.reset( nullptr ); }
  if ( m_auditorSuspension && auditorSvc()->isEnabled() ) {
    auditorSvc()->before( IAuditor::Suspension, name(), *s_ctx );
  }

  return StatusCode::SUCCESS;
}

StatusCode Gaudi::AsynchronousAlgorithm::postResumption() const {
  if ( !whiteboard()->selectStore( s_ctx->slot() ).isSuccess() ) {
    error() << "Resetting slot from fiber_specific_ptr failed" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( m_auditorSuspension && auditorSvc()->isEnabled() ) {
    auditorSvc()->after( IAuditor::Suspension, name(), *s_ctx );
  }
  if ( m_doAsyncTimeline ) {
    s_timelineRecorder.reset( new ITimelineSvc::TimelineRecorder( timelineSvc()->getRecorder( name(), *s_ctx ) ) );
  }
  return StatusCode::SUCCESS;
}

StatusCode Gaudi::AsynchronousAlgorithm::decorateSuspension( std::function<StatusCode()> f ) const {
  // preSuspension
  try {
    if ( auto sc = preSuspension(); !sc.isSuccess() ) { return sc; }
  } catch ( const GaudiException& e ) {
    error() << "Exception in preSuspension(): " << e.what() << endmsg;
    return StatusCode::FAILURE;
  }

  // Callable
  StatusCode fsc = StatusCode::SUCCESS;
  try {
    fsc = std::invoke( std::move( f ) );
  } catch ( const GaudiException& e ) {
    error() << "Exception in decorated callable: " << e.what() << endmsg;
    fsc = StatusCode::FAILURE;
  }

  // postResumption happen happen even if callable failed/threw
  StatusCode psc = StatusCode::SUCCESS;
  try {
    psc = postResumption();
  } catch ( const GaudiException& e ) {
    error() << "Exception in postResumption(): " << e.what() << endmsg;
    psc = StatusCode::FAILURE;
  }

  // Preserve the original callable failure over a postResumption failure.
  if ( !fsc.isSuccess() ) { return fsc; }

  return psc;
}

StatusCode Gaudi::AsynchronousAlgorithm::yield() const {
  return decorateSuspension( []() -> StatusCode {
    boost::this_fiber::yield();
    return StatusCode::SUCCESS;
  } );
}

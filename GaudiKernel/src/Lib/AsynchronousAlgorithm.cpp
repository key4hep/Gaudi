/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// ============================================================================
// Gaudi
#include <Gaudi/AsynchronousAlgorithm.h>

StatusCode Gaudi::AsynchronousAlgorithm::sysInitialize() {
  setAsynchronous( true );
  msg() << MSG::DEBUG << "Starting sysInitialize for AsynchronousAlgorithm" << endmsg;
  return Gaudi::Algorithm::sysInitialize();
}

StatusCode Gaudi::AsynchronousAlgorithm::sysExecute( const EventContext& ctx ) try {
  msg() << MSG::DEBUG << "Starting sysExecute for AsynchronousAlgorithm on slot " << ctx.slot()
        << "with s_currentSlot = " << fmt::to_string( fmt::ptr( s_currentSlot.get() ) ) << endmsg;
  if ( s_currentSlot.get() == nullptr ) {
    s_currentSlot.reset( new std::size_t( ctx.slot() ) );
  } else if ( *s_currentSlot != ctx.slot() ) {
    error() << "Current slot is " << ctx.slot() << " but s_currentSlot exists and is " << *s_currentSlot << endmsg;
    return StatusCode::FAILURE;
  }
  return Gaudi::Algorithm::sysExecute( ctx );
} catch ( const GaudiException& e ) {
  error() << "EXCEPTION MESSAGE IS: " << e.what() << endmsg;
  throw;
}

StatusCode Gaudi::AsynchronousAlgorithm::restoreAfterSuspend() const {
  if ( !whiteboard()->selectStore( *s_currentSlot ).isSuccess() ) {
    msg() << MSG::ERROR << "Resetting slot from fiber_specific_ptr failed" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode Gaudi::AsynchronousAlgorithm::yield() const {
  boost::this_fiber::yield();
  return restoreAfterSuspend();
}

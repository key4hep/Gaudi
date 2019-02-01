#include "AlgsExecutionStates.h"

#include <cstdint>

namespace {
  constexpr auto transition( AlgsExecutionStates::State first, AlgsExecutionStates::State second ) {
    static_assert( sizeof( AlgsExecutionStates::State ) == 1, "no more than 255 states please!" );
    using ui16 = uint_fast16_t;
    return static_cast<ui16>( first ) * 256 + static_cast<ui16>( second );
  }
} // namespace

StatusCode AlgsExecutionStates::set( unsigned int iAlgo, State newState ) {
  if ( iAlgo >= m_states.size() ) {
    log() << MSG::ERROR << "Index out of bound (" << iAlgo << " / " << m_states.size() << ")" << endmsg;
    return StatusCode::FAILURE;
  }

  switch ( transition( m_states[iAlgo], newState ) ) {
  case transition( INITIAL, CONTROLREADY ):   // Fallthrough
  case transition( CONTROLREADY, DATAREADY ): // Fallthrough
  case transition( DATAREADY, SCHEDULED ):    // Fallthrough
  case transition( SCHEDULED, ERROR ):        // Fallthrough
  case transition( SCHEDULED, EVTACCEPTED ):  // Fallthrough
  case transition( SCHEDULED, EVTREJECTED ):
    m_states[iAlgo] = newState;
    return StatusCode::SUCCESS;
  default:
    log() << MSG::ERROR << "[AlgIndex " << iAlgo << "] Transition from " << m_states[iAlgo] << " to " << newState
          << " is not allowed" << endmsg;
    m_states[iAlgo] = ERROR;
    return StatusCode::FAILURE;
  }
}

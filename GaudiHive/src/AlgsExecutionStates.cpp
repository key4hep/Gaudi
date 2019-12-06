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

  // Allow cycling of a single state
  if ( m_states[iAlgo] == newState ) return StatusCode::SUCCESS;

  switch ( transition( m_states[iAlgo], newState ) ) {
  case transition( INITIAL, CONTROLREADY ):
    [[fallthrough]];
  case transition( CONTROLREADY, DATAREADY ):
    [[fallthrough]];
  case transition( DATAREADY, SCHEDULED ):
    [[fallthrough]];
  case transition( DATAREADY, RESOURCELESS ):
    [[fallthrough]];
  case transition( RESOURCELESS, SCHEDULED ):
    [[fallthrough]];
  case transition( SCHEDULED, ERROR ):
    [[fallthrough]];
  case transition( SCHEDULED, EVTACCEPTED ):
    [[fallthrough]];
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

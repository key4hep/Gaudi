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

  State oldState = m_states[iAlgo];

  // Allow cycling of a single state
  if ( oldState == newState ) return StatusCode::SUCCESS;

  switch ( transition( oldState, newState ) ) {
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
    m_algsInState[oldState].erase( iAlgo );
    m_algsInState[newState].insert( iAlgo );
    return StatusCode::SUCCESS;
  default:
    log() << MSG::ERROR << "[AlgIndex " << iAlgo << "] Transition from " << m_states[iAlgo] << " to " << newState
          << " is not allowed" << endmsg;
    m_states[iAlgo] = ERROR;
    m_algsInState[oldState].erase( iAlgo );
    m_algsInState[ERROR].insert( iAlgo );
    return StatusCode::FAILURE;
  }
}

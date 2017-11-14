#include "AlgsExecutionStates.h"

namespace
{
  constexpr int transition( AlgsExecutionStates::State first, AlgsExecutionStates::State second )
  {
    return first * 256 + second;
  }
}

// A simple map to easily translate a state to its name
std::map<AlgsExecutionStates::State, std::string> AlgsExecutionStates::stateNames = {
    {INITIAL, "INITIAL"},         {CONTROLREADY, "CONTROLREADY"}, {DATAREADY, "DATAREADY"}, {SCHEDULED, "SCHEDULED"},
    {EVTACCEPTED, "EVTACCEPTED"}, {EVTREJECTED, "EVTREJECTED"},   {ERROR, "ERROR"}};

StatusCode AlgsExecutionStates::updateState( unsigned int iAlgo, State newState )
{
  if ( iAlgo >= m_states.size() ) {
    MsgStream( m_MS, "AlgsExecutionStates" ) << MSG::ERROR << "Index out of bound (" << iAlgo << " / "
                                             << m_states.size() << ")" << endmsg;
    return StatusCode::FAILURE;
  }

  switch ( transition( m_states[iAlgo], newState ) ) {
  case transition( INITIAL, CONTROLREADY ):   // Fallthrough
  case transition( CONTROLREADY, DATAREADY ): // Fallthrough
  case transition( DATAREADY, SCHEDULED ):    // Fallthrough
  case transition( SCHEDULED, EVTACCEPTED ):  // Fallthrough
  case transition( SCHEDULED, EVTREJECTED ):
    m_states[iAlgo] = newState;
    return StatusCode::SUCCESS;
  default:
    MsgStream( m_MS, "AlgsExecutionStates" ) << MSG::ERROR << "[AlgIndex " << iAlgo << "] Transition from "
                                             << stateNames.at( m_states[iAlgo] ) << " to " << stateNames.at( newState )
                                             << " is not allowed" << endmsg;
    m_states[iAlgo] = ERROR;
    return StatusCode::FAILURE;
  }
}

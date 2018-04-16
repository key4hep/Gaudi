#include "AlgsExecutionStates.h"

// A simple map to easily translate a state to its name
std::map<AlgsExecutionStates::State, std::string> AlgsExecutionStates::stateNames = {
    {INITIAL, "INITIAL"},         {CONTROLREADY, "CONTROLREADY"}, {DATAREADY, "DATAREADY"}, {SCHEDULED, "SCHEDULED"},
    {EVTACCEPTED, "EVTACCEPTED"}, {EVTREJECTED, "EVTREJECTED"},   {ERROR, "ERROR"}};

StatusCode AlgsExecutionStates::updateState( unsigned int iAlgo, State newState )
{

  MsgStream          log( m_MS, "AlgExecutionStates" );
  const unsigned int states_size = m_states.size();

  if ( iAlgo >= states_size ) {
    log << MSG::ERROR << "Index out of bound (" << iAlgo << " and the size of the states vector is " << states_size
        << ")" << endmsg;

    return StatusCode::FAILURE;
  }

  switch ( newState ) {
  case INITIAL:
    log << MSG::ERROR << "[AlgIndex " << iAlgo << "] Transition to INITIAL is not defined.";
    return StatusCode::FAILURE;
  //
  case CONTROLREADY:
    if ( m_states[iAlgo] != INITIAL ) {
      log << MSG::ERROR << "[AlgIndex " << iAlgo
          << "] Transition to CONTROLREADY possible only from INITIAL state! The state is " << m_states[iAlgo]
          << endmsg;
      return StatusCode::FAILURE;
    } else {
      m_states[iAlgo] = CONTROLREADY;
      return StatusCode::SUCCESS;
    }
  //
  case DATAREADY:
    if ( m_states[iAlgo] != CONTROLREADY ) {
      log << MSG::ERROR << "[AlgIndex " << iAlgo
          << "] Transition to DATAREADY possible only from CONTROLREADY state!The state is " << m_states[iAlgo]
          << endmsg;
      return StatusCode::FAILURE;
    } else {
      m_states[iAlgo] = DATAREADY;
      return StatusCode::SUCCESS;
    }
  case SCHEDULED:
    if ( m_states[iAlgo] != DATAREADY ) {
      log << MSG::ERROR << "[AlgIndex " << iAlgo
          << "] Transition to SCHEDULED possible only from DATAREADY state! The state is " << m_states[iAlgo] << endmsg;
      return StatusCode::FAILURE;
    } else {
      m_states[iAlgo] = SCHEDULED;
      return StatusCode::SUCCESS;
    }
  //
  case EVTACCEPTED:
    if ( m_states[iAlgo] != SCHEDULED ) {
      log << MSG::ERROR << "[AlgIndex " << iAlgo
          << "] Transition to EVTACCEPTED possible only from SCHEDULED state! The state is " << m_states[iAlgo]
          << endmsg;
      return StatusCode::FAILURE;
    } else {
      m_states[iAlgo] = EVTACCEPTED;
      return StatusCode::SUCCESS;
    }
  //
  case EVTREJECTED:
    if ( m_states[iAlgo] != SCHEDULED ) {
      log << MSG::ERROR << "[AlgIndex " << iAlgo
          << "] Transition to EVTREJECT possible only from SCHEDULED state! The state is " << m_states[iAlgo] << endmsg;
      return StatusCode::FAILURE;
    } else {
      m_states[iAlgo] = EVTREJECTED;
      return StatusCode::SUCCESS;
    }
  default:
    //
    m_states[iAlgo] = ERROR;
    return StatusCode::SUCCESS;
  }

  return StatusCode::FAILURE;
}

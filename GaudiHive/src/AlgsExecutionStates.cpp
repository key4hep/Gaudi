#include "AlgsExecutionStates.h"

// A simple map to easily translate a state to its name
std::map<AlgsExecutionStates::State,std::string> AlgsExecutionStates::stateNames = {
    {INITIAL,"INITIAL"},
    {CONTROLREADY,"CONTROLREADY"},
    {DATAREADY,"DATAREADY"},
    {SCHEDULED,"SCHEDULED"},
    {EVTACCEPTED,"EVTACCEPTED"},
    {EVTREJECTED,"EVTREJECTED"},
    {ERROR,"ERROR"}
    };

StatusCode AlgsExecutionStates::updateState(unsigned int iAlgo,State newState){
	MsgStream log(m_MS, "AlgExecutionStates");
	const unsigned int states_size = m_states[INITIAL].size(); //all fields have the same size

	if (iAlgo>=states_size)
		log << MSG::ERROR << "Index out of bound ("
		<< iAlgo << " and the size of the states vector is "
		<< states_size << ")" << endmsg;

	switch (newState) {
	case INITIAL:
		log << MSG::ERROR << "[AlgIndex " << iAlgo <<"] Transition to INITIAL is not defined.";
		return StatusCode::FAILURE;
		//
	case CONTROLREADY:
		if (!m_states[INITIAL].test(iAlgo)){
			log << MSG::ERROR << "[AlgIndex " << iAlgo <<"] Transition to CONTROLREADY possible only from INITIAL state! The state is " << m_states[iAlgo] << endmsg;
			return StatusCode::FAILURE;
		} else {
			m_states[INITIAL].reset(iAlgo);
			m_states[CONTROLREADY].set(iAlgo);
			return StatusCode::SUCCESS;
		}
		//
	case DATAREADY:
		if (!m_states[CONTROLREADY].test(iAlgo)){
			log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to DATAREADY possible only from CONTROLREADY state!The state is " << m_states[iAlgo] << endmsg;
			return StatusCode::FAILURE;
		} else {
			m_states[CONTROLREADY].reset(iAlgo);
			m_states[DATAREADY].set(iAlgo);
			return StatusCode::SUCCESS;
		}
	case SCHEDULED:
		if (!m_states[DATAREADY].test(iAlgo)){
			log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to SCHEDULED possible only from DATAREADY state! The state is " << m_states[iAlgo] << endmsg;
			return StatusCode::FAILURE;
		} else {
			m_states[DATAREADY].reset(iAlgo);
			m_states[SCHEDULED].set(iAlgo);
			return StatusCode::SUCCESS;
		}
		//
	case EVTACCEPTED:
		if (!m_states[SCHEDULED].test(iAlgo)){
			log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to EVTACCEPTED possible only from SCHEDULED state! The state is " << m_states[iAlgo] << endmsg;
			return StatusCode::FAILURE;
		} else {
			m_states[SCHEDULED].reset(iAlgo);
			m_states[EVTACCEPTED].set(iAlgo);
			return StatusCode::SUCCESS;
		}
		//
	case EVTREJECTED:
		if (!m_states[SCHEDULED].test(iAlgo)){
			log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to EVTACCEPTED possible only from SCHEDULED state! The state is " << m_states[iAlgo] << endmsg;
			return StatusCode::FAILURE;
		} else {
			m_states[SCHEDULED].reset(iAlgo);
			m_states[EVTREJECTED].set(iAlgo);
			return StatusCode::SUCCESS;
		}
	default:
		//
		log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Undefined state!" << endmsg;
		return StatusCode::FAILURE;
	}
	return StatusCode::FAILURE;
};

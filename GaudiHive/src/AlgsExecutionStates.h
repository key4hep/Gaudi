#ifndef GAUDIHIVE_ALGSEXECUTIONSTATES_H
#define GAUDIHIVE_ALGSEXECUTIONSTATES_H

// Framework include files
#include "GaudiKernel/Service.h" 

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>


//---------------------------------------------------------------------------

/**@class AlgsExecutionStates AlgsExecutionStates.h GaudiKernel/AlgsExecutionStates.h
 *
 *  The AlgsExecutionStates encodes the state machine for the execution of 
 *  algorithms within a single event. It is used by the concurrent schedulers 
 * 
    @author  Benedikt Hegner 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class AlgsExecutionStates{
public:
    
  /// Execution states of the algorithms
  enum State : unsigned short {
    INITIAL,
      CONTROLREADY,
      DATAREADY,
      SCHEDULED,
      EVTACCEPTED,
      EVTREJECTED,
        ERROR
      };
    
  static std::map<State,std::string> stateNames;
    
  AlgsExecutionStates(unsigned int algsNumber, SmartIF<IMessageSvc> MS):
    m_states(algsNumber,INITIAL),
    m_MS(MS){};    
    ~AlgsExecutionStates(){};
    StatusCode updateState(unsigned int iAlgo,State newState){
      MsgStream log(m_MS, "AlgExecutionStates");
      const unsigned int states_size = m_states.size();
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
	if (m_states[iAlgo]!=INITIAL){
	  log << MSG::ERROR << "[AlgIndex " << iAlgo <<"] Transition to CONTROLREADY possible only from INITIAL state! The state is " << m_states[iAlgo] << endmsg;
	  return StatusCode::FAILURE;              
	} else {     
	  m_states[iAlgo]=CONTROLREADY;
	  return StatusCode::SUCCESS; 
	}
        //
      case DATAREADY:
	if (m_states[iAlgo]!=CONTROLREADY){
	  log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to DATAREADY possible only from CONTROLREADY state!The state is " << m_states[iAlgo] << endmsg;
	  return StatusCode::FAILURE;              
	} else {       
	  m_states[iAlgo]=DATAREADY;
	  return StatusCode::SUCCESS; 
	}
      case SCHEDULED:
	if (m_states[iAlgo]!=DATAREADY){
	  log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to SCHEDULED possible only from DATAREADY state! The state is " << m_states[iAlgo] << endmsg;
	  return StatusCode::FAILURE;              
	} else {
	  m_states[iAlgo]=SCHEDULED;
	  return StatusCode::SUCCESS; 
	}           
        //
      case EVTACCEPTED:
	if (m_states[iAlgo]!=SCHEDULED){
	  log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to EVTACCEPTED possible only from SCHEDULED state! The state is " << m_states[iAlgo] << endmsg;
	  return StatusCode::FAILURE;   
	} else {
	  m_states[iAlgo]=EVTACCEPTED;
	  return StatusCode::SUCCESS; 
	} 
        //
      default:
      case EVTREJECTED:
        if (m_states[iAlgo]!=SCHEDULED){
          log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Transition to EVTACCEPTED possible only from SCHEDULED state! The state is " << m_states[iAlgo] << endmsg;
          return StatusCode::FAILURE;
        } else {
          m_states[iAlgo]=EVTREJECTED;
          return StatusCode::SUCCESS;
        }
        //                                
	log << MSG::ERROR  << "[AlgIndex " << iAlgo <<"] Undefined state!" << endmsg;
	return StatusCode::FAILURE;
      }
      return StatusCode::FAILURE;
    };
    
    void reset(){m_states.assign(m_states.size(),INITIAL);};
    bool algsPresent(State state) const{return std::find(m_states.begin(),m_states.end(),state)!=m_states.end();}
    bool allAlgsExecuted(){
      int execAlgos=std::count_if(m_states.begin(),m_states.end(),[](State s) {return (s == EVTACCEPTED || s== EVTREJECTED);});
      return m_states.size() == (unsigned int)execAlgos; };
    inline State algorithmState(unsigned int iAlgo){return iAlgo>=m_states.size()? ERROR : m_states[iAlgo];};

    typedef std::vector<State> states_vector;
    typedef states_vector::iterator vectIt;
    typedef states_vector::const_iterator const_vectIt;
    vectIt begin(){return m_states.begin();}; 
    vectIt end(){return m_states.end();};  
    const_vectIt begin() const{const_vectIt it = m_states.begin();return it;};
    const_vectIt end() const{const_vectIt it = m_states.end();return it;};

    states_vector m_states;    
 private:
    SmartIF<IMessageSvc> m_MS;
    
};

#endif // GAUDIHIVE_ALGSEXECUTIONSTATES_H

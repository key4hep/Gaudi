#include "ControlFlowManager.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  DecisionNode::~DecisionNode(){
    for (auto node : m_daughters) {
      delete node;
    }
  }

  //---------------------------------------------------------------------------
  void DecisionNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    for (auto daughter : m_daughters){ daughter->initialize(algname_index_map); }
  }

  //---------------------------------------------------------------------------
  int DecisionNode::updateState(std::vector<State>& states) const {
    int decision = ((m_allPass) ? 1 : -1);
    bool hasUndecidedChild = false;
    for (auto daughter : m_daughters){
      if (m_isLazy && (-1 !=decision || hasUndecidedChild ) )  {return decision;} // if lazy return once result is known already or we can't fully evaluate right now because one daugther decision is missing still
      auto res = daughter->updateState(states);
      if ( -1 == res) {hasUndecidedChild = true;}
      else if ( false == m_modeOR && res == 0 ){decision = 0;} // "and"-mode (once first result false, the overall decision is false) 
      else if ( true  == m_modeOR && res == 1){decision = 1;} // "or"-mode  (once first result true, the overall decision is true)
    }
    // what to do with yet undefined answers depends on whether AND or OR mode applies 
    if (!hasUndecidedChild &&  -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR){ return 0; }
      // AND mode: all results known, and no false -> accept 
      else { return 1; }
    }
    // in all other cases I stay with previous decisions
    return decision;
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_algoIndex = algname_index_map.at(m_algoName);
  }
  //---------------------------------------------------------------------------
  int AlgorithmNode::updateState( std::vector<State>& states ) const {
    // since we reached this point in the control flow, this algorithm is supposed to run
    // if it hasn't already   
    State& state = states[m_algoIndex];
    if (State::INITIAL == state) {state = State::CONTROLREADY;}
    // now derive the proper result to pass back
    if (true == m_allPass) { 
      return 1; 
    } else if (State::EVTACCEPTED == state) {
      return !m_inverted;
    } else if (State::EVTREJECTED == state) {
      return m_inverted;
    } else {
      return -1; // result not known yet 
    }
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::initialize(ControlFlowNode* node, const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_headNode = node;
    node->initialize(algname_index_map);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateEventState(std::vector<State>& algo_states) const {
    m_headNode->updateState(algo_states);
  };

} // namespace

#include "ControlFlowManager.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  std::string ControlFlowNode::stateToString(const int& stateId) const {
    if (0 == stateId ) return "FALSE";
    else if (1 == stateId ) return "TRUE";
    else return "UNDEFINED";
  }

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
  void DecisionNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map,
                                GraphMap& graph_map){
    graph_map[m_nodeName].push_back(this);

    for (auto daughter : m_daughters) {
      daughter->initialize(algname_index_map, graph_map);
    }
  }

  //---------------------------------------------------------------------------
  void DecisionNode::printState(std::stringstream& output,
                                const std::vector<int>& node_decisions,
                                const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " : "
           << stateToString(node_decisions[m_nodeIndex]) << std::endl;
    for (auto daughter : m_daughters ) {
      daughter->printState(output,node_decisions,recursionLevel+1);
    }
  }

  //---------------------------------------------------------------------------
  int DecisionNode::updateState(std::vector<State>& states,
                                std::vector<int>& node_decisions) const {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    int decision = ((m_allPass && m_isLazy) ? 1 : -1);
    bool hasUndecidedChild = false;
    for (auto daughter : m_daughters){
      if (m_isLazy && (-1 !=decision || hasUndecidedChild ) ) {
        node_decisions[m_nodeIndex] = decision;
        return decision;} // if lazy return once result is known already or we can't fully evaluate right now because one daugther decision is missing still
      auto res = daughter->updateState(states, node_decisions);
      if ( -1 == res) {hasUndecidedChild = true;}
      else if ( false == m_modeOR && res == 0 ){decision = 0;} // "and"-mode (once first result false, the overall decision is false)
      else if ( true  == m_modeOR && res == 1){decision = 1;} // "or"-mode  (once first result true, the overall decision is true)
    }
    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if (!hasUndecidedChild &&  -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR){ decision = 0; }
      // AND mode: all results known, and no false -> accept
      else { decision = 1; }
    }
    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;
    if (m_allPass) decision = 1;
    return decision;
  }

  //---------------------------------------------------------------------------
  void DecisionNode::updateDecision(std::vector<State>& states,
                                    std::vector<int>& node_decisions) const {
    int decision = ((m_allPass && m_isLazy) ? 1 : -1);
    bool hasUndecidedChild = false;

    for (auto daughter : m_daughters){
      // if lazy return once result is known already or we can't fully evaluate
      // right now because one daughter decision is missing still
      if (m_isLazy && (-1 != decision || hasUndecidedChild)) {
        node_decisions[m_nodeIndex] = decision;
        return;
      }

      auto res = node_decisions[daughter->getNodeIndex()];
      if (-1 == res) {
        hasUndecidedChild = true;
        //daughter->promoteToControlReadyState(states, node_decisions);
        State& state = states[daughter->getNodeIndex()];
        if (State::INITIAL == state) state = State::CONTROLREADY;
      // "and"-mode (once first result false, the overall decision is false)
      } else if (false == m_modeOR && res == 0) {
        decision = 0;
      // "or"-mode  (once first result true, the overall decision is true)
      } else if (true  == m_modeOR && res == 1) {
        decision = 1;
      }
    }

    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if (!hasUndecidedChild &&  -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR) {
        decision = 0;
      // AND mode: all results known, and no false -> accept
      } else {
        decision = 1;
      }
    }

    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;

    if (-1 != decision && m_parentNode) {
      DecisionNode* parentNode = dynamic_cast<DecisionNode*>(m_parentNode);
      if (parentNode) {
        parentNode->updateDecision(states, node_decisions);
      } else {
        std::cout << "FATAL: Casting of parent decision node did not succeed!" << std::endl;
      }
    }
  }

  //---------------------------------------------------------------------------
  void DecisionNode::promoteToControlReadyState(std::vector<State>& states,
                                                std::vector<int>& node_decisions) const {
    //std::cout << "REACHED DECISNODE " << m_nodeName << std::endl;
    if (-1 != node_decisions[m_nodeIndex]) {
      return;
    }

    for (auto daughter : m_daughters ) {
      auto res = node_decisions[daughter->getNodeIndex()];
      if (-1 == res) {
        daughter->promoteToControlReadyState(states, node_decisions);
        if (m_isLazy) return;
      } else if (m_isLazy) {
        if ((false == m_modeOR && res == 0) || (true == m_modeOR && res == 1)) return;
      }
    }
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_algoIndex = algname_index_map.at(m_algoName);
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map,
                                 GraphMap& graph_map){
    m_algoIndex = algname_index_map.at(m_algoName);
    graph_map[m_algoName].push_back(this);
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::promoteToControlReadyState(std::vector<State>& states,
                                                 std::vector<int>& node_decisions) const {

    //std::cout << "REACHED ALGONODE " << m_algoName << std::endl;
    State& state = states[m_algoIndex];
    if (State::INITIAL == state) state = State::CONTROLREADY;
  }


  //---------------------------------------------------------------------------
  void AlgorithmNode::printState(std::stringstream& output,
                                 const std::vector<int>& node_decisions,
                                 const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " : "
           << stateToString(node_decisions[m_nodeIndex]) << std::endl;
  }

  //---------------------------------------------------------------------------
  int AlgorithmNode::updateState(std::vector<State>& states,
                                 std::vector<int>& node_decisions) const {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    // since we reached this point in the control flow, this algorithm is supposed to run
    // if it hasn't already
    State& state = states[m_algoIndex];
    unsigned int decision = -1;
    if (State::INITIAL == state) {state = State::CONTROLREADY;}
    // now derive the proper result to pass back
    if (true == m_allPass) {
      decision = 1;
    } else if (State::EVTACCEPTED == state) {
      decision = !m_inverted;
    } else if (State::EVTREJECTED == state) {
      decision = m_inverted;
    } else {
      decision =  -1; // result not known yet
    }
    node_decisions[m_nodeIndex] = decision;
    return decision;
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::updateDecision(std::vector<State>& states,
                                     std::vector<int>& node_decisions) const {

    State& state = states[m_algoIndex];
    int decision = -1;
    //if (State::INITIAL == state) {state = State::CONTROLREADY;}

    // now derive the proper result to pass back
    if (true == m_allPass) {
      decision = 1;
    } else if (State::EVTACCEPTED == state) {
      decision = !m_inverted;
    } else if (State::EVTREJECTED == state) {
      decision = m_inverted;
    } else {
      decision =  -1; // result not known yet
    }
    node_decisions[m_nodeIndex] = decision;
    DecisionNode* parentNode = dynamic_cast<DecisionNode*>(m_parentNode);
    if (parentNode != 0) {
      //parentNode->updateDecision(m_nodeIndex, decision, node_decisions);
      parentNode->updateDecision(states, node_decisions);
    } else {
      std::cout << "Null pointer on type-cast for parent of node [" << m_nodeName << "]" << endmsg;
    }
    //parentNode->updateDecision(states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::initialize(ControlFlowNode* node,
                                      const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_headNode = node;
    node->initialize(algname_index_map, m_graphMap);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateEventState(std::vector<State>& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_headNode->updateState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::promoteToControlReadyState(std::vector<State>& algo_states,
                                                      std::vector<int>& node_decisions) const {
    m_headNode->promoteToControlReadyState(algo_states, node_decisions);
  }

} // namespace

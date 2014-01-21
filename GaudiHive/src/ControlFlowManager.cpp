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
  void DecisionNode::printState(std::stringstream& output,
                                std::vector<State>& states,
                                const std::vector<int>& node_decisions,
                                const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " (" << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString(node_decisions[m_nodeIndex]) << "(" << node_decisions[m_nodeIndex] <<")" << std::endl;
    for (auto daughter : m_daughters ) {
      daughter->printState(output,states,node_decisions,recursionLevel+2);
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
      // modified
      int& res = node_decisions[daughter->getNodeIndex()];
      if (-1 == res) {
        hasUndecidedChild = true;
        //daughter->promoteToControlReadyState(states, node_decisions);
        if (typeid(*daughter) != typeid(concurrency::DecisionNode)) {
          AlgorithmNode* algod = (AlgorithmNode*) daughter;
          State& state = states[algod->getAlgoIndex()];
          if (State::INITIAL == state) {
              state = State::CONTROLREADY;
          }
        } else {
          daughter->updateDecision(states, node_decisions);
        }

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

    // propagate decision upwards through the decision graph
    if (-1 != decision)
      for (auto p : m_parents)
          p->updateDecision(states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void DecisionNode::promoteToControlReadyState(std::vector<State>& states,
                                                std::vector<int>& node_decisions) const {

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
  void AlgorithmNode::promoteToControlReadyState(std::vector<State>& states,
                                                 std::vector<int>& node_decisions) const {

    State& state = states[m_algoIndex];
    if (State::INITIAL == state) state = State::CONTROLREADY;
  }


  //---------------------------------------------------------------------------
  void AlgorithmNode::printState(std::stringstream& output,
                                 std::vector<State>& states,
                                 const std::vector<int>& node_decisions,
                                 const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " (" << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString(node_decisions[m_nodeIndex]) << "(" << node_decisions[m_nodeIndex] << ")"
           << ", in state: " << states[m_algoIndex] << std::endl;
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

    for (auto p : m_parents)
      p->updateDecision(states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_headNode->initialize(algname_index_map);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addAlgorithmNode(const std::string& algoName, const std::string& parentName, bool inverted, bool allPass) {

    auto itP = m_graphAggMap.find(parentName);
    concurrency::DecisionNode* parentNode = itP->second;

    auto itA = m_graphAlgoMap.find(algoName);
    concurrency::AlgorithmNode* algoNode;
    if ( itA != m_graphAlgoMap.end()) {
      algoNode = itA->second;
    } else {
      algoNode = new concurrency::AlgorithmNode(m_nodeCounter,algoName,inverted,allPass);
      ++m_nodeCounter;
      m_graphAlgoMap[algoName] = algoNode;
      //std::cout << "CF: AlgoNode " << algoName << " at: " << algoNode << std::endl;
    }

    parentNode->addDaughterNode(algoNode);
    algoNode->addParentNode(parentNode);

  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addAggregateNode(const std::string& aggregateName, const std::string& parentName, bool modeOR, bool allPass, bool isLazy) {

    auto itP = m_graphAggMap.find(parentName);
    concurrency::DecisionNode* parentNode = itP->second;

    auto itA = m_graphAggMap.find(aggregateName);
    concurrency::DecisionNode* aggregateNode;
    if ( itA != m_graphAggMap.end()) {
      aggregateNode = itA->second;
    } else {
      aggregateNode = new concurrency::DecisionNode(m_nodeCounter,aggregateName,modeOR,allPass,isLazy);
      ++m_nodeCounter;
      m_graphAggMap[aggregateName] = aggregateNode;
      //std::cout << "CF: AggregateNode " << aggregateName << " at: " << aggregateNode << std::endl;
    }

    parentNode->addDaughterNode(aggregateNode);
    aggregateNode->addParentNode(parentNode);

  }

  ///
  void ControlFlowGraph::addHeadNode(const std::string& headName, bool modeOR, bool allPass, bool isLazy) {

    auto itH = m_graphAggMap.find(headName);
    if ( itH != m_graphAggMap.end()) {
      m_headNode = itH->second;
    } else {
      m_headNode = new concurrency::DecisionNode(m_nodeCounter, headName, modeOR, allPass, isLazy);
      ++m_nodeCounter;
      m_graphAggMap[headName] = m_headNode;
    }

  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::updateEventState(std::vector<State>& algo_states,
                                          std::vector<int>& node_decisions) const {
    m_headNode->updateState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::updateDecision(const std::string& algo_name,
                                        std::vector<State>& algo_states,
                                        std::vector<int>& node_decisions) const {
    m_graphAlgoMap.at(algo_name)->updateDecision(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::promoteToControlReadyState(std::vector<State>& algo_states,
                                                    std::vector<int>& node_decisions) const {
    m_headNode->promoteToControlReadyState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::initialize(ControlFlowGraph* cf_graph,
                                      const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_CFGraph = cf_graph;
    cf_graph->initialize(algname_index_map);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateEventState(std::vector<State>& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_CFGraph->updateEventState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateDecision(const std::string& algo_name,
                                          std::vector<State>& algo_states,
                                          std::vector<int>& node_decisions) const {
    m_CFGraph->updateDecision(algo_name, algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::promoteToControlReadyState(std::vector<State>& algo_states,
                                                      std::vector<int>& node_decisions) const {
    m_CFGraph->promoteToControlReadyState(algo_states, node_decisions);
  }

} // namespace

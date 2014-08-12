#include "ExecutionFlowManager.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  StatusCode ExecutionFlowManager::initialize(ControlFlowGraph* cf_graph,
                                            const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_CFGraph = cf_graph;
    StatusCode sc = cf_graph->initialize(algname_index_map);
    if (!sc.isSuccess())
      error() << "Could not initialize the flow graph." << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  StatusCode ExecutionFlowManager::initialize(ControlFlowGraph* cf_graph,
                                            const std::unordered_map<std::string,unsigned int>& algname_index_map,
                                            std::vector<EventSlot>& eventSlots){
    m_CFGraph = cf_graph;
    StatusCode sc = cf_graph->initialize(algname_index_map, eventSlots);
    if (!sc.isSuccess())
      error() << "Could not initialize the flow graph." << endmsg;

    auto ranker = concurrency::AlgorithmOnDataOutputRanker();
    m_CFGraph->rankAlgorithmsByDataOutput(ranker);

    return sc;
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::simulateExecutionFlow(IGraphVisitor& visitor) const {

    std::vector<int>& nodeDecisions = m_CFGraph->getNodeDecisions(0);

    std::vector<int> fixedNodeDecisions;
    int cntr = 0;
    std::vector<int> counters;
    while (!rootDecisionResolved(nodeDecisions)) {
      cntr += 1;
      int prevAlgosNum = visitor.m_nodesSucceeded;
      debug() << "  Proceeding with iteration #" << cntr << endmsg;
      fixedNodeDecisions = m_CFGraph->getNodeDecisions(0);
      m_CFGraph->m_headNode->accept(visitor);
      if ( fixedNodeDecisions == nodeDecisions) {
        error() << "  No progress on iteration " << cntr << " detected" << endmsg;
        debug() << nodeDecisions << endmsg;
        break;
      }
      info() << "   Iteration #" << cntr << " finished, total algorithms executed: " << visitor.m_nodesSucceeded << endmsg;

      std::stringstream s;
      s << cntr << ", " << (visitor.m_nodesSucceeded-prevAlgosNum) << "\n";

      std::ofstream myfile;
      myfile.open("RunSimulation.csv", std::ios::app);
      myfile << s.str();
      myfile.close();


      if (visitor.m_nodesSucceeded != prevAlgosNum)
        counters.push_back(visitor.m_nodesSucceeded);
    }

    info() << "Asymptotical concurrency speedup depth: " << (float) visitor.m_nodesSucceeded / (float) counters.size() << endmsg;

    // Reset algorithm states and node decisions
    m_CFGraph->getAlgoStates(0).reset();
    nodeDecisions.assign(nodeDecisions.size(),-1);

  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::updateEventState(AlgsExecutionStates& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_CFGraph->updateEventState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::updateDecision(const std::string& algo_name,
                                          const int& slotNum,
                                          AlgsExecutionStates& algo_states,
                                          std::vector<int>& node_decisions) const {
    m_CFGraph->updateDecision(algo_name, slotNum, algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::promoteToControlReadyState(AlgsExecutionStates& algo_states,
                                                      std::vector<int>& node_decisions,
                                                      const int& slotNum) const {
    m_CFGraph->m_headNode->promoteToControlReadyState(slotNum, algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  bool ExecutionFlowManager::algoDataDependenciesSatisfied(const std::string& algo_name, const int& slotNum) const {
    return m_CFGraph->getAlgorithmNode(algo_name)->dataDependenciesSatisfied(slotNum);
  }

  //---------------------------------------------------------------------------
  bool ExecutionFlowManager::rootDecisionResolved(const std::vector<int>& node_decisions) const {

    return (-1 != node_decisions[m_CFGraph->m_headNode->getNodeIndex()]) ? true : false;
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::touchReadyAlgorithms(IGraphVisitor& visitor) const {

    auto& states = m_CFGraph->getAlgoStates(visitor.m_slotNum);
    auto& decisions = m_CFGraph->getNodeDecisions(visitor.m_slotNum);

    //m_CFGraph->m_headNode->promoteToControlReadyState(slotNum,states,decisions);

    m_CFGraph->m_headNode->accept(visitor);

  }
}

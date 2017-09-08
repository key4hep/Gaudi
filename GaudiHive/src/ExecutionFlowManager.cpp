#include "ExecutionFlowManager.h"

namespace concurrency {

 namespace recursive_CF {

  //---------------------------------------------------------------------------
  StatusCode ExecutionFlowManager::initialize(ControlFlowGraph* graph,
                                              const std::unordered_map<std::string,unsigned int>& algname_index_map) {
    m_CFGraph = graph;
    StatusCode sc = graph->initialize(algname_index_map);
    if (!sc.isSuccess())
      error() << "Could not initialize the flow graph." << endmsg;

    return sc;
  }

  void ExecutionFlowManager::updateEventState(AlgsExecutionStates& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_CFGraph->updateEventState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  bool ExecutionFlowManager::rootDecisionResolved(const std::vector<int>& node_decisions) const {

    return (-1 != node_decisions[m_CFGraph->getHeadNode()->getNodeIndex()]) ? true : false;
  }
 }
}

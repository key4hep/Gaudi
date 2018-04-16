#include "ExecutionFlowManager.h"

namespace concurrency
{

  namespace recursive_CF
  {

    //---------------------------------------------------------------------------
    void ExecutionFlowManager::initialize( ControlFlowGraph* graph,
                                           const std::unordered_map<std::string, unsigned int>& algname_index_map )
    {
      m_CFGraph = graph;
      graph->initialize( algname_index_map );
    }

    void ExecutionFlowManager::updateEventState( AlgsExecutionStates& algo_states,
                                                 std::vector<int>&    node_decisions ) const
    {
      m_CFGraph->updateEventState( algo_states, node_decisions );
    }

    //---------------------------------------------------------------------------
    bool ExecutionFlowManager::rootDecisionResolved( const std::vector<int>& node_decisions ) const
    {

      return ( -1 != node_decisions[m_CFGraph->getHeadNode()->getNodeIndex()] ) ? true : false;
    }
  }
}

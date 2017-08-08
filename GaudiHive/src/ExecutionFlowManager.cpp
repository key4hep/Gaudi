#include "ExecutionFlowManager.h"
#include "PRGraphVisitors.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  StatusCode ExecutionFlowManager::initialize(PrecedenceRulesGraph* graph,
                                            const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_PRGraph = graph;
    StatusCode sc = graph->initialize(algname_index_map);
    if (!sc.isSuccess())
      error() << "Could not initialize the flow graph." << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  StatusCode ExecutionFlowManager::initialize(PrecedenceRulesGraph* graph,
                                            const std::unordered_map<std::string,unsigned int>& algname_index_map,
                                            const std::string& mode,
                                            bool enableCondSvc){
    m_PRGraph = graph;
    StatusCode sc = graph->initialize(algname_index_map, enableCondSvc);

    if (!sc.isSuccess()) {
      error() << "Could not initialize the execution flow graph." << endmsg;
      return sc;
    }

    // Rank algorithms if any known optimization mode is supplied
    if (mode == "PCE") {
      auto ranker = concurrency::RankerByProductConsumption();
      m_PRGraph->rankAlgorithms(ranker);
    } else if (mode == "COD") {
      auto ranker = concurrency::RankerByCummulativeOutDegree();
      m_PRGraph->rankAlgorithms(ranker);
    } else if (mode == "E") {
      auto ranker = concurrency::RankerByEccentricity();
      m_PRGraph->rankAlgorithms(ranker);
    } else if (mode == "T") {
      auto ranker = concurrency::RankerByTiming();
      m_PRGraph->rankAlgorithms(ranker);
    } else if (mode == "DRE") {
      auto ranker = concurrency::RankerByDataRealmEccentricity();
      m_PRGraph->rankAlgorithms(ranker);
    } else if (!mode.empty()){
      error() << "Requested optimization mode '" << mode << "' is not known." << endmsg;
      sc = StatusCode::FAILURE;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::simulateExecutionFlow(IGraphVisitor& visitor) const {

    std::vector<int>& nodeDecisions = visitor.m_slot->controlFlowState;

    std::vector<int> fixedNodeDecisions;
    int cntr = 0;
    std::vector<int> counters;
    while (!rootDecisionResolved(nodeDecisions)) {
      cntr += 1;
      int prevAlgosNum = visitor.m_nodesSucceeded;
      debug() << "  Proceeding with iteration #" << cntr << endmsg;
      fixedNodeDecisions = visitor.m_slot->controlFlowState;
      m_PRGraph->m_headNode->accept(visitor);
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
    visitor.m_slot->algsStates.reset();
    nodeDecisions.assign(nodeDecisions.size(),-1);

  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::updateEventState(AlgsExecutionStates& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_PRGraph->updateEventState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::updateDecision(const std::string& algo_name,
                                            IGraphVisitor& visitor) const {
    m_PRGraph->updateDecision(algo_name, visitor);
  }

  //---------------------------------------------------------------------------
  bool ExecutionFlowManager::rootDecisionResolved(const std::vector<int>& node_decisions) const {

    return (-1 != node_decisions[m_PRGraph->m_headNode->getNodeIndex()]) ? true : false;
  }

  //---------------------------------------------------------------------------
  void ExecutionFlowManager::touchReadyAlgorithms(IGraphVisitor& visitor) const {

    m_PRGraph->m_headNode->accept(visitor);

  }
}

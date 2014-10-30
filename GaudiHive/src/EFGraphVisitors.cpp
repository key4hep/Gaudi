#include "EFGraphVisitors.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter(DecisionNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visit(DecisionNode& node) {

    //std::cout << "1-st level Decision: " << node.getNodeName() << std::endl;
    bool allChildDecisionsResolved = true;
    for (auto child : node.getDaughters()) {
      int& childDecision = child->m_graph->getNodeDecisions(m_slotNum)[child->getNodeIndex()];

      if (childDecision == 1 && node.m_modeOR && node.m_isLazy) {
        node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;
        return true;
      }

      if (childDecision == -1) {
        allChildDecisionsResolved = false;
      }
    }

    if (allChildDecisionsResolved)
      node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;

    return allChildDecisionsResolved;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visitLeave(DecisionNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }



  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter(AlgorithmNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }

  //--------------------------------------------------------------------------
  bool RunSimulator::visit(AlgorithmNode& node) {

    std::vector<int>& decisions = node.m_graph->getNodeDecisions(m_slotNum);
    AlgsExecutionStates& states = node.m_graph->getAlgoStates(m_slotNum);
    int& decision = decisions[node.getNodeIndex()];

    if (State::INITIAL == states[node.getAlgoIndex()]) {
      states.updateState(node.getAlgoIndex(), State::CONTROLREADY);
      if (node.dataDependenciesSatisfied(m_slotNum)) {
        states.updateState(node.getAlgoIndex(), State::DATAREADY);
        states.updateState(node.getAlgoIndex(), State::SCHEDULED);
        states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
        decision = 1;
        ++m_nodesSucceeded;
        //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
        return true;
      }
    } else if (State::CONTROLREADY == states[node.getAlgoIndex()] && node.dataDependenciesSatisfied(m_slotNum)) {
      states.updateState(node.getAlgoIndex(), State::DATAREADY);
      states.updateState(node.getAlgoIndex(), State::SCHEDULED);
      states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
      decision = 1;
      ++m_nodesSucceeded;
      //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
      return true;
    }

    return false;
  }



  //---------------------------------------------------------------------------
    bool Trigger::visitEnter(DecisionNode& node) const {

      if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
        return true;
      return false;
    }

    //---------------------------------------------------------------------------
    bool Trigger::visit(DecisionNode& node) {

      //std::cout << "1-st level Decision: " << node.getNodeName() << std::endl;
      bool allChildDecisionsResolved = true;
      for (auto child : node.getDaughters()) {
        int& childDecision = child->m_graph->getNodeDecisions(m_slotNum)[child->getNodeIndex()];

        if (childDecision == 1 && node.m_modeOR && node.m_isLazy) {
          node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;
          return true;
        }

        if (childDecision == -1) {
          allChildDecisionsResolved = false;
        }
      }

      if (allChildDecisionsResolved)
        node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;

      return allChildDecisionsResolved;
    }

    //---------------------------------------------------------------------------
    bool Trigger::visitLeave(DecisionNode& node) const {

      if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
        return true;
      return false;
    }



    //---------------------------------------------------------------------------
    bool Trigger::visitEnter(AlgorithmNode& node) const {

      if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
        return true;
      return false;
    }

    //--------------------------------------------------------------------------
    bool Trigger::visit(AlgorithmNode& node) {

      bool result = false;

      auto& decisions = node.m_graph->getNodeDecisions(m_slotNum);
      auto& states = node.m_graph->getAlgoStates(m_slotNum);

      if (node.promoteToControlReadyState(m_slotNum,states,decisions))
        result = node.promoteToDataReadyState(m_slotNum);

      return result;
    }


    //--------------------------------------------------------------------------
    bool RankerByProductConsumption::visit(AlgorithmNode& node) {

      auto& products = node.getOutputDataNodes();
      float rank = 0;

      for (auto p : products)
        rank += p->getConsumers().size();

      node.setRank(rank);
      /*std::stringstream s;
      s << node.getNodeName() << ", " << rank << "\n";
      std::ofstream myfile;
      myfile.open("AlgoRank.csv", std::ios::app);
      myfile << s.str();
      myfile.close();*/

      return true;
    }

    //--------------------------------------------------------------------------
    bool RankerByExecutionBranchPotential::visit(AlgorithmNode& node) {

      std::ifstream myfile;
      myfile.open("InputExecutionPlan.graphml", std::ios::in);

      boost::ExecPlan execPlan;

      boost::dynamic_properties dp;
      dp.property("name", boost::get(&boost::AlgoNodeStruct::m_name, execPlan));
      dp.property("index", boost::get(&boost::AlgoNodeStruct::m_index, execPlan));
      dp.property("dataRank", boost::get(&boost::AlgoNodeStruct::m_rank, execPlan));
      dp.property("runtime", boost::get(&boost::AlgoNodeStruct::m_runtime, execPlan));

      boost::read_graphml(myfile, execPlan, dp);

      typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
      std::pair<itV, itV> vp;
      typedef boost::graph_traits<boost::ExecPlan>::vertex_descriptor AlgoVertex;

      for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
        AlgoVertex v = *vp.first;
        auto index = boost::get(&boost::AlgoNodeStruct::m_name, execPlan);
        if (index[v] == node.getNodeName()) {
          runThroughAdjacents(v,execPlan);
          float rank = m_nodesSucceeded;
          node.setRank(rank);
          reset();
          //std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
        }
      }

      return true;
    }

    //--------------------------------------------------------------------------
    void RankerByExecutionBranchPotential::runThroughAdjacents(boost::graph_traits<boost::ExecPlan>::vertex_descriptor vertex,
                                                               boost::ExecPlan graph) {
      typename boost::graph_traits<boost::ExecPlan>::adjacency_iterator itVB;
      typename boost::graph_traits<boost::ExecPlan>::adjacency_iterator itVE;

      for (boost::tie(itVB, itVE) = adjacent_vertices(vertex, graph); itVB != itVE; ++itVB) {
        m_nodesSucceeded += 1;
        runThroughAdjacents(*itVB, graph);
      }

    }

    //--------------------------------------------------------------------------
    bool RankerByTiming::visit(AlgorithmNode& node) {

      std::ifstream myfile;
      myfile.open("InputExecutionPlan.graphml", std::ios::in);

      boost::ExecPlan execPlan;

      boost::dynamic_properties dp;
      dp.property("name", boost::get(&boost::AlgoNodeStruct::m_name, execPlan));
      dp.property("index", boost::get(&boost::AlgoNodeStruct::m_index, execPlan));
      dp.property("dataRank", boost::get(&boost::AlgoNodeStruct::m_rank, execPlan));
      dp.property("runtime", boost::get(&boost::AlgoNodeStruct::m_runtime, execPlan));

      boost::read_graphml(myfile, execPlan, dp);

      typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
      std::pair<itV, itV> vp;
      typedef boost::graph_traits<boost::ExecPlan>::vertex_descriptor AlgoVertex;

      for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
        AlgoVertex v = *vp.first;
        auto index = boost::get(&boost::AlgoNodeStruct::m_name, execPlan);
        if (index[v] == node.getNodeName()) {
          auto index_runtime = boost::get(&boost::AlgoNodeStruct::m_runtime, execPlan);
          float rank = index_runtime[v];
          node.setRank(rank);
          //std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
        }
      }
      return true;
    }

    //--------------------------------------------------------------------------
    bool RankerByEccentricity::visit(AlgorithmNode& node) {

      std::ifstream myfile;
      myfile.open("Eccentricity.graphml", std::ios::in);

      boost::ExecPlan execPlan;

      boost::dynamic_properties dp;
      dp.property("name", boost::get(&boost::AlgoNodeStruct::m_name, execPlan));
      dp.property("Eccentricity", boost::get(&boost::AlgoNodeStruct::m_eccentricity, execPlan));

      boost::read_graphml(myfile, execPlan, dp);

      typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
      std::pair<itV, itV> vp;
      typedef boost::graph_traits<boost::ExecPlan>::vertex_descriptor AlgoVertex;

      for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
        AlgoVertex v = *vp.first;
        auto index = boost::get(&boost::AlgoNodeStruct::m_name, execPlan);
        if (index[v] == node.getNodeName()) {
          auto index_eccentricity = boost::get(&boost::AlgoNodeStruct::m_eccentricity, execPlan);
          float rank = index_eccentricity[v];
          node.setRank(rank);
          //std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
        }
      }
      return true;
    }
}

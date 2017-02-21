#include "PRGraphVisitors.h"

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

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != -1)
      return false;
    return true;
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
  bool Trigger::visitEnter(AlgorithmNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != -1)
      return false;
    return true;
  }

  //--------------------------------------------------------------------------
  bool Trigger::visit(AlgorithmNode& node) {

    bool result = false;

    auto& decisions = node.m_graph->getNodeDecisions(m_slotNum);
    auto& states = node.m_graph->getAlgoStates(m_slotNum);

    // Try to shift an algorithm with I->CF, and then, if successful, with CF->DF
    if (node.promoteToControlReadyState(m_slotNum,states,decisions))
      result = node.promoteToDataReadyState(m_slotNum);

    //returns true only when an algorithm is DF-ready
    // i.e., the visitor reached its final goal with the algorithm
    return result;
  }


  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter(DecisionNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != -1)
      return false;
    return true;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visit(DecisionNode& node) {

    bool foundNonResolvedChild = false;
    bool foundNegativeChild = false;
    bool foundPositiveChild = false;
    int decision = -1;

    for (auto child : node.getDaughters()) {
      int& childDecision = child->m_graph->getNodeDecisions(m_slotNum)[child->getNodeIndex()];

      if (childDecision == -1)
        foundNonResolvedChild = true;
      else if (childDecision == 1)
        foundPositiveChild = true;
      else
        foundNegativeChild = true;

      if (node.m_isLazy) {
        if (node.m_modeOR && foundPositiveChild) {
          decision = 1;
          break;
        } else if (!node.m_modeOR && foundNegativeChild) {
          decision = 0;
          break;
        }
      } else {
        if (foundNonResolvedChild)
          break;
      }
    } // end monitoring children

    if (!foundNonResolvedChild && decision == -1) {
      if (node.m_modeOR) // OR
        if (foundPositiveChild) decision = 1;
        else decision = 0;
      else // AND
        if (foundNegativeChild) decision = 0;
        else decision = 1;
    }

    if (node.m_allPass && !foundNonResolvedChild)
      decision = 1;

    if (decision != -1) {
      node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = decision;
      return true;
    }
    return false;
  }



  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter(AlgorithmNode& node) const {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != -1)
      return false;
    return true;
  }

  //--------------------------------------------------------------------------
  bool Supervisor::visit(AlgorithmNode& node) {

    bool result = false;

    auto& decisions = node.m_graph->getNodeDecisions(m_slotNum);
    auto& states = node.m_graph->getAlgoStates(m_slotNum);

    // Try to shift an algorithm with I->CF, and then, if successful, with CF->DF
    if (node.promoteToControlReadyState(m_slotNum,states,decisions))
      result = node.promoteToDataReadyState(m_slotNum);

    //returns true only when an algorithm is DF-ready
    // i.e., the visitor reached its final goal with the algorithm
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
    bool RankerByCummulativeOutDegree::visit(AlgorithmNode& node) {

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
    void RankerByCummulativeOutDegree::runThroughAdjacents(boost::graph_traits<boost::ExecPlan>::vertex_descriptor vertex,
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

    //--------------------------------------------------------------------------
    bool RankerByDataRealmEccentricity::visit(AlgorithmNode& node) {

      // Find eccentricity of the node (only within the data realm of the execution flow graph)
      recursiveVisit(node);

      float rank = m_maxKnownDepth;
      node.setRank(rank);

      // Reset visitor for next nodes, if any
      reset();

      return true;
    }

    //--------------------------------------------------------------------------
    void RankerByDataRealmEccentricity::recursiveVisit(AlgorithmNode& node) {

      m_currentDepth += 1;

      auto& products = node.getOutputDataNodes();

      if (products.empty())
        if ( (m_currentDepth - 1) > m_maxKnownDepth)
          m_maxKnownDepth = m_currentDepth - 1;

      for (auto p : products)
        for ( auto algoNode : p->getConsumers())
          recursiveVisit(*algoNode);

      m_currentDepth -= 1;

    }
}

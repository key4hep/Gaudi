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
    bool AlgorithmOnDataOutputRanker::visit(AlgorithmNode& node) {

      auto& products = node.getOutputDataNodes();
      uint rank = 0;

      for (auto p : products)
        rank += p->getConsumers().size();

      node.setOutputDataRank(rank);
      /*std::stringstream s;
      s << node.getNodeName() << ", " << rank << "\n";
      std::ofstream myfile;
      myfile.open("AlgoRank.csv", std::ios::app);
      myfile << s.str();
      myfile.close();*/

      return true;
    }
}





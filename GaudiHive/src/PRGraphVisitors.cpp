#include "PRGraphVisitors.h"
#include "AlgsExecutionStates.h"

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/ICondSvc.h"

namespace concurrency {

  typedef AlgsExecutionStates::State State;

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter(AlgorithmNode& node) const {

    if (State::CONTROLREADY != m_slot->algsStates[node.getAlgoIndex()])
      return false;

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit(AlgorithmNode& node) {

    bool result = true; // return true if this algorithm has no data inputs

    for ( auto dataNode : node.getInputDataNodes() ) {

      result = dataNode->accept(*this);

      // With ConditionNodes, one may decide NOT to break here so that associated
      // ConditionAlgorithms are scheduled ASAP. This behavior can be made configurable
      if (!result) break; // skip checking other inputs if this input was not produced yet
    }

    if (result) {
      m_slot->algsStates.updateState( node.getAlgoIndex(), State::DATAREADY ).ignore();

      if (m_trace) {
        auto sourceNode = (m_cause.m_source == Cause::source::Task) ?
                node.m_graph->getAlgorithmNode(m_cause.m_sourceName) : nullptr;
        node.m_graph->addEdgeToExecutionPlan(sourceNode, &node);
      }
    }

    // return true only if an algorithm is promoted to DR
    return result;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter(DataNode&) const {
    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit(DataNode& node) {
    /* Implements 'observer' strategy, i.e., only check if producer of this DataNode
     * has been already executed or not */

    bool result = false; // return false if this DataNode has no producers at all

    for ( auto algoNode : node.getProducers() ) {
      const auto& state = m_slot->algsStates[algoNode->getAlgoIndex()];
      if ( State::EVTACCEPTED == state || State::EVTREJECTED == state ) {
        result = true;
        break; // skip checking other producers if one was found to be executed
      }
    }

    // return true only if this DataNode is produced
    return result;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter(ConditionNode& node) const {

    if (node.m_condSvc->isValidID(*(m_slot->eventContext), node.getPath()))
      return false; // do not enter this ConditionNode if the condition has bee already loaded

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit(ConditionNode& node) {
    /* Implements 'requester' strategy, i.e., requests this ConditionNode to be loaded
     * by its associated ConditionAlgorithm */

    auto promoter = Supervisor(*m_slot, m_cause);

    for (auto condAlg : node.getProducers())
      condAlg->accept(promoter);

    // this method is called if, and only if, this ConditionNode is not yet produced.
    // thus, by definition, this ConditionNode is not yet available at this moment
    return false;
  }


  //--------------------------------------------------------------------------
  bool DecisionUpdater::visit(AlgorithmNode& node) {

    auto& states = m_slot->algsStates;
    const State& state = states[node.getAlgoIndex()];
    int decision = -1;

    if ( true == node.isOptimist() )
      decision = 1;
    else if ( State::EVTACCEPTED == state )
      decision = !node.isLiar();
    else if ( State::EVTREJECTED == state )
      decision = node.isLiar();

    if ( -1 != decision ) {

      m_slot->controlFlowState[node.getNodeIndex()] = decision;

      auto promoter = DataReadyPromoter(*m_slot, m_cause, m_trace);
      for ( auto consumer : node.getConsumerNodes() )
        consumer->accept(promoter);

      auto vis = concurrency::Supervisor(*m_slot, m_cause, m_trace);
      for ( auto p : node.getParentDecisionHubs() )
        p->accept(vis);

      return true; // return true only if the algorithm produced a decision
    }

    return false;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter(DecisionNode& node) const {

    if (m_slot->controlFlowState[node.getNodeIndex()] != -1)
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
      int& childDecision = m_slot->controlFlowState[child->getNodeIndex()];

      if (childDecision == -1)
        foundNonResolvedChild = true;
      else if (childDecision == 1)
        foundPositiveChild = true;
      else
        foundNegativeChild = true;

      if (node.m_modePromptDecision) {
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
      if (node.m_modeOR) { // OR
        if (foundPositiveChild) decision = 1;
        else decision = 0;
      } else { // AND
        if (foundNegativeChild) decision = 0;
        else decision = 1;
      }
    }

    if (node.m_allPass && !foundNonResolvedChild)
      decision = 1;

    if (decision != -1) {
      m_slot->controlFlowState[node.getNodeIndex()] = decision;
      return true;
    }
    return false;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter(AlgorithmNode& node) const {

    if (m_slot->controlFlowState[node.getNodeIndex()] != -1)
      return false;
    return true;
  }

  //--------------------------------------------------------------------------
  bool Supervisor::visit(AlgorithmNode& node) {

    bool result = false;

    auto& states = m_slot->algsStates;
    auto& state = states[node.getAlgoIndex()];

    // Promote with INITIAL->CR
    if ( State::INITIAL == state )
      states.updateState( node.getAlgoIndex(), State::CONTROLREADY ).ignore();

    // Try to promote with CR->DR
    if ( State::CONTROLREADY == state ) {
      auto promoter = DataReadyPromoter(*m_slot, m_cause, m_trace);
      result = promoter.visit(node);
    } else {
      result = true;
    }

    // return true only when an algorithm is not lower than DR in its FSM
    // i.e., the visitor has done everything it could with this algorithm
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
    dp.property("name", boost::get(&boost::AlgoProps::m_name, execPlan));
    dp.property("index", boost::get(&boost::AlgoProps::m_index, execPlan));
    dp.property("dataRank", boost::get(&boost::AlgoProps::m_rank, execPlan));
    dp.property("runtime", boost::get(&boost::AlgoProps::m_runtime, execPlan));

    boost::read_graphml(myfile, execPlan, dp);

    typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
      boost::AlgoVertex v = *vp.first;
      auto index = boost::get(&boost::AlgoProps::m_name, execPlan);
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
    dp.property("name", boost::get(&boost::AlgoProps::m_name, execPlan));
    dp.property("index", boost::get(&boost::AlgoProps::m_index, execPlan));
    dp.property("dataRank", boost::get(&boost::AlgoProps::m_rank, execPlan));
    dp.property("runtime", boost::get(&boost::AlgoProps::m_runtime, execPlan));

    boost::read_graphml(myfile, execPlan, dp);

    typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
      boost::AlgoVertex v = *vp.first;
      auto index = boost::get(&boost::AlgoProps::m_name, execPlan);
      if (index[v] == node.getNodeName()) {
        auto index_runtime = boost::get(&boost::AlgoProps::m_runtime, execPlan);
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
    dp.property("name", boost::get(&boost::AlgoProps::m_name, execPlan));
    dp.property("Eccentricity", boost::get(&boost::AlgoProps::m_eccentricity, execPlan));

    boost::read_graphml(myfile, execPlan, dp);

    typedef boost::graph_traits<boost::ExecPlan>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for (vp = boost::vertices(execPlan); vp.first != vp.second; ++vp.first) {
      boost::AlgoVertex v = *vp.first;
      auto index = boost::get(&boost::AlgoProps::m_name, execPlan);
      if (index[v] == node.getNodeName()) {
        auto index_eccentricity = boost::get(&boost::AlgoProps::m_eccentricity, execPlan);
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

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter(DecisionNode& node) const {

    if (m_slot->controlFlowState[node.getNodeIndex()] != 1)
      return true;
    return false;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visit(DecisionNode& node) {

    //std::cout << "1-st level Decision: " << node.getNodeName() << std::endl;
    bool allChildDecisionsResolved = true;
    for (auto child : node.getDaughters()) {
      int& childDecision = m_slot->controlFlowState[child->getNodeIndex()];

      if (childDecision == 1 && node.m_modeOR && node.m_modePromptDecision) {
        m_slot->controlFlowState[node.getNodeIndex()] = 1;
        return true;
      }

      if (childDecision == -1) {
        allChildDecisionsResolved = false;
      }
    }

    if (allChildDecisionsResolved)
      m_slot->controlFlowState[node.getNodeIndex()] = 1;

    return allChildDecisionsResolved;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter(AlgorithmNode& node) const {

    if (m_slot->controlFlowState[node.getNodeIndex()] != 1)
      return true;
    return false;
  }

  //--------------------------------------------------------------------------
  bool RunSimulator::visit(AlgorithmNode& node) {

    auto& states = m_slot->algsStates;
    int& decision = m_slot->controlFlowState[node.getNodeIndex()];

    auto dataPromoter = DataReadyPromoter(*m_slot, m_cause);

    if (State::INITIAL == states[node.getAlgoIndex()]) {
      states.updateState(node.getAlgoIndex(), State::CONTROLREADY);
      if (dataPromoter.visit(node)) {
        states.updateState(node.getAlgoIndex(), State::SCHEDULED);
        states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
        decision = 1;
        ++m_nodesSucceeded;
        //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
        return true;
      }
    } else if (State::CONTROLREADY == states[node.getAlgoIndex()] && dataPromoter.visit(node)) {
      states.updateState(node.getAlgoIndex(), State::SCHEDULED);
      states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
      decision = 1;
      ++m_nodesSucceeded;
      //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
      return true;
    }

    return false;
  }
}

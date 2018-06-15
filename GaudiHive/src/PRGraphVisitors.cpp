#include "PRGraphVisitors.h"
#include "AlgsExecutionStates.h"

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/ICondSvc.h"

#include <queue>

namespace concurrency
{

  using AState = AlgsExecutionStates::State;

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter( AlgorithmNode& node ) const
  {

    if ( AState::CONTROLREADY != m_slot->algsStates[node.getAlgoIndex()] ) return false;

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit( AlgorithmNode& node )
  {

    bool result = true; // return true if this algorithm has no data inputs

    for ( auto dataNode : node.getInputDataNodes() ) {

      result = dataNode->accept( *this );

      // With ConditionNodes, one may decide NOT to break here so that associated
      // ConditionAlgorithms are scheduled ASAP. This behavior can be made configurable
      if ( !result ) break; // skip checking other inputs if this input was not produced yet
    }

    if ( result ) {
      m_slot->algsStates.set( node.getAlgoIndex(), AState::DATAREADY ).ignore();

      // Inform parent slot if there is one
      if ( m_slot->parentSlot ) {
        m_slot->parentSlot->subSlotAlgsReady.push_back( std::make_pair( m_slot->eventContext, node.getAlgoIndex() ) );
      }

      if ( m_trace ) {
        auto sourceNode = ( m_cause.m_source == Cause::source::Task )
                              ? node.m_graph->getAlgorithmNode( m_cause.m_sourceName )
                              : nullptr;
        node.m_graph->addEdgeToPrecTrace( sourceNode, &node );
      }
    }

    // return true only if an algorithm is promoted to DR
    return result;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter( DataNode& ) const { return true; }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit( DataNode& node )
  {
    /* Implements 'observer' strategy, i.e., only check if producer of this DataNode
     * has been already executed or not */

    auto const& producers = node.getProducers();
    for ( auto algoNode : producers ) {
      const auto& state = m_slot->algsStates[algoNode->getAlgoIndex()];
      if ( AState::EVTACCEPTED == state || AState::EVTREJECTED == state ) {
        return true; // skip checking other producers if one was found to be executed
      }
    }

    // Check parent slot if necessary
    if ( m_slot->parentSlot ) {
      for ( auto algoNode : producers ) {
        const auto& state = m_slot->parentSlot->algsStates[algoNode->getAlgoIndex()];
        if ( AState::EVTACCEPTED == state || AState::EVTREJECTED == state ) {
          return true; // skip checking other producers if one was found to be executed
        }
      }
    }

    // return true only if this DataNode is produced
    return false;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter( ConditionNode& node ) const
  {

    if ( node.m_condSvc->isValidID( *( m_slot->eventContext ), node.getPath() ) )
      return false; // do not enter this ConditionNode if the condition has bee already loaded

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit( ConditionNode& node )
  {
    /* Implements 'requester' strategy, i.e., requests this ConditionNode to be loaded
     * by its associated ConditionAlgorithm */

    auto promoter = Supervisor( *m_slot, m_cause, m_trace );

    for ( auto condAlg : node.getProducers() ) condAlg->accept( promoter );

    // this method is called if, and only if, this ConditionNode is not yet produced.
    // thus, by definition, this ConditionNode is not yet available at this moment
    return false;
  }

  //--------------------------------------------------------------------------
  bool DecisionUpdater::visit( AlgorithmNode& node )
  {

    auto&         states   = m_slot->algsStates;
    const AState& state    = states[node.getAlgoIndex()];
    int           decision = -1;

    if ( true == node.isOptimist() )
      decision = 1;
    else if ( AState::EVTACCEPTED == state )
      decision = !node.isLiar();
    else if ( AState::EVTREJECTED == state )
      decision = node.isLiar();

    if ( -1 != decision ) {

      m_slot->controlFlowState[node.getNodeIndex()] = decision;

      auto promoter = DataReadyPromoter( *m_slot, m_cause, m_trace );
      for ( const auto& output : node.getOutputDataNodes() )
        for ( auto& consumer : output->getConsumers() ) consumer->accept( promoter );

      auto vis = concurrency::Supervisor( *m_slot, m_cause, m_trace );
      for ( auto& p : node.getParentDecisionHubs() ) p->accept( vis );

      return true; // return true only if the algorithm produced a decision
    }

    return false;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter( DecisionNode& node ) const
  {
    // Protect against graph traversal escaping from sub-slots
    if ( m_slot->parentSlot ) {
      // Examine the ancestry of this node, looking for sub-slot entry point
      bool                      canFindExit = false;
      std::queue<DecisionNode*> allAncestors;
      allAncestors.push( &node );
      while ( allAncestors.size() ) {

        DecisionNode* thisAncestor = allAncestors.front();
        allAncestors.pop();

        if ( thisAncestor->getNodeName() == m_slot->entryPoint ) {

          // This ancestor is the sub-slot exit
          canFindExit = true;
          break;

        } else {

          // Go further up the node ancestry
          for ( auto& evenOlder : thisAncestor->m_parents ) {

            allAncestors.push( evenOlder );
          }
        }
      }

      // If the sub-slot entry point is not in this node's ancestry, don't visit the node
      if ( !canFindExit ) return false;
    }

    if ( m_slot->controlFlowState[node.getNodeIndex()] != -1 ) return false;
    return true;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visit( DecisionNode& node )
  {

    bool foundNonResolvedChild = false;
    bool foundNegativeChild    = false;
    bool foundPositiveChild    = false;
    int  decision              = -1;

    // Leave a sub-slot if this is the exit node
    EventSlot* oldSlot = nullptr;
    if ( m_slot->parentSlot && m_slot->entryPoint == node.getNodeName() ) {
      oldSlot = m_slot;
      m_slot  = m_slot->parentSlot;
    }

    // If children are in sub-slots, loop over all
    auto searchResult = m_slot->subSlotsByNode.find( node.getNodeName() );
    if ( searchResult != m_slot->subSlotsByNode.end() ) {
      bool breakout = false;
      for ( unsigned int slotIndex : searchResult->second ) {

        // Enter the sub-slot
        m_slot = &( m_slot->allSubSlots[slotIndex] );

        for ( auto child : node.getDaughters() ) {

          int& childDecision = m_slot->controlFlowState[child->getNodeIndex()];

          if ( childDecision == -1 )
            foundNonResolvedChild = true;
          else if ( childDecision == 1 )
            foundPositiveChild = true;
          else
            foundNegativeChild = true;

          if ( node.m_modePromptDecision ) {
            if ( node.m_modeOR && foundPositiveChild ) {
              decision = 1;
              breakout = true;
              break;
            } else if ( !node.m_modeOR && foundNegativeChild ) {
              decision = 0;
              breakout = true;
              break;
            }
          } else {
            if ( foundNonResolvedChild ) {
              breakout = true;
              break;
            }
          }
        }

        // Leave the sub-slot
        m_slot = m_slot->parentSlot;
        if ( breakout ) break;
      }
    } else {
      for ( auto child : node.getDaughters() ) {
        int& childDecision = m_slot->controlFlowState[child->getNodeIndex()];

        if ( childDecision == -1 )
          foundNonResolvedChild = true;
        else if ( childDecision == 1 )
          foundPositiveChild = true;
        else
          foundNegativeChild = true;

        if ( node.m_modePromptDecision ) {
          if ( node.m_modeOR && foundPositiveChild ) {
            decision = 1;
            break;
          } else if ( !node.m_modeOR && foundNegativeChild ) {
            decision = 0;
            break;
          }
        } else {
          if ( foundNonResolvedChild ) break;
        }
      }
    } // end monitoring children

    if ( !foundNonResolvedChild && decision == -1 ) {
      if ( node.m_modeOR ) { // OR
        if ( foundPositiveChild )
          decision = 1;
        else
          decision = 0;
      } else { // AND
        if ( foundNegativeChild )
          decision = 0;
        else
          decision = 1;
      }
    }

    if ( node.m_inverted && decision == 1 )
      decision = 0;
    else if ( node.m_inverted && decision == 0 )
      decision = 1;

    if ( node.m_allPass && !foundNonResolvedChild ) decision = 1;

    if ( decision != -1 ) {
      m_slot->controlFlowState[node.getNodeIndex()] = decision;

      // if a decision was made for this node, propagate the result upwards
      for ( auto parent : node.m_parents ) {
        parent->accept( *this );
      }

      if ( oldSlot ) m_slot = oldSlot;
      return true;
    }

    // if no decision can be made yet, request further information downwards
    // Enter subslots for children if needed
    if ( searchResult != m_slot->subSlotsByNode.end() ) {
      for ( unsigned int slotIndex : searchResult->second ) {

        // Enter sub-slot
        m_slot = &( m_slot->allSubSlots[slotIndex] );

        for ( auto child : node.getDaughters() ) {
          bool result = child->accept( *this );
          if ( !node.m_modeConcurrent )
            if ( result ) break; // stop on first unresolved child if its decision hub is sequential
        }

        // Leave sub-slot
        m_slot = m_slot->parentSlot;
      }
    } else {
      for ( auto child : node.getDaughters() ) {
        bool result = child->accept( *this );
        if ( !node.m_modeConcurrent )
          if ( result ) break; // stop on first unresolved child if its decision hub is sequential
      }
    }

    if ( oldSlot ) m_slot = oldSlot;
    return false;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter( AlgorithmNode& node ) const
  {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != -1 ) return false;
    return true;
  }

  //--------------------------------------------------------------------------
  bool Supervisor::visit( AlgorithmNode& node )
  {

    bool result = false;

    auto& states = m_slot->algsStates;
    auto& state  = states[node.getAlgoIndex()];

    // Promote with INITIAL->CR
    if ( AState::INITIAL == state ) states.set( node.getAlgoIndex(), AState::CONTROLREADY ).ignore();

    // Try to promote with CR->DR
    if ( AState::CONTROLREADY == state ) {
      auto promoter = DataReadyPromoter( *m_slot, m_cause, m_trace );
      result        = promoter.visit( node );
    } else {
      result = true;
    }

    // return true only when an algorithm is not lower than DR in its FSM
    // i.e., the visitor has done everything it could with this algorithm
    return result;
  }

  //--------------------------------------------------------------------------
  bool RankerByProductConsumption::visit( AlgorithmNode& node )
  {

    auto& products = node.getOutputDataNodes();
    float rank     = 0;

    for ( auto p : products ) rank += p->getConsumers().size();

    node.setRank( rank );
    /*std::stringstream s;
    s << node.getNodeName() << ", " << rank << "\n";
    std::ofstream myfile;
    myfile.open("AlgoRank.csv", std::ios::app);
    myfile << s.str();
    myfile.close();*/

    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByCummulativeOutDegree::visit( AlgorithmNode& node )
  {

    std::ifstream myfile;
    myfile.open( "InputExecutionPlan.graphml", std::ios::in );

    precedence::PrecTrace execPlan;

    using precedence::AlgoTraceProps;
    using boost::get;

    boost::dynamic_properties dp;
    dp.property( "name", get( &AlgoTraceProps::m_name, execPlan ) );
    dp.property( "index", get( &AlgoTraceProps::m_index, execPlan ) );
    dp.property( "dataRank", get( &AlgoTraceProps::m_rank, execPlan ) );
    dp.property( "runtime", get( &AlgoTraceProps::m_runtime, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        runThroughAdjacents( v, execPlan );
        float rank = m_nodesSucceeded;
        node.setRank( rank );
        reset();
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }

    return true;
  }

  //--------------------------------------------------------------------------
  void RankerByCummulativeOutDegree::runThroughAdjacents(
      boost::graph_traits<precedence::PrecTrace>::vertex_descriptor vertex, precedence::PrecTrace graph )
  {
    typename boost::graph_traits<precedence::PrecTrace>::adjacency_iterator itVB;
    typename boost::graph_traits<precedence::PrecTrace>::adjacency_iterator itVE;

    for ( boost::tie( itVB, itVE ) = adjacent_vertices( vertex, graph ); itVB != itVE; ++itVB ) {
      m_nodesSucceeded += 1;
      runThroughAdjacents( *itVB, graph );
    }
  }

  //--------------------------------------------------------------------------
  bool RankerByTiming::visit( AlgorithmNode& node )
  {

    std::ifstream myfile;
    myfile.open( "InputExecutionPlan.graphml", std::ios::in );

    precedence::PrecTrace execPlan;
    using precedence::AlgoTraceProps;
    using boost::get;

    boost::dynamic_properties dp;
    dp.property( "name", get( &AlgoTraceProps::m_name, execPlan ) );
    dp.property( "index", get( &AlgoTraceProps::m_index, execPlan ) );
    dp.property( "dataRank", get( &AlgoTraceProps::m_rank, execPlan ) );
    dp.property( "runtime", get( &AlgoTraceProps::m_runtime, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        auto  index_runtime = get( &AlgoTraceProps::m_runtime, execPlan );
        float rank          = index_runtime[v];
        node.setRank( rank );
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }
    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByEccentricity::visit( AlgorithmNode& node )
  {

    std::ifstream myfile;
    myfile.open( "Eccentricity.graphml", std::ios::in );

    precedence::PrecTrace execPlan;

    boost::dynamic_properties dp;
    using boost::get;

    dp.property( "name", get( &precedence::AlgoTraceProps::m_name, execPlan ) );
    dp.property( "Eccentricity", get( &precedence::AlgoTraceProps::m_eccentricity, execPlan ) );

    boost::read_graphml( myfile, execPlan, dp );

    typedef boost::graph_traits<precedence::PrecTrace>::vertex_iterator itV;
    std::pair<itV, itV> vp;

    for ( vp = boost::vertices( execPlan ); vp.first != vp.second; ++vp.first ) {
      precedence::AlgoTraceVertex v     = *vp.first;
      auto                        index = get( &precedence::AlgoTraceProps::m_name, execPlan );
      if ( index[v] == node.getNodeName() ) {
        auto  index_eccentricity = get( &precedence::AlgoTraceProps::m_eccentricity, execPlan );
        float rank               = index_eccentricity[v];
        node.setRank( rank );
        // std::cout << "Rank of " << index[v] << " is " << rank << std::endl;
      }
    }
    return true;
  }

  //--------------------------------------------------------------------------
  bool RankerByDataRealmEccentricity::visit( AlgorithmNode& node )
  {

    // Find eccentricity of the node (only within the data realm of the execution flow graph)
    recursiveVisit( node );

    float rank = m_maxKnownDepth;
    node.setRank( rank );

    // Reset visitor for next nodes, if any
    reset();

    return true;
  }

  //--------------------------------------------------------------------------
  void RankerByDataRealmEccentricity::recursiveVisit( AlgorithmNode& node )
  {

    m_currentDepth += 1;

    auto& products = node.getOutputDataNodes();

    if ( products.empty() )
      if ( ( m_currentDepth - 1 ) > m_maxKnownDepth ) m_maxKnownDepth = m_currentDepth - 1;

    for ( auto p : products )
      for ( auto algoNode : p->getConsumers() ) recursiveVisit( *algoNode );

    m_currentDepth -= 1;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter( DecisionNode& node ) const
  {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != 1 ) return true;
    return false;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visit( DecisionNode& node )
  {

    bool allChildDecisionsResolved = true;

    for ( const auto& child : node.getDaughters() ) {

      child->accept( *this );

      int childDecision                                    = m_slot->controlFlowState[child->getNodeIndex()];
      if ( childDecision == -1 ) allChildDecisionsResolved = false;

      // process children sequentially if their decision hub is sequential
      if ( !node.m_modeConcurrent && childDecision == -1 ) return false;

      if ( childDecision == 1 && node.m_modeOR && node.m_modePromptDecision ) {
        m_slot->controlFlowState[node.getNodeIndex()] = 1;

        // if a decision was made for this node, propagate the result upwards
        for ( auto parent : node.m_parents ) {
          parent->accept( *this );
        }
        return true;
      }
    }

    if ( allChildDecisionsResolved ) {
      m_slot->controlFlowState[node.getNodeIndex()] = 1;

      // if a decision was made for this node, propagate the result upwards
      for ( auto parent : node.m_parents ) {
        parent->accept( *this );
      }
    }

    return allChildDecisionsResolved;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter( AlgorithmNode& node ) const
  {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != 1 ) return true;
    return false;
  }

  //--------------------------------------------------------------------------
  bool RunSimulator::visit( AlgorithmNode& node )
  {

    auto& states   = m_slot->algsStates;
    int&  decision = m_slot->controlFlowState[node.getNodeIndex()];

    auto dataPromoter = DataReadyPromoter( *m_slot, m_cause );

    if ( AState::INITIAL == states[node.getAlgoIndex()] ) {
      states.set( node.getAlgoIndex(), AState::CONTROLREADY );
      if ( dataPromoter.visit( node ) ) {
        states.set( node.getAlgoIndex(), AState::SCHEDULED );
        states.set( node.getAlgoIndex(), AState::EVTACCEPTED );
        decision = 1;
        ++m_nodesSucceeded;
        // std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
        return true;
      }
    } else if ( AState::CONTROLREADY == states[node.getAlgoIndex()] && dataPromoter.visit( node ) ) {
      states.set( node.getAlgoIndex(), AState::SCHEDULED );
      states.set( node.getAlgoIndex(), AState::EVTACCEPTED );
      decision = 1;
      ++m_nodesSucceeded;
      // std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
      return true;
    }

    return false;
  }
}

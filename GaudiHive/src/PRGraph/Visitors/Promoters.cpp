/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Promoters.h"
#include "../../AlgsExecutionStates.h"
#include "Validators.h"

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/ICondSvc.h"

#include <queue>

namespace concurrency {
  using AState = AlgsExecutionStates::State;

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter( AlgorithmNode& node ) const {

    if ( AState::CONTROLREADY != m_slot->algsStates[node.getAlgoIndex()] ) return false;

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit( AlgorithmNode& node ) {

    bool result = true; // return true if this algorithm has no data inputs

    for ( auto dataNode : node.getInputDataNodes() ) {

      result = dataNode->accept( *this );

      // With ConditionNodes, one may decide NOT to break here so that associated
      // ConditionAlgorithms are scheduled ASAP. This behavior can be made configurable
      if ( !result ) break; // skip checking other inputs if this input was not produced yet
    }

    if ( result ) {
      m_slot->algsStates.set( node.getAlgoIndex(), AState::DATAREADY ).ignore();

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
  bool DataReadyPromoter::visit( DataNode& node ) {
    /* Implements 'observer' strategy, i.e., only check if producer of this DataNode
     * has been already executed or not */

    auto const& producers = node.getProducers();
    for ( auto algoNode : producers ) {
      const auto& state = m_slot->algsStates[algoNode->getAlgoIndex()];
      if ( AState::EVTACCEPTED == state || AState::EVTREJECTED == state ) {
        return true; // skip checking other producers if one was found to be executed
      }
    }

    // return true only if this DataNode is produced
    return false;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visitEnter( ConditionNode& node ) const {

    if ( node.m_condSvc->isValidID( *( m_slot->eventContext ), node.name() ) )
      return false; // do not enter this ConditionNode if the condition has bee already loaded

    return true;
  }

  //--------------------------------------------------------------------------
  bool DataReadyPromoter::visit( ConditionNode& node ) {
    /* Implements 'requester' strategy, i.e., requests this ConditionNode to be loaded
     * by its associated ConditionAlgorithm */

    auto promoter = Supervisor( *m_slot, m_cause, m_trace );

    for ( auto condAlg : node.getProducers() ) condAlg->accept( promoter );

    // this method is called if, and only if, this ConditionNode is not yet produced.
    // thus, by definition, this ConditionNode is not yet available at this moment
    return false;
  }

  //--------------------------------------------------------------------------
  bool DecisionUpdater::visit( AlgorithmNode& node ) {

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

      // propagate decision upward to active regions of the graph
      auto  vis     = concurrency::Supervisor( *m_slot, m_cause, m_trace );
      auto& parents = node.getParentDecisionHubs();
      if ( parents.size() == 1 ) {
        parents[0]->accept( vis );
      } else if ( m_slot->parentSlot ) {
        auto scout = SubSlotScout( m_slot, node );
        for ( auto& p : parents ) {
          p->accept( scout );
          if ( scout.reply() ) p->accept( vis );
          scout.reset();
        }
      } else {
        auto scout = ActiveLineageScout( m_slot, node );
        for ( auto& p : parents ) {
          p->accept( scout );
          if ( scout.reply() ) p->accept( vis );
          scout.reset();
        }
      }

      return true; // return true only if the algorithm produced a decision
    }

    return false;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visitEnter( DecisionNode& node ) const {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != -1 ) return false;
    return true;
  }

  //---------------------------------------------------------------------------
  bool Supervisor::visit( DecisionNode& node ) {

    bool foundNonResolvedChild = false;
    bool foundNegativeChild    = false;
    bool foundPositiveChild    = false;
    int  decision              = -1;

    // Leave a sub-slot if this is the exit node
    EventSlot* oldSlot = nullptr;
    if ( m_slot->parentSlot && m_slot->entryPoint == node.name() ) {
      oldSlot = m_slot;
      m_slot  = m_slot->parentSlot;
    }

    // If children are in sub-slots, loop over all
    auto searchResult = m_slot->subSlotsByNode.find( node.name() );
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

      // propagate aggregated decision upward to active regions of the graph
      if ( node.m_parents.size() == 1 ) {
        node.m_parents[0]->accept( *this );
      } else if ( m_slot->parentSlot ) {
        auto scout = SubSlotScout( m_slot, node );
        for ( auto& p : node.m_parents ) {
          p->accept( scout );
          if ( scout.reply() ) p->accept( *this );
          scout.reset();
        }
      } else {
        auto scout = ActiveLineageScout( m_slot, node );
        for ( auto& p : node.m_parents ) {
          p->accept( scout );
          if ( scout.reply() ) p->accept( *this );
          scout.reset();
        }
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
  bool Supervisor::visitEnter( AlgorithmNode& node ) const {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != -1 ) return false;
    return true;
  }

  //--------------------------------------------------------------------------
  bool Supervisor::visit( AlgorithmNode& node ) {

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

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter( DecisionNode& node ) const {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != 1 ) return true;
    return false;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visit( DecisionNode& node ) {

    bool allChildDecisionsResolved = true;

    for ( const auto& child : node.getDaughters() ) {

      child->accept( *this );

      int childDecision = m_slot->controlFlowState[child->getNodeIndex()];
      if ( childDecision == -1 ) allChildDecisionsResolved = false;

      // process children sequentially if their decision hub is sequential
      if ( !node.m_modeConcurrent && childDecision == -1 ) return false;

      if ( childDecision == 1 && node.m_modeOR && node.m_modePromptDecision ) {
        m_slot->controlFlowState[node.getNodeIndex()] = 1;

        // if a decision was made for this node, propagate the result upwards
        for ( auto parent : node.m_parents ) { parent->accept( *this ); }
        return true;
      }
    }

    if ( allChildDecisionsResolved ) {
      m_slot->controlFlowState[node.getNodeIndex()] = 1;

      // if a decision was made for this node, propagate the result upwards
      for ( auto parent : node.m_parents ) { parent->accept( *this ); }
    }

    return allChildDecisionsResolved;
  }

  //---------------------------------------------------------------------------
  bool RunSimulator::visitEnter( AlgorithmNode& node ) const {

    if ( m_slot->controlFlowState[node.getNodeIndex()] != 1 ) return true;
    return false;
  }

  //--------------------------------------------------------------------------
  bool RunSimulator::visit( AlgorithmNode& node ) {

    auto& states   = m_slot->algsStates;
    int&  decision = m_slot->controlFlowState[node.getNodeIndex()];

    auto dataPromoter = DataReadyPromoter( *m_slot, m_cause );

    if ( AState::INITIAL == states[node.getAlgoIndex()] ) {
      states.set( node.getAlgoIndex(), AState::CONTROLREADY ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      if ( dataPromoter.visit( node ) ) {
        states.set( node.getAlgoIndex(), AState::SCHEDULED ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
        states.set( node.getAlgoIndex(), AState::EVTACCEPTED ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
        decision = 1;
        ++m_nodesSucceeded;
        // std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
        return true;
      }
    } else if ( AState::CONTROLREADY == states[node.getAlgoIndex()] && dataPromoter.visit( node ) ) {
      states.set( node.getAlgoIndex(), AState::SCHEDULED ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      states.set( node.getAlgoIndex(), AState::EVTACCEPTED ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      decision = 1;
      ++m_nodesSucceeded;
      // std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
      return true;
    }

    return false;
  }
} // namespace concurrency

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
#include "Validators.h"

#include <algorithm>

namespace concurrency {

  //---------------------------------------------------------------------------
  bool NodePropertiesValidator::visit( DecisionNode& node ) {

    if ( node.m_modeConcurrent && node.m_modePromptDecision ) {

      if ( !m_foundViolations )
        m_status
            << "  'Concurrent'/'Prompt' contradiction(s) found. Settings are mutually exclusive within a task group. "
               "Discarding 'Prompt' for ";

      m_status << ( m_foundViolations ? ", " : "" ) << node.name();

      if ( !m_foundViolations ) m_foundViolations = true;

      return false;
    }

    return true;
  }

  //---------------------------------------------------------------------------
  bool ActiveLineageScout::visit( DecisionNode& node ) {

    // Test if this node is already resolved
    if ( m_slot->controlFlowState[node.getNodeIndex()] != -1 ) {
      m_active = false;
      return m_active;
    }

    // Test if the node that sent this scout is out-of-sequence in this node
    if ( !node.m_modeConcurrent ) {

      for ( auto& child : node.getDaughters() ) {

        if ( child->name() == m_previousNodeName ) break;

        if ( m_slot->controlFlowState[child->getNodeIndex()] == -1 ) {
          m_active = false;
          return m_active;
        }
      }
    }

    this->visitParents( node );

    return this->reply();
  }

  //---------------------------------------------------------------------------
  void ActiveLineageScout::visitParents( DecisionNode& node ) {

    for ( auto& parent : node.m_parents ) {
      m_active           = true;
      m_previousNodeName = node.name();
      parent->accept( *this );

      // Any active parent means that this node is active
      if ( this->reply() ) break;
    }
  }

  //---------------------------------------------------------------------------
  void SubSlotScout::visitParents( DecisionNode& node ) {

    // Leave a sub-slot if this is the exit node
    const EventSlot* oldSlot = nullptr;
    if ( m_slot->parentSlot && m_slot->entryPoint == node.name() ) {
      oldSlot           = m_slot;
      m_slot            = m_slot->parentSlot;
      m_foundEntryPoint = true;
    }

    // Examine all parents
    for ( auto& parent : node.m_parents ) {
      m_active           = true;
      m_foundEntryPoint  = ( m_slot->parentSlot == nullptr );
      m_previousNodeName = node.name();
      parent->accept( *this );

      // Any active parent means that this node is active
      if ( this->reply() ) break;
    }

    if ( oldSlot ) m_slot = oldSlot;
  }

  //---------------------------------------------------------------------------
  bool ConditionalLineageFinder::visit( DecisionNode& node ) {

    auto propValidator = NodePropertiesValidator();
    propValidator.visit( node );

    // check if the visitor found a conditional path
    if ( node.m_modePromptDecision && propValidator.passed() ) {
      m_positive = true;
      return true;
    }

    // check if the visitor found an unconditional path
    if ( node.m_parents.empty() ) {
      m_negative = true;
      return true;
    }

    for ( const auto& parent : node.m_parents ) {
      this->visit( *parent );
      // check if a node is on both conditional and unconditional branches
      // and stop since further navigation won't change the conclusion
      if ( this->positive() && this->negative() ) break;
    }

    return true;
  }

  //---------------------------------------------------------------------------
  bool ConditionalLineageFinder::visit( AlgorithmNode& node ) {

    for ( const auto& parent : node.getParentDecisionHubs() ) {
      this->visit( *parent );
      if ( this->positive() && this->negative() ) break;
    }

    return true;
  }

  //---------------------------------------------------------------------------
  bool ProductionAmbiguityFinder::visit( DataNode& node ) {

    if ( node.getProducers().size() > 1 ) {

      if ( !m_foundViolations ) m_foundViolations = true;

      auto scout = ConditionalLineageFinder();

      for ( const auto& producer : node.getProducers() ) {

        producer->accept( scout );

        if ( scout.negative() ) {
          // register unconditional violation
          auto pr = m_unconditionalProducers.try_emplace( &node );
          pr.first->second.insert( producer );
        } else {
          // register conditional violation
          auto pr = m_conditionalProducers.try_emplace( &node );
          pr.first->second.insert( producer );
        }

        scout.reset();
      }
    }

    return true;
  }

  //---------------------------------------------------------------------------
  bool ProductionAmbiguityFinder::visit( ConditionNode& node ) {

    if ( node.getProducers().size() > 1 ) {

      if ( !m_foundViolations ) m_foundViolations = true;

      // all violations related to Condition Node are unconditional as
      // Condition Algorithms are detached from the CF realm
      for ( const auto& producer : node.getProducers() ) {
        // register unconditional violation
        auto pr = m_unconditionalProducers.try_emplace( &node );
        pr.first->second.insert( producer );
      }
    }

    return true;
  }

  //---------------------------------------------------------------------------
  std::string ProductionAmbiguityFinder::reply() const {

    std::ostringstream status{"  No topology violations found in the DF realm"};

    if ( m_foundViolations ) {
      status << "  Conditional (C) and/or unconditional (U) topology violations found in the DF realm:\n\n";

      for ( const auto& upr : m_unconditionalProducers ) {

        // add unconditional violations to the report
        status << " (U): " << upr.first->name() << " <---- |";
        for ( const auto& algo : upr.second )
          status << " " << algo->name() << " (U)"
                 << " |";

        // add conditional violations to the report
        auto result = m_conditionalProducers.find( upr.first );
        if ( result != m_conditionalProducers.end() ) {
          for ( const auto& algo : result->second )
            status << " " << algo->name() << " (C)"
                   << " |";
        }

        status << "\n";
      }

      // add remaining conditional violations to the report
      for ( const auto& cpr : m_conditionalProducers ) {

        if ( m_unconditionalProducers.find( cpr.first ) != m_unconditionalProducers.end() ) continue;

        status << " (C): " << cpr.first->name() << " <---- |";
        for ( const auto& algo : cpr.second )
          status << " " << algo->name() << " (C)"
                 << " |";
        status << "\n";
      }
    }

    return status.str();
  }

  //---------------------------------------------------------------------------
  bool TarjanSCCFinder::visit( AlgorithmNode& nodeAt ) {

    // DFS ******************************************************

    // put the node on stack
    m_stack.push_back( &nodeAt );

    // initialize and cache the low-link value of this node
    m_nodes_count += 1;
    unsigned int lowlink_init{m_nodes_count};
    // record initial low-link value of this node
    m_lowlinks.insert( {&nodeAt, {lowlink_init, lowlink_init}} );
    auto& lowlink = m_lowlinks[&nodeAt].second;

    for ( const auto& output : nodeAt.getOutputDataNodes() ) {
      for ( const auto& consumer : output->getConsumers() ) {
        consumer->accept( *this );
        // DFS callback ******************************************
        // propagate the low-link value back
        if ( on_stack( *consumer ) ) {
          if ( lowlink > m_lowlinks[consumer].second ) lowlink = m_lowlinks[consumer].second;
          // this allows to detect loops (i.e., a cycle consisting of a single algorithm node: A->d->A),
          // but there is a protection against this case at Algorithm::initialize()
          if ( consumer == &nodeAt ) m_scc.insert( {lowlink, {&nodeAt}} );
        }
      }
    }

    // If an SCC is found, book-keep it and take it off the stack
    if ( lowlink_init == lowlink ) {
      if ( m_scc.find( lowlink ) == m_scc.end() ) m_scc.insert( {lowlink, {}} );

      for ( auto stackNodeRIter = m_stack.rbegin(); stackNodeRIter != m_stack.rend(); ++stackNodeRIter ) {

        bool lastSCCNodeOnStack{*stackNodeRIter == &nodeAt};
        if ( lowlink == m_lowlinks[*stackNodeRIter].second ) m_scc[lowlink].push_back( *stackNodeRIter );

        m_stack.pop_back();
        if ( lastSCCNodeOnStack ) break;
      }
    }

    return true;
  }

  //---------------------------------------------------------------------------
  std::string TarjanSCCFinder::reply() {

    if ( !passed() ) {

      m_status << "  Strongly connected components found in DF realm:";

      for ( auto& pr : m_scc ) {
        if ( pr.second.size() > 1 ) {
          m_status << "\n o [lowlink:" << std::to_string( pr.first ) << "] |";

          // rotate SCCs to get reproducible order
          std::vector sortedSCC( pr.second.begin(), pr.second.end() );
          std::sort( sortedSCC.begin(), sortedSCC.end(), CompareNodes<AlgorithmNode*>() );
          std::rotate( pr.second.begin(), std::find( pr.second.begin(), pr.second.end(), sortedSCC.front() ),
                       pr.second.end() );

          for ( const auto& algoPtr : pr.second ) m_status << " " << algoPtr->name() << " |";
        }
      }
    }

    return m_status.str();
  }

} // namespace concurrency

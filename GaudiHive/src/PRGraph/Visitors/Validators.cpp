#include "Validators.h"

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

} // namespace concurrency

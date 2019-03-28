#include "Validators.h"

namespace concurrency {
  //---------------------------------------------------------------------------
  bool NodePropertiesValidator::visit( DecisionNode& node ) {

    if ( node.m_modeConcurrent && node.m_modePromptDecision ) {

      if ( !m_foundViolations )
        m_status
            << "  'Concurrent'/'Prompt' contradiction(s) found. Settings are mutually exclusive within a task group. "
               "Discarding 'Prompt' for ";

      m_status << ( m_foundViolations ? ", " : "" ) << node.getNodeName();

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

        if ( child->getNodeName() == m_previousNodeName ) break;

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
      m_previousNodeName = node.getNodeName();
      parent->accept( *this );

      // Any active parent means that this node is active
      if ( this->reply() ) break;
    }
  }

  //---------------------------------------------------------------------------
  void SubSlotScout::visitParents( DecisionNode& node ) {

    // Leave a sub-slot if this is the exit node
    const EventSlot* oldSlot = nullptr;
    if ( m_slot->parentSlot && m_slot->entryPoint == node.getNodeName() ) {
      oldSlot           = m_slot;
      m_slot            = m_slot->parentSlot;
      m_foundEntryPoint = true;
    }

    // Examine all parents
    for ( auto& parent : node.m_parents ) {
      m_active           = true;
      m_foundEntryPoint  = ( m_slot->parentSlot == nullptr );
      m_previousNodeName = node.getNodeName();
      parent->accept( *this );

      // Any active parent means that this node is active
      if ( this->reply() ) break;
    }

    if ( oldSlot ) m_slot = oldSlot;
  }
} // namespace concurrency

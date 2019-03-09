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
  bool ActiveSubgraphScout::visit( DecisionNode& node ) {

    if ( m_slot.controlFlowState[node.getNodeIndex()] != -1 ) {
      m_active = false;
      return m_active;
    }

    if ( !node.m_modeConcurrent ) {

      for ( auto& child : node.getDaughters() ) {

        if ( child->getNodeName() == m_previousNodeName ) break;

        if ( m_slot.controlFlowState[child->getNodeIndex()] == -1 ) {
          m_active = false;
          return m_active;
        }
      }
    }

    m_previousNodeName = node.getNodeName();
    if ( node.m_parents.size() > 0 ) node.m_parents[0]->accept( *this );

    return m_active;
  }
} // namespace concurrency
